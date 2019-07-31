/********************************************************************\

  Name:         feDRS4.cc
  Created by:   Kolby Kiesling

  Contents:     Experiment specific readout code (user part) of
                DRS4 oscilloscope.

\********************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "DRS.h"
DRS *drs; // initialize DRS object for later use, cannot call after calling MIDAS...
DRSBoard *board;
int i_start=0, i_end=0;



#include "midas.h"
#include "experim.h"
#include "mfe.h"

/*-- Globals -------------------------------------------------------*/

/* The frontend name (client name) as seen by other MIDAS clients   */
const char *frontend_name = "feDRS4";
/* The frontend file name, don't change it */
const char *frontend_file_name = __FILE__;

/* frontend_loop is called periodically if this variable is TRUE    */
BOOL frontend_call_loop = FALSE;

/* a frontend status page is displayed with this frequency in ms */
INT display_period = 3000;

/* maximum event size produced by this frontend */
INT max_event_size = 10000;

/* maximum event size for fragmented events (EQ_FRAGMENTED) */
INT max_event_size_frag = 5 * 1024 * 1024;

/* buffer size to hold events */
INT event_buffer_size = 100 * 10000;

/*
 Forward Declaration of DRS Types 
int i_start=0, i_end; // Initialization process to find N-number of DRS boards...
DRS *drs;
DRSBoard *board; 

drs = new DRS();
for(int i=i_start; i<drs->GetNumberOfBoards(); i++){
  i_end=i; // find number of boards connected
}
*/



/*-- Function declarations -----------------------------------------*/

INT frontend_init(void);
INT frontend_exit(void);
INT begin_of_run(INT run_number, char *error);
INT end_of_run(INT run_number, char *error);
INT pause_run(INT run_number, char *error);
INT resume_run(INT run_number, char *error);
INT frontend_loop();

INT read_trigger_event(char *pevent, INT off);
INT read_periodic_event(char *pevent, INT off);

INT poll_event(INT source, INT count, BOOL test);
INT interrupt_configure(INT cmd, INT source, POINTER_T adr);

/*-- Equipment list ------------------------------------------------*/

EQUIPMENT equipment[] = {

   {"DRS Trigger",               /* equipment name */
      {3, 0,                 /* event ID, trigger mask */
         "SYSTEM",           /* event buffer */
         EQ_POLLED,          /* equipment type */
         0,                  /* event source */
         "MIDAS",            /* format */
         TRUE,               /* enabled */
         RO_RUNNING |        /* read only when running */
         RO_ODB,             /* and update ODB */
         100,                /* poll for 100ms */
         0,                  /* stop run after this event limit */
         0,                  /* number of sub events */
         0,                  /* don't log history */
         "", "", "",},
      read_trigger_event,    /* readout routine */
   },

   {"DRS Periodic",              /* equipment name */
      {2, 0,                 /* event ID, trigger mask */
         "SYSTEM",           /* event buffer */
         EQ_PERIODIC,        /* equipment type */
         0,                  /* event source */
         "MIDAS",            /* format */
         TRUE,               /* enabled */
         RO_RUNNING | RO_TRANSITIONS |   /* read when running and on transitions */
         RO_ODB,             /* and update ODB */
         1000,               /* read every sec */
         0,                  /* stop run after this event limit */
         0,                  /* number of sub events */
         TRUE,               /* log history */
         "", "", "",},
      read_periodic_event,   /* readout routine */
   },

   {""}
};

/********************************************************************\
              Callback routines for system transitions

  These routines are called whenever a system transition like start/
  stop of a run occurs. The routines are called on the following
  occations:

  frontend_init:  When the frontend program is started. This routine
                  should initialize the hardware.

  frontend_exit:  When the frontend program is shut down. Can be used
                  to releas any locked resources like memory, commu-
                  nications ports etc.

  begin_of_run:   When a new run is started. Clear scalers, open
                  rungates, etc.

  end_of_run:     Called on a request to stop a run. Can send
                  end-of-run event and close run gates.

  pause_run:      When a run is paused. Should disable trigger events.

  resume_run:     When a run is resumed. Should enable trigger events.
\********************************************************************/

/*-- Frontend Init -------------------------------------------------*/

INT frontend_init()
{
   /* put any hardware initialization here */
   /* Forward Declaration of DRS Types    */

   drs = new DRS();
   for (int i=i_start; i<drs->GetNumberOfBoards(); i++){
     i_end=i+1; // find number of boards connected
   }
 
   int nBoards;
   for (int i=i_start; i<i_end; i++) {
     board=drs->GetBoard(i); // fetch the board index
     printf("Found DRS%d board %2d on USB, serial #%04d, firmware revision %5d\n", 
         board->GetDRSType(), i, board->GetBoardSerialNumber(), board->GetFirmwareVersion());
     board->SetLED(0); // turn LED off...
  }
  
  nBoards=drs->GetNumberOfBoards();
  printf("Boards found\t%d\n\n", nBoards);
  if (nBoards==0) {
    printf("No DRS4 evaluation board found\n");
    return FE_ERR_HW;
  }
  if (i_end) {} // compiling warnings
   /* print message and return FE_ERR_HW if frontend should not be started */
  for (int i=0; i<i_end; i++) {
    board=drs->GetBoard(i); // fetch the board
    printf("--------------------\nInitializing DRS%d board %2d on USB, serial #%04d, firmware revision %5d\n--------------------\n", 
         board->GetDRSType(), i, board->GetBoardSerialNumber(), board->GetFirmwareVersion());
    // TODO: use DRS ONE as the master unit and all other DRS units will be slaves
    board->Init(); // initialize the board for daq
    board->SetFrequency(5, true); // set sampling frequency
    board->SetTranspMode(1);
    board->SetInputRange(0); // -0.5V to 0.5V
    board->EnableTcal(1); // enable internal clock
    if (board->GetBoardType() >= 8) {
      board->EnableTrigger(1, 0); // enable hardware trigger
      board->SetTriggerSource(1<<0); // set channel zero as source (CH1)
    }
    else if (board->GetBoardType() == 7) {
      board->EnableTrigger(0, 1); // lemo off, analog on
      board->SetTriggerSource(1<<0);
    }
    
    if (i==0) {
      board->SetIndividualTriggerLevel(0, 0.1); // individual thresholds on DRS 1
      board->SetIndividualTriggerLevel(1, 0.2);
      board->SetIndividualTriggerLevel(2, 0.3);
      board->SetIndividualTriggerLevel(3, 0.4);
      board->SetTriggerSource(15); // set total coincidence between all channels
    }
    else { // set slave units for external triggers?
      if (board->GetBoardType() >= 8) {
	board->EnableTrigger(1, 0);           // enable hardware trigger
	board->SetTriggerSource(1<<4);        // set external trigger as source
      } 
      else {                          // Evaluation Board V3
	board->EnableTrigger(1, 0);           // lemo on, analog trigger off
      }
    }
    board->SetTriggerLevel(0.025); // 0.025V?
    board->SetTriggerPolarity(true); // negative pulses?
    board->SetTriggerDelayNs(0); // no delays on triggers...
  }
  return SUCCESS;
}

/*-- Frontend Exit -------------------------------------------------*/

INT frontend_exit()
{
  printf("Exiting DAQ\n");
  return SUCCESS;
}

/*-- Begin of Run --------------------------------------------------*/

INT begin_of_run(INT run_number, char *error)
{
   /* put here clear scalers etc. */

   return SUCCESS;
}

/*-- End of Run ----------------------------------------------------*/

INT end_of_run(INT run_number, char *error)
{
   return SUCCESS;
}

/*-- Pause Run -----------------------------------------------------*/

INT pause_run(INT run_number, char *error)
{
   return SUCCESS;
}

/*-- Resuem Run ----------------------------------------------------*/

INT resume_run(INT run_number, char *error)
{
   return SUCCESS;
}

/*-- Frontend Loop -------------------------------------------------*/

INT frontend_loop()
{
   /* if frontend_call_loop is true, this routine gets called when
      the frontend is idle or once between every event */
   return SUCCESS;
}

/*------------------------------------------------------------------*/

/********************************************************************\

  Readout routines for different events

\********************************************************************/

/*-- Trigger event routines ----------------------------------------*/

INT poll_event(INT source, INT count, BOOL test)
/* Polling routine for events. Returns TRUE if event
   is available. If test equals TRUE, don't return. The test
   flag is used to time the polling */
{
   int i;
   DWORD flag;

   for (i = 0; i < count; i++) {
      /* poll hardware and set flag to TRUE if new event is available */
      flag = TRUE;

      if (flag)
         if (!test)
            return TRUE;
   }

   return 0;
}

/*-- Interrupt configuration ---------------------------------------*/

INT interrupt_configure(INT cmd, INT source, POINTER_T adr)
{
   switch (cmd) {
   case CMD_INTERRUPT_ENABLE:
      break;
   case CMD_INTERRUPT_DISABLE:
      break;
   case CMD_INTERRUPT_ATTACH:
      break;
   case CMD_INTERRUPT_DETACH:
      break;
   }
   return SUCCESS;
}

/*-- Event readout -------------------------------------------------*/

INT read_trigger_event(char *pevent, INT off)
{
   WORD *pdata, a;

   /* init bank structure */
   bk_init(pevent);

   /* create structured ADC0 bank */
   bk_create(pevent, "ADC0", TID_WORD, (void **)&pdata);

   /* following code to "simulates" some ADC data */
   for (a = 0; a < 4; a++)
      *pdata++ = rand()%1024 + rand()%1024 + rand()%1024 + rand()%1024;

   bk_close(pevent, pdata);

   /* create variable length TDC bank */
   bk_create(pevent, "TDC0", TID_WORD, (void **)&pdata);

   /* following code to "simulates" some TDC data */
   for (a = 0; a < 4; a++)
      *pdata++ = rand()%1024 + rand()%1024 + rand()%1024 + rand()%1024;

   bk_close(pevent, pdata);

   /* limit event rate to 100 Hz. In a real experiment remove this line */
   ss_sleep(10);

   return bk_size(pevent);
}

/*-- Periodic event ------------------------------------------------*/

INT read_periodic_event(char *pevent, INT off)
{
   float *pdata;
   int a;

   /* init bank structure */
   bk_init(pevent);

   /* create SCLR bank */
   bk_create(pevent, "PRDC", TID_FLOAT, (void **)&pdata);

   /* following code "simulates" some values */
   for (a = 0; a < 4; a++)
      *pdata++ = 100*sin(M_PI*time(NULL)/60+a/2.0);

   bk_close(pevent, pdata);

   return bk_size(pevent);
}
