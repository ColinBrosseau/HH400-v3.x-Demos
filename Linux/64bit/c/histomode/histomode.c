/************************************************************************

  Demo access to HydraHarp 400 Hardware via HHLIB v 3.0.
  The program performs a measurement based on hardcoded settings.
  The resulting histogram (65536 channels) is stored in an ASCII output file.

  Michael Wahl, PicoQuant GmbH, August 2014

  Note: This is a console application (i.e. run in Windows cmd box)

  Note: At the API level channel numbers are indexed 0..N-1 
		where N is the number of channels the device has.

  
  Tested with the following compilers:

  - MinGW 2.0.0-3 (free compiler for Win 32 bit)
  - MS Visual C++ 6.0 (Win 32 bit)
  - MS Visual Studio 2010 (Win 64 bit)
  - Borland C++ 5.3 (Win 32 bit)
  - gcc 4.8.1 (Linux 32/64 bit)

************************************************************************/

#ifdef _WIN32
#include <windows.h>
#include <dos.h>
#include <conio.h>
#else
#include <unistd.h>
#define Sleep(msec) usleep(msec*1000)
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "hhdefin.h"
#include "hhlib.h"
#include "errorcodes.h"

#include "cmdline.h"

unsigned int counts[HHMAXINPCHAN][MAXHISTLEN];


int main(int argc, char* argv[])
{

 struct gengetopt_args_info ai;
 if (cmdline_parser(argc, argv, &ai) != 0) {
   exit(1);
 }

 // Get command line options
 unsigned int filename_given = ai.Filename_given;
 char* filename = ai.Filename_arg; // output filename
 int Tacq = (int)(ai.Duration_arg*1000); // Measurement time (millisec)
 int Binning = ai.Binning_arg; 
 int SyncDivider = ai.SyncDivider_arg; ; 
 int SyncCFDZeroCross = ai.SyncCFDZeroCross_arg; // (mV)
 int SyncCFDLevel = ai.SyncCFDLevel_arg; // (mV)
 int SyncChannelOffset = ai.SyncChannelOffset_arg; // (ps)
 int InputCFDZeroCross = ai.InputCFDZeroCross_arg; // (mV)
 int InputCFDLevel = ai.InputCFDLevel_arg; // (mV)
 int InputChannelOffset = ai.InputChannelOffset_arg; // (ps)  

 if (filename_given){
   printf("Filename: %s\n", filename);
   printf("Duration (ms): %d\n", Tacq);
   printf("Binning: %d\n", Binning);
   printf("SyncDivider: %d\n", SyncDivider);
   printf("SyncCFDZeroCross: %d\n", SyncCFDZeroCross);
   printf("SyncCFDLevel: %d\n", SyncCFDLevel);
   printf("SyncChannelOffset: %d\n", SyncChannelOffset);
   printf("InputCFDZeroCross: %d\n", InputCFDZeroCross);
   printf("InputCFDLevel: %d\n", InputCFDLevel);
   printf("InputChannelOffset: %d\n", InputChannelOffset);
 }
   
  
 int dev[MAXDEVNUM];
 int found=0;
 FILE *fpout;
 int retcode;
 int ctcstatus;
 char LIB_Version[8];
 char HW_Model[16];
 char HW_Partno[8];
 char HW_Version[8];
 char HW_Serial[8];
 char Errorstring[40];
 int NumChannels;
 int HistLen;
 int Offset=0;
 double Resolution;
 int Syncrate;
 int Countrate;
 double Integralcount;
 int i,j;
 int flags;
 int warnings;
 char warningstext[16384]; //must have 16384 bytest text buffer

 HH_GetLibraryVersion(LIB_Version);
if (filename_given){
  printf("\nHydraHarp 400 HHLib.DLL Demo Application    M. Wahl, PicoQuant GmbH, 2014");
  printf("\n~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~");
  printf("\nLibrary version is %s",LIB_Version);
  if(strncmp(LIB_Version,LIB_VERSION,sizeof(LIB_VERSION))!=0)
    printf("\nWarning: The application was built for version %s.",LIB_VERSION);
 }

 if (filename_given){
   if((fpout=fopen(filename, "w"))==NULL)
     {
       printf("\ncannot open output file\n");
       goto ex;
     }
 }

 if (filename_given){
  fprintf(fpout,"Binning           : %d\n",Binning);
  fprintf(fpout,"Offset            : %d\n",Offset);
  fprintf(fpout,"AcquisitionTime   : %d\n",Tacq);
  fprintf(fpout,"SyncDivider       : %d\n",SyncDivider);
  fprintf(fpout,"SyncCFDZeroCross  : %d\n",SyncCFDZeroCross);
  fprintf(fpout,"SyncCFDLevel      : %d\n",SyncCFDLevel);
  fprintf(fpout,"SyncChannelOffset: %d\n", SyncChannelOffset);
  fprintf(fpout,"InputCFDZeroCross : %d\n",InputCFDZeroCross);
  fprintf(fpout,"InputCFDLevel     : %d\n",InputCFDLevel);
  fprintf(fpout,"InputChannelOffset: %d\n",InputChannelOffset); 
 }

 if (filename_given){
   printf("\nSearching for HydraHarp devices...");
   printf("\nDevidx     Status");
 }

 for(i=0;i<MAXDEVNUM;i++)
 {
	retcode = HH_OpenDevice(i, HW_Serial);
	if(retcode==0) //Grab any HydraHarp we can open
	{
	   if (filename_given)
		printf("\n  %1d        S/N %s", i, HW_Serial);
	   dev[found]=i; //keep index to devices we want to use
	   found++;
	}
	else
	{
	    if (filename_given){
		if(retcode==HH_ERROR_DEVICE_OPEN_FAIL)
			printf("\n  %1d        no device", i);
		else
		{
			HH_GetErrorString(Errorstring, retcode);
			printf("\n  %1d        %s", i,Errorstring);
		}
	    }
	}
 }
 
 //In this demo we will use the first HydraHarp device we find, i.e. dev[0].
 //You can also use multiple devices in parallel.
 //You can also check for specific serial numbers, so that you always know
 //which physical device you are talking to.

 if(found<1)
 {
    if (filename_given)
	printf("\nNo device available.");
    goto ex;
 }

 if (filename_given){
   printf("\nUsing device #%1d",dev[0]);
   printf("\nInitializing the device...");
 }
 
 fflush(stdout);

 retcode = HH_Initialize(dev[0],MODE_HIST,0);  //Histo mode with internal clock
 if(retcode<0)
 {
    if (filename_given)
      printf("\nHH_Initialize error %d. Aborted.\n",retcode);
    goto ex;
 }
 
 retcode = HH_GetHardwareInfo(dev[0],HW_Model,HW_Partno,HW_Version); //this is is only for information
 if(retcode<0)
 {
    if (filename_given)
      printf("\nHH_GetHardwareInfo error %d. Aborted.\n",retcode);
    goto ex;
 }
 else
   if (filename_given)
     printf("\nFound Model %s Part no %s Version %s",HW_Model,HW_Partno,HW_Version);

 retcode = HH_GetNumOfInputChannels(dev[0],&NumChannels);
 if(retcode<0)
 {
   if (filename_given)
     printf("\nHH_GetNumOfInputChannels error %d. Aborted.\n",retcode);
   goto ex;
 }
 else
   if (filename_given)
	printf("\nDevice has %i input channels.",NumChannels);
 fflush(stdout);
 
 if (filename_given)
   printf("\nCalibrating...");
 retcode=HH_Calibrate(dev[0]);
 if(retcode<0)
 {
   if (filename_given)
     printf("\nCalibration Error %d. Aborted.\n",retcode);
   goto ex;
 }

 fflush(stdout);
  
 retcode = HH_SetSyncDiv(dev[0],SyncDivider);
 if(retcode<0)
 {
   if (filename_given)
     printf("\nPH_SetSyncDiv error %d. Aborted.\n",retcode);
   goto ex;
 }

 retcode=HH_SetSyncCFD(dev[0],SyncCFDLevel,SyncCFDZeroCross);
 if(retcode<0)
 {
   if (filename_given)
     printf("\nHH_SetSyncCFD error %d. Aborted.\n",retcode);
   goto ex;
 }

 retcode = HH_SetSyncChannelOffset(dev[0],SyncChannelOffset);
 if(retcode<0)
 {
   if (filename_given)
     printf("\nHH_SetSyncChannelOffset error %d. Aborted.\n",retcode);
   goto ex;
 }

 for(i=0;i<NumChannels;i++) // we use the same input settings for all channels, you can change this
 {
	 retcode=HH_SetInputCFD(dev[0],i,InputCFDLevel,InputCFDZeroCross);
	 if(retcode<0)
	 {
	   if (filename_given)
	     printf("\nHH_SetInputCFD error %d. Aborted.\n",retcode);
	   goto ex;
	 }

	 retcode = HH_SetInputChannelOffset(dev[0],i,InputChannelOffset);
	 if(retcode<0)
	 {
	   if (filename_given)
	     printf("\nHH_SetInputChannelOffset error %d. Aborted.\n",retcode);
	   goto ex;
	 }
 }


 retcode = HH_SetHistoLen(dev[0], MAXLENCODE, &HistLen);
 if(retcode<0)
 {
   if (filename_given)
     printf("\nHH_SetHistoLen error %d. Aborted.\n",retcode);
   goto ex;
 }
 if (filename_given)
   printf("\nHistogram length is %d",HistLen);

 retcode = HH_SetBinning(dev[0],Binning);
 if(retcode<0)
 {
   if (filename_given)
     printf("\nHH_SetBinning error %d. Aborted.\n",retcode);
   goto ex;
 }

 retcode = HH_SetOffset(dev[0],Offset);
 if(retcode<0)
 {
   if (filename_given)
     printf("\nHH_SetOffset error %d. Aborted.\n",retcode);
   goto ex;
 }
 
 retcode = HH_GetResolution(dev[0], &Resolution);
 if(retcode<0)
 {
   if (filename_given)
     printf("\nHH_GetResolution error %d. Aborted.\n",retcode);
   goto ex;
 }

 if (filename_given)
   printf("\nResolution is %1.1lfps\n", Resolution);

 fflush(stdout);
 
 //Note: after Init or SetSyncDiv you must allow >400 ms for valid  count rate readings
 //Otherwise you get new values after every 100ms
 Sleep(400);

 retcode = HH_GetSyncRate(dev[0], &Syncrate);
 if(retcode<0)
 {
   if (filename_given)
     printf("\nHH_GetSyncRate error %d. Aborted.\n",retcode);
   goto ex;
 }
 if (filename_given)
   printf("\nSyncrate=%1d/s", Syncrate);

 for(i=0;i<NumChannels;i++) // for all channels
 {
	 retcode = HH_GetCountRate(dev[0],i,&Countrate);
	 if(retcode<0)
	 {
	   if (filename_given)
	     printf("\nHH_GetCountRate error %d. Aborted.\n",retcode);
	   goto ex;
	 }
	 if (filename_given)
	   printf("\nCountrate[%1d]=%1d/s", i, Countrate);
 }

 if (filename_given)
   printf("\n");

 //new from v1.2: after getting the count rates you can check for warnings
 retcode = HH_GetWarnings(dev[0],&warnings);
 if(retcode<0)
 {
   if (filename_given)
     printf("\nHH_GetWarnings error %d. Aborted.\n",retcode);
   goto ex;
 }
 if(warnings)
 {
	 HH_GetWarningsText(dev[0],warningstext, warnings);
	 if (filename_given)
	   printf("\n\n%s",warningstext);
 }
	 

 retcode = HH_SetStopOverflow(dev[0],0,10000); //for example only
 if(retcode<0)
 {
   if (filename_given)
     printf("\nHH_SetStopOverflow error %d. Aborted.\n",retcode);
   goto ex;
 }

 fflush(stdout);
	 
 HH_ClearHistMem(dev[0]);
 if(retcode<0)
   {
     if (filename_given)
       printf("\nHH_ClearHistMem error %d. Aborted.\n",retcode);
     goto ex;
   }

 retcode = HH_GetSyncRate(dev[0], &Syncrate);
 if(retcode<0)
   {
     if (filename_given)
       printf("\nHH_GetSyncRate error %d. Aborted.\n",retcode);
     goto ex;
   }
 if (filename_given)
   printf("\nSyncrate=%1d/s", Syncrate);

 for(i=0;i<NumChannels;i++) // for all channels
   {
     retcode = HH_GetCountRate(dev[0],i,&Countrate);
     if(retcode<0)
       {
	 if (filename_given)
	   printf("\nHH_GetCountRate error %d. Aborted.\n",retcode);
	 goto ex;
       }
     if (filename_given)
       printf("\nCountrate[%1d]=%1d/s", i, Countrate);
   }

 //here you could check for warnings again
        
 retcode = HH_StartMeas(dev[0],Tacq);
 if(retcode<0)
   {
     if (filename_given)
       printf("\nHH_StartMeas error %d. Aborted.\n",retcode);
     goto ex;
   }

 if (filename_given)
   printf("\n\nMeasuring for %1d milliseconds...",Tacq);
        
 ctcstatus=0;
 while(ctcstatus==0)
   {
     retcode = HH_CTCStatus(dev[0], &ctcstatus);
     if(retcode<0)
       {
	 if (filename_given)
	   printf("\nHH_StartMeas error %d. Aborted.\n",retcode);
	 goto ex;
       }
   }
         
 retcode = HH_StopMeas(dev[0]);
 if(retcode<0)
   {
     if (filename_given)
       printf("\nHH_StopMeas error %1d. Aborted.\n",retcode);
     goto ex;
   }

 if (filename_given)
   printf("\n");
 
 for(i=0;i<NumChannels;i++) // for all channels
   {
     retcode = HH_GetHistogram(dev[0],counts[i],i,0);
     if(retcode<0)
       {
	 if (filename_given)
	   printf("\nHH_GetHistogram error %1d. Aborted.\n",retcode);
	 goto ex;
       }

     Integralcount = 0;
     for(j=0;j<HistLen;j++)
       Integralcount+=counts[i][j];

     if (filename_given)
       printf("\n  Integralcount[%1d]=%1.0lf",i,Integralcount);
   }
 if (filename_given)
   printf("\n");

 retcode = HH_GetFlags(dev[0], &flags);
 if(retcode<0)
   {
     if (filename_given)
       printf("\nHH_GetFlags error %1d. Aborted.\n",flags);
     goto ex;
   }

 if (filename_given)
   if(flags&FLAG_OVERFLOW) printf("\n  Overflow.");
 
 for(j=0;j<HistLen;j++)
 {
	for(i=0;i<NumChannels;i++)
	  if (filename_given)
	    fprintf(fpout,"%5d ",counts[i][j]);
	  else
	    printf("%5d ",counts[i][j]);
	    
	if (filename_given)
	  fprintf(fpout,"\n");
	else
	  printf("\n");
 }
 
 if (filename_given)
   fprintf(fpout,"\n");

ex:
 for(i=0;i<MAXDEVNUM;i++) //no harm to close all
 {
	HH_CloseDevice(i);
 }
 if(fpout)
   fclose(fpout);
 
 return 0;
}


