#include "fat12.h"
#include "../Utils.h"
#include "../FileManager.h"
#include "../file.h"
#include "../UserUtils.h"
#include "../satadriver.h"
#include "../atapi.h"
#include "../fat32/FAT32.h"
#include "../atapi.h"
#include "../malloc.h"

FAT12DBR gFat12Dbr;

DWORD gFat12FatSecOff;

DWORD gFat12RootDirSecOff;

DWORD gFat12DataSecOff;

DWORD gFat12ClusterSize;

DWORD gFat12RootSecCnt;

#define FAT12_FIRST_CLUSTER_NO 2

int gFloppyDev = -1;

DWORD gFat12FatBase = 0;

DWORD gFat12RootDirBase = 0;


int readFat12Dirs(DWORD secno, LPFILEBROWSER files) {
	int cnt = 0;
	int iret = 0;

	char fattmpbuf[2048];

	iret = vm86ReadFloppy(getCylinder(secno),getHeader(secno),getSector(secno), 1, (char*)fattmpbuf, gFloppyDev);
	if (iret <= 0)
	{
		__drawGraphChars((unsigned char*)"read floppy first sector error\n", 0);
		return FALSE;
	}

	LPFAT32DIRECTORY dir = (LPFAT32DIRECTORY)fattmpbuf;
	int dircnt = gFat12Dbr.BPB_BytesPerSec/sizeof(FAT32DIRECTORY);
	for (int i = 0; i < dircnt; i++)
	{
		if (dir->attr != 0x0f && *dir->mainName)
		{
			if (*dir->suffixName != 0x20)
			{
				__memcpy(files->pathname, (char*)dir->mainName, 8);
				*(files->pathname + 8) = 0;
				__strcat(files->pathname, ".");

				__memcpy(files->pathname + 9, (char*)dir->suffixName, 3);
				*(files->pathname + 12) = 0;
			}
			else {
				__memcpy(files->pathname, (char*)dir->mainName, 11);
				*(files->pathname + 11) = 0x20;
				*(files->pathname + 12) = 0;
			}

			files->secno = dir->clusterLow + ((DWORD)dir->clusterHigh << 16);
			files->attrib = dir->attr;
			files->filesize = dir->size;

			files++;

			cnt++;
		}
	}

	//__free((DWORD)fattmpbuf);

	return cnt;
}



int browseFat12File(LPFILEBROWSER files) {
	int iret = 0;
	char szout[1024];

	//gFloppyDev = 0;

	//��չint13h�޷���ȡ���̣�Ϊʲô��
	if (gFloppyDev == -1)
	{
		//floppy not support 0x42xx int13h
		gFloppyDev = getAtapiDev(0, 0x7f);
		if (gFloppyDev == -1)
		{
			__drawGraphChars((unsigned char*)"not found floppy device\n", 0);
			return FALSE;
		}
		else {
			__printf(szout, "find floppy device:%x\n", gFloppyDev);

		}
	}
	
	iret = vm86ReadFloppy(getCylinder(0), getHeader(0), getSector(0), 2, (char*)&gFat12Dbr, gFloppyDev);
	if (iret <= 0)
	{
		gFloppyDev = 1;
		iret = vm86ReadFloppy(getCylinder(0), getHeader(0), getSector(0), 2, (char*)&gFat12Dbr, gFloppyDev);
		if (iret <= 0)
		{
			__drawGraphChars((unsigned char*)"read floppy dbr sector error\n", 0);
			return FALSE;
		}
	}

	if (__memcmp((CHAR*)&gFat12Dbr.BS_FileSysType,"FAT12",5))
	{
		__drawGraphChars((unsigned char*)"read floppy dbr sector format error\n", 0);
		return FALSE;
	}

	gFat12ClusterSize = gFat12Dbr.BPB_SecPerClus*gFat12Dbr.BPB_BytesPerSec;	//512

	int rootdirsize = gFat12Dbr.BPB_RootEntCnt * sizeof(FAT32DIRECTORY);		//1c00
	int rootdirseccnt = rootdirsize / gFat12Dbr.BPB_BytesPerSec;				//14
	int mod = rootdirsize % gFat12Dbr.BPB_BytesPerSec;
	if (mod)
	{
		rootdirseccnt++;
	}
	gFat12RootSecCnt = rootdirseccnt;	//14

	gFat12FatSecOff = gFat12Dbr.BPB_HiddSec + gFat12Dbr.BPB_RsvdSecCnt;	//1

	gFat12RootDirSecOff = gFat12FatSecOff + gFat12Dbr.BPB_FATSz16*gFat12Dbr.BPB_NumFATs;		//19
	gFat12DataSecOff = gFat12RootDirSecOff + rootdirseccnt;						//33

	if (gFat12FatBase == 0)
	{
		gFat12FatBase = __kMalloc(0x10000);
	}

	iret = vm86ReadFloppy(getCylinder(gFat12FatSecOff), getHeader(gFat12FatSecOff), getSector(gFat12FatSecOff),
		gFat12Dbr.BPB_FATSz16, (char*)gFat12FatBase, gFloppyDev);
	if (iret <= 0)
	{
		__drawGraphChars((unsigned char*)"read floppy fat sector error\n", 0);
		return FALSE;
	}

	if (gFat12RootDirBase == 0)
	{
		gFat12RootDirBase = (DWORD)__kMalloc(0x10000);
	}

	iret = vm86ReadFloppy(getCylinder(gFat12RootDirSecOff),getHeader(gFat12RootDirSecOff),getSector(gFat12RootDirSecOff),
		gFat12RootSecCnt, (char*)gFat12RootDirBase, gFloppyDev);
	if (iret <= 0)
	{
		__drawGraphChars((unsigned char*)"read floppy root dir sector error\n", 0);
		return FALSE;
	}

	int cnt = 0;

	LPFAT32DIRECTORY dir = (LPFAT32DIRECTORY)gFat12RootDirBase;

	for (int i = 0; i < gFat12Dbr.BPB_RootEntCnt; i ++)
	{
		if (dir->attr != 0x0f && (dir->attr == FILE_ATTRIBUTE_ARCHIVE || dir->attr == FILE_ATTRIBUTE_DIRECTORY) && *dir->mainName)
		{
			if (*dir->suffixName != 0x20)
			{
				__memcpy(files->pathname, (char*)dir->mainName, 8);
				*(files->pathname + 8) = 0;
				__strcat(files->pathname, ".");

				__memcpy(files->pathname + 9, (char*)dir->suffixName, 3);
				*(files->pathname + 12) = 0;
			}
			else {
				__memcpy(files->pathname, (char*)dir->mainName, 11);
				*(files->pathname + 11) = 0x20;
				*(files->pathname + 12) = 0;
			}

			files->secno = (dir->clusterLow + ((DWORD)dir->clusterHigh << 16)) /*- FAT12_FIRST_CLUSTER_NO*/;
			files->attrib = dir->attr;
			files->filesize = dir->size;

			files++;

			cnt++;
		}
		dir++;
	}

	return cnt;
}



int fat12FileReader(DWORD clusterno,int filesize, char * lpdata, int readsize) {

	int readoksize = 0;

	int ret = 0;
	if (readsize > filesize)
	{
		readsize = filesize;
	}
	else if (readsize <= 0)
	{
		return FALSE;
	}

	int readtimes = readsize / gFat12Dbr.BPB_SecPerClus;
	int readmod = readsize % gFat12Dbr.BPB_SecPerClus;
	if (readmod)
	{
		readtimes++;
	}

	for (int i = 0; i < readtimes; i++)
	{
		DWORD sectorno = gFat12DataSecOff + (clusterno - FAT12_FIRST_CLUSTER_NO) * gFat12Dbr.BPB_SecPerClus;

		ret = vm86ReadFloppy(getCylinder(sectorno), getHeader(sectorno), getSector(sectorno),
			gFat12Dbr.BPB_SecPerClus, lpdata, gFloppyDev);
		if (ret )
		{
			lpdata += gFat12ClusterSize;

			readoksize += gFat12ClusterSize;
		}
		else {
			__drawGraphChars((unsigned char*)"fat12 read cluster error\n", 0);
			break;
		}

		clusterno = getNextFAT12Cluster(clusterno);
		if (clusterno == FALSE)
		{
			break;
		}
	}

	return readoksize;
}


int getNextFAT12Cluster(int clusterno) {

	char * next = (clusterno * 3) / 2 + (char*)gFat12FatBase;

	int mod = clusterno * 3 % 2;
	if (mod)
	{
		unsigned int low = *next >> 4;
		unsigned int high = *(next + 1) << 4;
		unsigned int nextclusterno = high + low;
		if (nextclusterno >= 0xff8)
		{
			return FALSE;
		}
		else {
			return nextclusterno;
		}
	}
	else {
		unsigned int low = *next;
		unsigned int high = (*(next + 1)&0xf) << 8;
		unsigned int nextclusterno = high + low;
		if (nextclusterno >= 0xff8)
		{
			return FALSE;
		}
		else {
			return nextclusterno;
		}
	}
}


//LBA(�߼�������)=��ͷ�� �� ÿ�ŵ������� �� ��ǰ��������� + ÿ�ŵ������� �� ��ǰ���ڴ�ͷ�� + ��ǰ���������� �C 1

int getCylinder(int lba) {
	return lba / (SPT * HPC);
}

int getHeader(int lba) {
	return  (lba / SPT) % HPC;
}


int getSector(int lba) {
	return lba % SPT + 1;
}



int vm86ReadFloppy(int c,int h,int s,short seccnt, char * buf, int disk) {

	//return vm86ReadBlock(c*h*s - 1, 0, seccnt, buf, disk, BYTES_PER_SECTOR);

	LPV86VMIPARAMS params = (LPV86VMIPARAMS)V86VMIPARAMS_ADDRESS;
	while (params->bwork == 1)
	{
		__sleep(0);
	}

	//__drawGraphChars((unsigned char*)"vm86ReadFloppy bwork is 0,work start\n", 0);

	params->intno = 0x13;
	params->reax = 0;
	params->recx = 0;
	params->redx = 0;
	params->rebx = 0;
	params->resi = 0;
	params->redi = 0;
	params->res = 0;
	params->rds = 0;
	params->result = 0;

	params->bwork = 1;

	while (params->bwork == 1)
	{
		__sleep(0);
	}

	params->intno = 0x13;
	params->reax = seccnt | 0x0200;
	params->recx = s | (c<<8);
	params->redx = disk | (h<<8);
	params->rebx = INT13_RM_FILEBUF_OFFSET;
	params->resi = 0;
	params->redi = 0;
	params->res = INT13_RM_FILEBUF_SEG;
	params->rds = 0;
	params->result = 0;

// 	LPINT13PAT pat = (LPINT13PAT)V86VMIDATA_ADDRESS;
// 	pat->len = 0x10;
// 	pat->reserved = 0;
// 	pat->seccnt = seccnt;
// 	pat->segoff = (INT13_RM_FILEBUF_SEG << 16) + INT13_RM_FILEBUF_OFFSET;
// 	pat->secnolow = secno;
// 	pat->secnohigh = secnohigh;

	params->bwork = 1;

	while (params->bwork == 1)
	{
		__sleep(0);
	}

	//__drawGraphChars((unsigned char*)"bwork is 0,work complete\n", 0);

	int result = 0;

	if (params->result > 0)
	{
		__memcpy(buf, (char*)INT13_RM_FILEBUF_ADDR, seccnt * BYTES_PER_SECTOR);
		result = seccnt * BYTES_PER_SECTOR;
	}

	return result;
}


/*
; �� LBA ���� CHS
mov dl, 18
div dl
mov ch, al
mov dh, al
mov cl, ah
shr ch, 1
inc cl
and dh, 1

; ��һ������
mov ax, 0x0201
xor dl, dl
int 13h
*/

/*
��ڲ�����
��ah��= int 13H �Ĺ��ܺţ�2��ʾ��������3��ʾд������
��al��= ��ȡ/д���������
��ch��= �ŵ���
��cl��= ������
��dh��= ��ͷ�ţ��������̼���ţ�������һ������һ����ͷ����д��
��dl��= ��������  ������0��ʼ��0������A  1������B Ӳ�̴�80H��ʼ��80H��Ӳ��C 81H�� Ӳ��D
es:bx ָ����մ������������ݵ��ڴ���/ָ��д����̵�����

���ز�����
�����ɹ�����ah��= 0 ����al��= ��ȡ/д���������
����ʧ�ܣ���ah��= �������
*/