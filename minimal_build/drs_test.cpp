#include "DRS.h"
#include <iostream>
using namespace std;

int main() {
   int i, j, nBoards;
   DRS *drs;
   DRSBoard *b;
   float time_array[8][1024];
   float wave_array[8][1024];
   /* do initial scan */
   drs = new DRS();

   /* show any found board(s) */
   for (i=0 ; i<drs->GetNumberOfBoards() ; i++) {
      b = drs->GetBoard(i);
      printf("Found DRS4 evaluation board, serial #%d, firmware revision %d\n", 
         b->GetBoardSerialNumber(), b->GetFirmwareVersion());
   }

   /* exit if no board found */
   nBoards = drs->GetNumberOfBoards();
   if (nBoards == 0) {
      printf("No DRS4 evaluation board found\n");
      return 0;
   }
   
   return 1;
}