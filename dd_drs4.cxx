//********************************************************************
//
//  Name:         dd_drs4.cxx
//  Created by:   Kolby Kiesling
//
//  Contents:     Device driver for PSI DRS4
//
//  $Id: $
//
//********************************************************************
#include <string>
#include <cstdio>
#include <cstdlib>
#include <cstdarg>
#include <cstring>
#include <cassert>
#include <cmath>
#include <algorithm>
#include <iostream>
#include "midas.h"
//#include "DRS.h"
#undef calloc
//using namespace std;
//DRS *drs;
//DRSBoard *b;

#define DEFAULT_TIMEOUT 1000     // milliseconds
#define SHORT_TIMEOUT 100


#define TRIGGER_0_OUT 16
#define TRIGGER_1_OUT 17
#define TRIGGER_2_OUT 18
#define SUM_OUT 19


#define DD_DRS4_SETTINGS_STR "\
Read Period ms = FLOAT : 200\n\
"

typedef struct {
  int readPeriod_ms; // maybe need?
} DD_DRS4_SETTINGS;


typedef struct {
  DD_DRS4_SETTINGS settings;
  DD_DRS4_SETTINGS settingsIncoming;

  INT num_channels;
  INT(*bd)(INT cmd, ...);      // bus driver entry function
  void *bd_info;               // private info of bus driver
  HNDLE hkey;                  // ODB key for bus driver info


  float *array;                // Most recent measurement or NaN, one for each channel
  DWORD *update_time;          // seconds

  INT get_label_calls;

} DD_DRS4_INFO;



int drs4_apply_settings(DD_DRS4_INFO* info) {
  /* Format of most commands to R / W from the DRS4:
   * (char) + (int) + (opt. modifier) 
   * Where char is the identifying register, int is the data write and optional modifier is for special cases...
   */
  char cmd[256]; // only can set the pulser right now...
  char str[256];
  memset(cmd, 0, sizeof(cmd));
  memset(str, 0, sizeof(str));
  // want to write for loops to automate the r / w process...
  
  //snprintf(cmd, sizeof(cmd)-1, "sc %d\r\n", info->settings.set_coincidence); // set coincidence timing
  //BD_PUTS(cmd); // only need a few BD_PUTS, do not really care about returns in this case because these registers are really
  
  return FE_SUCCESS; // TODO: make sure things applied okay...
}


void drs4_settings_updated(INT hDB, INT hkey, void* vinfo)
{ // simple routines to iterate through to check for changes in the ODB...
  printf("Settings updated\n");
  
  DD_DRS4_INFO* info = (DD_DRS4_INFO*) vinfo;

  bool changed=false;
  
  if (info->settingsIncoming.readPeriod_ms != info->settings.readPeriod_ms) {
    std::cout << "   readPeriod_ms changed from ``" << info->settings.readPeriod_ms << "'' to ``" << info->settingsIncoming.readPeriod_ms << "''" << std::endl;
    info->settings.readPeriod_ms = info->settingsIncoming.readPeriod_ms;
    changed=true;
  }

  if (changed) drs4_apply_settings(info); // TODO: only apply the changed one...
}


//---- standard device driver routines -------------------------------

INT dd_drs4_init(HNDLE hkey, void **pinfo, INT channels, INT(*bd)(INT cmd, ...))
{
  int status;
  HNDLE hDB, hkeydd;
  DD_DRS4_INFO *info;
  printf("dd_drs4_init: channels = %d\n", channels);

  info = new DD_DRS4_INFO;
  *pinfo = info;

  cm_get_experiment_database(&hDB, NULL);

  info->array = (float*) calloc(channels, sizeof(float));
  info->update_time = (DWORD*) calloc(channels, sizeof(DWORD));
  
  for (int i=0; i<channels; ++i) { // TODO: obviously change this to be back to channels, just making notes for the meantime...
    info->array[i] = ss_nan();
    info->update_time[i] = 0;
  }
  
  info->get_label_calls=0;  
  
  info->num_channels = channels;  // TODO: make sure it is 19 channel readout
  info->bd = bd;
  info->hkey = hkey;

  // DD Settings
  status = db_create_record(hDB, hkey, "DD", DD_DRS4_SETTINGS_STR); // should make the database correctly now...
  if (status != DB_SUCCESS)
     return FE_ERR_ODB;

  status = db_find_key(hDB, hkey, "DD", &hkeydd);
  if (status != DB_SUCCESS) {
    return FE_ERR_ODB;
  }
  int size = sizeof(info->settingsIncoming);
  status = db_get_record(hDB, hkeydd, &info->settingsIncoming, &size, 0);
  if (status != DB_SUCCESS) return FE_ERR_ODB;

  status = db_open_record(hDB, hkeydd, &info->settingsIncoming,
                          size, MODE_READ, drs4_settings_updated, info);
  if (status != DB_SUCCESS) {
    return FE_ERR_ODB;
  }
  memcpy(&info->settings, &info->settingsIncoming, sizeof(info->settingsIncoming));


  // Initialize bus driver
  status = info->bd(CMD_INIT, info->hkey, &info->bd_info);
  if (status != SUCCESS) return status;
  
  printf("Sending initialization commands to DRS4\n");
  char str[256];
  memset(str, 0, sizeof(str));
  int len=0;
  if (len) {}
  
  // TODO: Check to see if DRS4 is outputing data
  status = BD_PUTS("ra 19\r\n"); // read firmware ID...
  ss_sleep(50);
  int tries=0;
  if (tries) {}
  if (status) {}
  //if (len == 0) {
  //}

  drs4_apply_settings(info); // settings are probably not functional, but they appear to be getting there...
  //status = info->bd(CMD_EXIT, info->bd_info);
  
  return FE_SUCCESS;
}

//--------------------------------------------------------------------

INT dd_drs4_exit(DD_DRS4_INFO * info)
{
  printf("Running dd_drs4_exit\n");

  // Close serial
  info->bd(CMD_EXIT, info->bd_info);

  delete info;

  return FE_SUCCESS;
}

//--------------------------------------------------------------------

INT dd_drs4_set(DD_DRS4_INFO * info, INT channel, int value) // TODO: make sure value is int everywhere...
{
  if (channel < 0 || channel >= info->num_channels) // This function may not be necessary since everything is in
    return FE_ERR_DRIVER; // the settings now...

  channel+=info->num_channels;

  char cmd[256];
  char str[256];
  memset(cmd, 0, sizeof(cmd));
  memset(str, 0, sizeof(str));
  printf("Set channel %d to %d\n", channel, value);
  
  switch (channel) {
    case TRIGGER_0_OUT: // Pulser status
      break;
    case TRIGGER_1_OUT:
      break;
  }

  return FE_SUCCESS;
}

//--------------------------------------------------------------------

INT dd_drs4_get(DD_DRS4_INFO * info, INT channel, float *pvalue)
{
  //INT chn = channel + 2;
  printf("\n--------------------\nChecking Channel:\t%d\n--------------------\n", channel);
  INT status = 0;
  char str[256], cmd[256];
  *pvalue = ss_nan();
  memset(str, 0,sizeof(str)-1);
  memset(cmd, 0, sizeof(cmd));
  float frq=-1;

  switch (channel) {
    case TRIGGER_0_OUT:
      *pvalue = frq;
      //info->chn_0_frq = frq;
      printf("Trigger:\t%d\tFrequency:\t%f\n", channel, frq);
      break;
    case TRIGGER_1_OUT:
      *pvalue = frq;
      //info->chn_1_frq = frq;
      printf("Trigger:\t%d\tFrequency:\t%f\n", channel, frq);
      break;
    case TRIGGER_2_OUT:
      *pvalue = frq;
      printf("Trigger:\t%d\tFrequency:\t%f\n", channel, frq);
      break;
    case SUM_OUT:
      *pvalue = frq;
      //info->chn_1_frq = frq;
      printf("Sum Rates\tFrequency:\t%f\n", frq);
      break;
    default:
      *pvalue = frq;
      break;
  }

  //ss_sleep(1);
  return FE_SUCCESS;
}

//--------------------------------------------------------------------

INT dd_drs4_get_label(DD_DRS4_INFO * info, INT channel, char *name)
{

  // Keep track of calls to this function to get right channel labels with multi.c class driver.
  //if (info->num_channels==2 && info->get_label_calls > info->num_channels/2) {
    //channel+=info->num_channels;
  //}

  switch (channel) {
    case TRIGGER_0_OUT:
      strncpy(name, "Trigger 0 (Hz)", NAME_LENGTH-1);
      break;
    case TRIGGER_1_OUT:
      strncpy(name, "Trigger 1 (Hz)", NAME_LENGTH-1);
      break;
    case TRIGGER_2_OUT:
      strncpy(name, "Trigger 2 (Hz)", NAME_LENGTH-1);
      break;
    case SUM_OUT:
      strncpy(name, "Sum (Hz)", NAME_LENGTH-1);
      break;
    default:
      memset(name, 0, NAME_LENGTH);
      snprintf(name, NAME_LENGTH-1, "Channel %d Hz", channel);
      //return FE_ERR_DRIVER;
  }

  //printf("dd_arduino_fan_pid_get_label: chan=%d, Name=``%s''\n",channel,name);
  //info->get_label_calls++;
  return FE_SUCCESS;
}

//---- device driver entry point -------------------------------------
#ifdef __cplusplus
extern "C" {
#endif

INT dd_drs4(INT cmd, ...)
{
  va_list argptr;
  HNDLE hKey;
  INT channel, status;
  DWORD flags=0;
  float value, *pvalue;
  void *info, *bd;
  char* name;

  va_start(argptr, cmd);
  status = FE_SUCCESS;

  switch (cmd) {
    case CMD_INIT:
      hKey = va_arg(argptr, HNDLE);
      info = va_arg(argptr, void *);
      channel = va_arg(argptr, INT);
      flags = va_arg(argptr, DWORD);
      if (flags==0) {} // prevent set-but-unused compile warning
      bd = va_arg(argptr, void *);
      status = dd_drs4_init(hKey, (void**)info, channel, (INT (*)(INT, ...)) bd);
      break;

    case CMD_EXIT:
      info = va_arg(argptr, void *);
      status = dd_drs4_exit((DD_DRS4_INFO*) info);
      break;

    case CMD_SET:
      info = va_arg(argptr, void *);
      channel = va_arg(argptr, INT);
      value = (int) va_arg(argptr, double); // probably will break...
      status = dd_drs4_set((DD_DRS4_INFO*) info, channel, value);
      break;

    case CMD_GET:
      info = va_arg(argptr, void *);
      channel = va_arg(argptr, INT);
      pvalue = va_arg(argptr, float *);
      status = dd_drs4_get((DD_DRS4_INFO*) info, channel, pvalue);
      break;

    case CMD_GET_LABEL:
      info = va_arg(argptr, void *);
      channel = va_arg(argptr, INT);
      name = va_arg(argptr, char *);
      status = dd_drs4_get_label((DD_DRS4_INFO*) info, channel, name);
      break;

    default:
      break;
  }

  va_end(argptr);

  return status;
}

#ifdef __cplusplus
}
#endif

//--------------------------------------------------------------------


