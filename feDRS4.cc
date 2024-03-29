//********************************************************************
//
//  Name:         feDRS4.cc
//  Created by:   Kolby Kiesling
//
//  Contents:     readout of DRS4 Eval. Board
//
//  $Id: $
//
//********************************************************************
#include "DRS.h"
#include <cstdio>

  /* Forward Declaration of DRS Types */
int i_start=0, i_end; // Initialization process to find N-number of DRS boards...
DRS *drs;
DRSBoard *board; 

//drs = new DRS();
for(int i=i_start; i<drs->GetNumberOfBoards(); i++){
  i_end=i; // find number of boards connected
}


#include "midas.h"
#include "mfe.h"
#include <math.h>


//#ifdef __cplusplus
//extern "C" {
//#endif
//#ifdef __cplusplus
//}
//#endif


//-- Globals ---------------------------------------------------------
//#ifdef __cplusplus
//extern "C" {
//#endif

const char *frontend_name = "feDRS4";  // client name visible to other MIDAS clients
const char *frontend_file_name = __FILE__;     // This frontend's source file name.  Do not change it.

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


/*-- Function declarations -----------------------------------------*/

INT frontend_init(void);
INT frontend_exit(void);
INT begin_of_run(INT run_number, char *error);
INT end_of_run(INT run_number, char *error);
INT pause_run(INT run_number, char *error);
INT resume_run(INT run_number, char *error);
INT frontend_loop(void);

INT read_trigger_event(char *pevent, INT off);
INT read_periodic_event(char *pevent, INT off);

INT poll_event(INT source, INT count, BOOL test);
INT interrupt_configure(INT cmd, INT source, POINTER_T adr);



//-- Equipment list --------------------------------------------------

// device driver list

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
   int nBoards;
   for (int i=i_start; i<i_end; i++) {
     board=drs->GetBoard(i); // fetch the board index
     printf("Found DRS4 Evaluation board, serial #%d, firmware revision %d\n",
	    board->GetBoardSerialNumber(), board->GetFirmwareVersion());
  }
  
  nBoards=drs->GetNumberOfBoards();
  if (nBoards==0) {
    printf("No DRS4 evaluation board found\n");
    return -1;
  }
  

   /* print message and return FE_ERR_HW if frontend should not be started */

   return SUCCESS;
}

/*-- Frontend Exit -------------------------------------------------*/

INT frontend_exit()
{
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

//--------------------------------------------------------------------
//#ifdef __cplusplus
//}
//#endif
