// split_rawdata.cpp : Defines the entry point for the console application.
//

#include "stdio.h"
#include <vector>

using namespace std;
typedef struct sStreamFileInfo
{
	int index;
	FILE *pDataFile;
	FILE *pInfoFile;
}StreamFileInfo;

#define _32BIT 0
#if _32BIT
typedef unsigned long element;
#else
typedef unsigned long long element;
#endif

unsigned char GetNalType(unsigned char* pBuf, int iLen)
{
	while ((*pBuf==0) && (iLen>0))
	{
		pBuf ++;
		iLen --;
	}
	if ((*pBuf == 1) && (iLen>2))
	{
		return pBuf[1] & 0x1f;
	}
    return pBuf[1];
}

int mainaa(int argc, char* argv[])
{
	FILE* pInfoFile = NULL;
	FILE* pDataFile = NULL;
	FILE* pOutputFile = NULL;
	FILE* pOutputInfoFile = NULL;
	vector<StreamFileInfo> FileList;

	pInfoFile = fopen("Encode2RTP_layer0_dump.info","rb");
	pDataFile = fopen("Encode2RTP_layer0_dump.data","rb");

	element ulIndex=0;
	element ulNow;
	element ulSize;
	element codecType;
	element ulTimestamp,prev_timestamp;
	int layer_num;
	int layer_index;
	int layer_width;
	int layer_height;
	int nal_ref_idc;
	int did;
	int tid;
	int max_tid;
	int frameType;
	int frameIdc;
	int priority;
	int max_priority;
	unsigned char *buffer = new unsigned char[10*1024*1024];
	unsigned long long sizefile = ftell(pInfoFile);
	unsigned long streamNum = 0;
	unsigned long frameNum = 0;
	bool bFound = false;
	prev_timestamp = 0;
	while(!feof(pInfoFile) && !feof(pDataFile))
	{
		fread(&ulIndex, sizeof(ulIndex), 1,pInfoFile);
		fread(&ulNow, sizeof(ulNow), 1, pInfoFile);
		fread(&ulSize, sizeof(ulSize), 1, pInfoFile);
		fread(&codecType, sizeof(codecType), 1, pInfoFile);
		fread(&ulTimestamp, sizeof(ulTimestamp), 1,pInfoFile);
		fread(&layer_num, sizeof(layer_num), 1,pInfoFile);
		fread(&layer_index, sizeof(layer_index), 1,pInfoFile);
		fread(&layer_width, sizeof(layer_width), 1,pInfoFile);
		fread(&layer_height, sizeof(layer_height), 1, pInfoFile);
		fread(&nal_ref_idc, sizeof(nal_ref_idc), 1,pInfoFile);
		fread(&did, sizeof(did), 1,pInfoFile);
		fread(&tid, sizeof(tid), 1,pInfoFile);
		fread(&max_tid, sizeof(max_tid), 1,pInfoFile);
		fread(&frameType, sizeof(frameType), 1,pInfoFile);
		fread(&frameIdc, sizeof(frameIdc), 1,pInfoFile);
		fread(&priority, sizeof(priority), 1,pInfoFile);
		fread(&max_priority, sizeof(max_priority), 1,pInfoFile);

		fread(buffer,1,ulSize, pDataFile);
		if (FileList.size() == 0)
		{
			StreamFileInfo info;
			memset(&info,0,sizeof(StreamFileInfo));
			info.index = streamNum++;
			char filename[40];
			sprintf(filename,"Encode2RTP_layer0_dump%d.data",info.index);
			info.pDataFile = fopen(filename, "wb");
			sprintf(filename,"Encode2RTP_layer0_dump%d.info",info.index);
			info.pInfoFile = fopen(filename, "wb");
			FileList.push_back(info);
			pOutputFile = info.pDataFile;
			pOutputInfoFile = info.pInfoFile;
		}
		fprintf(pOutputInfoFile,":%d,%lld,%d,%d\n",frameType, ulTimestamp, frameIdc, ulSize);
		if (ulTimestamp != prev_timestamp)
		{
			if (ulTimestamp-prev_timestamp > 50)
			{
				printf("%ld,%lld\n",frameNum,ulTimestamp-prev_timestamp);
			}
			prev_timestamp = ulTimestamp;
			frameNum++;
		}
		fwrite(buffer,1,ulSize, pOutputFile);
	}

	delete []buffer;
	fclose(pDataFile);
	fclose(pInfoFile);
	for (int i=0;i<FileList.size();i++)
	{
		fclose(FileList[i].pDataFile);
		fclose(FileList[i].pInfoFile);
	}

	return 0;
}
