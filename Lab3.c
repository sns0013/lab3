
/*****************************************************************************\
* Laboratory Exercises COMP 3510                                              *
* Author: Saad Biaz                                                           *
* Date  : March 5, 2013                                                       *
\*****************************************************************************/

/*****************************************************************************\
*                             Global system headers                           *
\*****************************************************************************/
#include "common3.h"



/*****************************************************************************\
*                             Global data types                               *
\*****************************************************************************/



/*****************************************************************************\
*                             Global definitions                              *
\*****************************************************************************/
#define MAX_QUEUE_SIZE 2


/*****************************************************************************\
*                            Global data structures                           *
\*****************************************************************************/




/*****************************************************************************\
*                                  Global data                                *
\*****************************************************************************/
Event Queue[MAX_QUEUE_SIZE][MAX_NUMBER_DEVICES];
int NextService[MAX_NUMBER_DEVICES] = {0};
int NextStore[MAX_NUMBER_DEVICES] = {0};
int TotalEvents, TotalDeviceServed, SumME, NumberDevices, LowestDevNum;
double AverageME, AverageRT, AverageTT, TotalRT, TotalTT;
double AverageDRT, AverageDTT, AverageDME;
int DeviceToServe[MAX_NUMBER_DEVICES] = {0};
int DeviceServed[MAX_NUMBER_DEVICES] = {0};
int MissedEvents[MAX_NUMBER_DEVICES] = {0};
double SumRT[MAX_NUMBER_DEVICES] = {0};
double SumTT[MAX_NUMBER_DEVICES] = {0};


/*****************************************************************************\
*                               Function prototypes                           *
\*****************************************************************************/

void Control(void);
void InterruptRoutineHandlerDevice(void);
void BookKeeping();


/*****************************************************************************\
* function: main()                                                            *
* usage:    Create an artificial environment for embedded systems. The parent *
*           process is the "control" process while children process will gene-*
*           generate events on devices                                        *
*******************************************************************************
* Inputs: ANSI flat C command line parameters                                 *
* Output: None                                                                *
*                                                                             *
* INITIALIZE PROGRAM ENVIRONMENT                                              *
* START CONTROL ROUTINE                                                       *
\*****************************************************************************/

int main (int argc, char **argv) {

   if (Initialization(argc,argv)){
     Control();
   }
} /* end of main function */

/***********************************************************************\
* Input : none                                                          *
* Output: None                                                          *
* Function: Monitor Devices and process events (written by students)    *
\***********************************************************************/
void Control(void){
    Timestamp TT;
    int DeviceNum = 0;

    while(1){
        if(LowestDevNum < DeviceNum){
         	//printf("OUTBefore***********%d****************\n", DeviceNum);           	
            DeviceNum = LowestDevNum;
            //printf("OUTAfter***********%d****************\n", DeviceNum);
            LowestDevNum = MAX_NUMBER_DEVICES;
        }
        while(DeviceToServe[DeviceNum]){
            Event S = Queue[NextService[DeviceNum]][DeviceNum];
            Server(&Queue[NextService[DeviceNum]][DeviceNum]);
            DeviceServed[DeviceNum]++;
            //printf(">>>>>Serve>>>> %d\n", DeviceNum);
            NextService[DeviceNum] = (NextService[DeviceNum] + 1) & MAX_QUEUE_SIZE;
            SumTT[DeviceNum] += (Now() - S.When);

            DeviceToServe[DeviceNum]--;
            
            if(LowestDevNum < DeviceNum){
             	//printf("INBefore***********%d****************\n", DeviceNum);           	
            	DeviceNum = LowestDevNum;
            	//printf("INAfter***********%d****************\n", DeviceNum);
            	LowestDevNum = MAX_NUMBER_DEVICES;
        	}
        }
        DeviceNum = (DeviceNum + 1) & 0x1F;
    }
}


/***********************************************************************\
* Input : None                                                          *
* Output: None                                                          *
* Function: This routine is run whenever an event occurs on a device    *
*           The id of the device is encoded in the variable flag        *
\***********************************************************************/
void InterruptRoutineHandlerDevice(void){
    // printf("An event occurred at %f  Flags = %d \n", Now(), Flags);
	Timestamp RT;
	Status tempFlags = Flags;
	Flags = 0;
    NumberDevices = 0;
    LowestDevNum = MAX_NUMBER_DEVICES;
	while(tempFlags){
        if(tempFlags & 1){
            Event E = BufferLastEvent[NumberDevices];
            Queue[NextStore[NumberDevices]][NumberDevices] = E;
            NextStore[NumberDevices] = (NextStore[NumberDevices] + 1) & MAX_QUEUE_SIZE;
            DisplayEvent('E', &E);
            SumRT[NumberDevices] += (Now() - E.When);
            DeviceToServe[NumberDevices]++;
            if(NumberDevices < LowestDevNum){
            	LowestDevNum = NumberDevices;
        	}  
        }
        tempFlags = tempFlags >> 1;
        NumberDevices++;
    }
}


/***********************************************************************\
* Input : None                                                          *
* Output: None                                                          *
* Function: This must print out the number of Events buffered not yet   *
*           not yet processed (Server() function not yet called)        *
\***********************************************************************/
void BookKeeping(void){
  // For EACH device, print out the following metrics :
  // 1) the percentage of missed events, 2) the average response time, and
  // 3) the average turnaround time.
  // Print the overall averages of the three metrics 1-3 above
    printf("\n >>>>>> Done\n");
    int DeviceNum = MAX_NUMBER_DEVICES - 1;

    // Individual Diagnostics
    while(DeviceNum + 1){

    	if(BufferLastEvent[DeviceNum].EventID > 1){
       		TotalEvents += (BufferLastEvent[DeviceNum].EventID + 1);
       		printf("Seen: %d\n", BufferLastEvent[DeviceNum].EventID + 1);
       		if(DeviceServed[DeviceNum] > 1){
       			MissedEvents[DeviceNum] = (BufferLastEvent[DeviceNum].EventID + 1) - DeviceServed[DeviceNum];
       		}        		
    	}
 
        TotalRT += SumRT[DeviceNum];                                                                            // Sum of all Response Time
        TotalTT += SumTT[DeviceNum];                                                                            // Sum of all Turnaround Time
        AverageDRT = SumRT[DeviceNum]/DeviceServed[DeviceNum];                                                  // Average Response Time for each Device
        AverageDTT = SumTT[DeviceNum]/DeviceServed[DeviceNum];                                                  // Average Turnaround Time for each Device
        
        if(DeviceServed[DeviceNum] > 0){
        	printf("Served: %d\n", DeviceServed[DeviceNum]);
            TotalDeviceServed += DeviceServed[DeviceNum];                                                       // Total Number of Processed Events
            //MissedEvents[DeviceNum] = (BufferLastEvent[DeviceNum].EventID + 1) - DeviceServed[DeviceNum];       // Number of Missed Events for each Device
            AverageDME = MissedEvents[DeviceNum]*100/(BufferLastEvent[DeviceNum].EventID + 1);                  // Average Percent Missed Events for each Device
            printf("Device: [%d] --- Percent ME: [%d] --- Average RT: [%f] --- Average TT: [%f]\n", DeviceNum, MissedEvents[DeviceNum], AverageDRT, AverageDTT);
        }

        DeviceNum--;

    }
    printf("TotalEvents: %d\n", TotalEvents);
    // Totals Diagnostics
    SumME = TotalEvents - TotalDeviceServed;                                                                    // Total Number of Missed Events
    printf("MissedEvents: %d\n", SumME);
    AverageME = SumME/TotalEvents * 100;                                                                          // Average Number of all Missed Events
    AverageRT = TotalRT/TotalEvents;                                                                            // Average Total Response Time
    AverageTT = TotalTT/TotalEvents;                                                                            // Average Total Turnaround Time
    printf("\n\nTOTALS:\n");
    printf("Queue Size: [%d]",MAX_QUEUE_SIZE);
    printf("\nAverage ME: [%f]\nAverage RT: [%f]\nAverage TT: [%f]\n", AverageME, AverageRT, AverageTT);
}




