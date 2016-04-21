#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <string>

#include "WbxAeJitterPolicy.h"
#include "WbxAeJitterBuffer.h"
#include "WbxAeJitterPolicyTrain.h"
#include "WbxAeJitterPolicyDolphin10.h"
#include "WbxAeJitterPolicyModify.h"
#include "safe_mem_lib.h"

int g_AETraceLevel = 0;

void usage()
{
		printf("exe input.csv outputwme.csv mode(0-wme, 1-train, 2- Dolphin 10, 3 - Origin)\n");
}
int main(int argc, char* argv[])
{
  usage();
  printf("Current Input argc:%d\n", argc);
  for(int i =0; i< argc; i++)
  printf("This %d's argv is %s \n", i, argv[i]);
	if (argc < 4)
	{
    usage();
    printf("Please enter the right parameters\n");
    return  -1;
	}
  
	FILE* OriginFile = NULL;
	FILE* ResultFile = NULL;
	PolicyUTDataStructure inputData;
	PolicyUTDataStructure resultData;

	
  
  OriginFile = fopen(argv[1], "rb");
  if (OriginFile == NULL) {

    printf("Open %s failed.", argv[1]);
  }
	ResultFile = fopen(argv[2], "wb");
  if ( ResultFile == NULL) {
    printf("Open %s failed.", argv[2]);
    fclose(OriginFile);
    OriginFile = NULL;
  }

  int mode = atoi(argv[3]);
  if ( mode < 0 || mode > 3) {
    mode = 0;
  }
  
	IWbxJitterPolicy* testPolicy =  NULL; // for WME and Dolphin 10
  if (mode == 0) {
    	CreateIWbxJitterPolicy(&testPolicy);
  }
  if (mode ==2 ) {
    CreateIWbxJitterPolicyDolphin10(&testPolicy);
  }
  if (mode == 3) {
    CreateIWbxJitterPolicyOrigin(&testPolicy);
  }

  
  IWbxJitterPolicyTrain* testPolicyTrain = NULL;
  if (mode == 1) {
      CreateIWbxJitterPolicyTrain(&testPolicyTrain);
  }

  
  
  
  int readcount = 0;
  
	if (OriginFile && ResultFile && (testPolicy || testPolicyTrain ))
	{
    if (testPolicy) {
      testPolicy->SetDelay(WBXAE_AJB_DEFAULT_MIN_DELAY * 2, WBXAE_AJB_DEFAULT_MAX_DELAY);
      
    }
    if (testPolicyTrain) {
      testPolicyTrain->SetDelay(WBXAE_AJB_DEFAULT_MIN_DELAY * 2, WBXAE_AJB_DEFAULT_MAX_DELAY);
    }

		while (!feof(OriginFile))
		{
			cisco_memset_s(&inputData, sizeof(PolicyUTDataStructure),0);
      
			fscanf(OriginFile,"%u,%u,%u,%u,%u,%u,%d,%d\n", &inputData.ssrc, &inputData.samplesPer10MS, &inputData.receivedTime, &inputData.rtpSequce, &inputData.rtpTimestamp, &inputData.playTime, &inputData.averageDelay, &inputData.delayedPlay);
      
			cisco_memcpy_s(&resultData, sizeof(PolicyUTDataStructure), &inputData, sizeof(PolicyUTDataStructure));
      
      
      // Test WME
      if (testPolicy) {
        testPolicy->JitterPolicy(resultData.ssrc, resultData.samplesPer10MS, resultData.receivedTime, resultData.rtpSequce,resultData.rtpTimestamp, resultData.playTime);
        resultData.delayedPlay = resultData.playTime - resultData.receivedTime;
        resultData.averageDelay = testPolicy->GetAverageDelay();
      }
      
      if (testPolicyTrain) {
        // Test Train
        readcount++;
        if (readcount == 1) {
          testPolicyTrain->JitterPolicy(resultData.ssrc, resultData.samplesPer10MS, resultData.receivedTime, resultData.rtpSequce,resultData.rtpTimestamp, true, resultData.playTime);
          
        }
        else{
          testPolicyTrain->JitterPolicy(resultData.ssrc, resultData.samplesPer10MS, resultData.receivedTime, resultData.rtpSequce,resultData.rtpTimestamp, false, resultData.playTime);
          
        }
        resultData.delayedPlay = resultData.playTime - resultData.receivedTime;
        resultData.averageDelay = testPolicyTrain->GetAverageDelay();
   
      }

      char writedata[250];
      sprintf(writedata, "%u,%u,%u,%u,%u,%u,%d,%d\n", resultData.ssrc, resultData.samplesPer10MS, resultData.receivedTime, resultData.rtpSequce, resultData.rtpTimestamp, resultData.playTime, resultData.averageDelay, resultData.delayedPlay);
      fwrite(writedata, sizeof(char), strlen(writedata), ResultFile);
      
      

		}
	}
	if (OriginFile)
	{
		fclose(OriginFile);
		OriginFile = NULL;
	}

	if (ResultFile)
	{
		fclose(ResultFile);
		ResultFile = NULL;
	}

  if (testPolicyTrain) {
    delete testPolicyTrain;
    testPolicyTrain = NULL;
  }
  if (testPolicy) {
    delete testPolicy;
    testPolicy = NULL;
  }
  printf("Processed end!");
	return 0;
}