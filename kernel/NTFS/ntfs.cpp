#include "ntfs.h"
#include "../file.h"
#include "../satadriver.h"
#include "ntfsFile.h"
#include "../Utils.h"
#include "../video.h"

NTFSDBR				gNtfsDbr;

unsigned __int64	gMsfOffset;

char		gMsfRoot[MFTEntrySize];


int initNTFS() {
	int ret = 0;

	readFile = readNtfsFile;
	writeFile = writeNtfsFile;

	ret = getNtfsDBR();

	g_bytesPerSec = gNtfsDbr.bytePerSector;
	g_SecsPerCluster = gNtfsDbr.secPerCluster;
	g_ClusterSize = g_SecsPerCluster * g_bytesPerSec;
	gMsfOffset = gNtfsDbr.hideSectors + (DWORD)gNtfsDbr.MFT * g_SecsPerCluster;

	ret = readMSFRoot();

#ifdef LIUNUX_DEBUG_FLAG
	unsigned char szout[4096];
	__printf((char*)szout,"ntfs bytes per sector:%x,cluster size:%x,sector per cluster:%x\n",
		g_bytesPerSec,g_ClusterSize,g_SecsPerCluster);
	__drawGraphChars((unsigned char*)szout, 0);
	__memset((char*)szout, 0, 4096);
	__dump((char*)&gNtfsDbr, BYTES_PER_SECTOR, 0, szout);
	__drawGraphChars(szout, 0);
#endif

	return 0;
}



int getNtfsDBR() {
	int ret = 0;
	unsigned int partitionSize[4];

	for (int i = 0; i < 4; i ++ )
	{
		partitionSize[i] = gMBR.dpt[i].sectortotal;
	}
	
	if (partitionSize[0] > partitionSize[1])
	{
		unsigned int secno = gMBR.dpt[0].offset;
		int ret = readSector(secno, 0, 1, (char*)&gNtfsDbr);
	}
	else {
		unsigned int secno = gMBR.dpt[1].offset;
		int ret = readSector(secno, 0, 1, (char*)&gNtfsDbr);
	}

	return ret;
}



int readMSFRoot() {
	
	DWORD low = (gMsfOffset + MSF_ROOTDIR_OFFSET) & 0xffffffff;
	DWORD high = (gMsfOffset + MSF_ROOTDIR_OFFSET) >> 32;
	int ret = readSector(low,high, 2, gMsfRoot);
	return ret;
}




