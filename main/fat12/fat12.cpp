#include "fat12.h"
#include "Utils.h"
#include "../FileBrowser.h"
#include "file.h"

#include "ata.h"
#include "atapi.h"
#include "fat32/FAT32.h"
#include "VM86.h"
#include "malloc.h"
#include "floppy.h"


#define FAT12_FIRST_CLUSTER_NO		2

#ifdef VM86_PROCESS_TASK
//#define FLOPPY_INT13_READWRITE
//#define FLOPPY_INT255_READWRITE
#endif


FAT12DBR * gFat12Dbr;

DWORD gFat12FatSecOff;

DWORD gFat12RootDirSecOff;

DWORD gFat12DataSecOff;

DWORD gFat12ClusterSize;

DWORD gFat12RootSecCnt;

DWORD gFat12FatBase = 0;

DWORD gFat12RootDirBase = 0;

int gFloppyDev = -1;



int readFat12Dirs(DWORD clsnum, LPFILEBROWSER files) {
	int cnt = 0;
	int iret = 0;

	char * fattmpbuf = (char*)FLOPPY_DMA_BUFFER;
	char szout[1024];
#ifdef FLOPPY_INT13_READWRITE
	iret = vm86ReadFloppy(getCylinder(secno), getHeader(secno), getSector(secno), 1, (char*)fattmpbuf, gFloppyDev);
#else
	iret = readFloppySector(gFloppyDev, (unsigned long)fattmpbuf, clsnum, 2);
#endif
	if (iret <= 0)
	{
		__printf(szout,( char*)"read floppy first sector error\n");
		return FALSE;
	}

	LPFAT32DIRECTORY dir = (LPFAT32DIRECTORY)fattmpbuf;
	int dircnt = gFat12Dbr->BPB_BytesPerSec/sizeof(FAT32DIRECTORY);
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
			if (dir->attr == 0) {
				files->attrib = FILE_ATTRIBUTE_ARCHIVE;
			}
			files->filesize = dir->size;

			files++;

			cnt++;
		}
	}

	return cnt;
}



int browseFat12File(LPFILEBROWSER files) {
	gFat12Dbr = (FAT12DBR*)FLOPPY_DBR_BUFFER;
	int iret = 0;
	char szout[1024];

#ifdef FLOPPY_INT13_READWRITE
	//��չint13h�޷���ȡ���̣�Ϊʲô��
	if (gFloppyDev == -1)
	{
		//floppy not support 0x42xx int13h?
		gFloppyDev = getAtapiDev(0, 0x7f);
		if (gFloppyDev == -1)
		{
			__printf(szout,( char*)"not found floppy device\n");
			return FALSE;
		}
		else {
			__printf(szout, "find floppy device:%x\n", gFloppyDev);
		}
	}
#endif
	
	gFloppyDev = 0;
#ifdef FLOPPY_INT13_READWRITE
	iret = vm86ReadFloppy(getCylinder(0), getHeader(0), getSector(0), 2, (char*)&gFat12Dbr, gFloppyDev);	
#else
	iret = readFloppySector(gFloppyDev, (unsigned long)gFat12Dbr, 0, 2);
#endif
	if (iret <= 0)
	{
		gFloppyDev = 1;

#ifdef FLOPPY_INT13_READWRITE
		iret = vm86ReadFloppy(getCylinder(0), getHeader(0), getSector(0), 2, (char*)&gFat12Dbr, gFloppyDev);	
#else
		iret = readFloppySector(gFloppyDev, (unsigned long)gFat12Dbr, 0, 2);
#endif
		if (iret <= 0)
		{
			__printf(szout, ( char*)"read floppy dbr sector error\n");
			return FALSE;
		}
	}

	__dump((char*)gFat12Dbr, 512, 1, (unsigned char*)FLOPPY_DMA_BUFFER + 0x1000);
	//__drawGraphChars((char*)FLOPPY_DMA_BUFFER + 0x1000, 0);

	if (__memcmp((CHAR*)gFat12Dbr->BS_FileSysType,"FAT12",5))
	{
		gFloppyDev = gFloppyDev ^ 1;

#ifdef FLOPPY_INT13_READWRITE
		iret = vm86ReadFloppy(getCylinder(0), getHeader(0), getSector(0), 2, (char*)&gFat12Dbr, gFloppyDev);	
#else
		iret = readFloppySector(gFloppyDev, (unsigned long)gFat12Dbr, 0, 2);
#endif
		if (iret <= 0 || __memcmp((CHAR*)gFat12Dbr->BS_FileSysType, "FAT12", 5))
		{
			__printf(szout, (char*)"read floppy dbr sector error\n");
			return FALSE;
		}
	}

	gFat12ClusterSize = gFat12Dbr->BPB_SecPerClus*gFat12Dbr->BPB_BytesPerSec;	//512

	int rootdirsize = gFat12Dbr->BPB_RootEntCnt * sizeof(FAT32DIRECTORY);		//1c00
	int rootdirseccnt = rootdirsize / gFat12Dbr->BPB_BytesPerSec;				//14
	int mod = rootdirsize % gFat12Dbr->BPB_BytesPerSec;
	if (mod)
	{
		rootdirseccnt++;
	}
	gFat12RootSecCnt = rootdirseccnt;	//14

	gFat12FatSecOff = gFat12Dbr->BPB_HiddSec + gFat12Dbr->BPB_RsvdSecCnt;	//1 0x200

	gFat12RootDirSecOff = gFat12FatSecOff + gFat12Dbr->BPB_FATSz16*gFat12Dbr->BPB_NumFATs;		//19		0x2600
	gFat12DataSecOff = gFat12RootDirSecOff + rootdirseccnt;						//33 0x4200

	if (gFat12FatBase == 0)
	{
		gFat12FatBase = (DWORD)FLOPPY_FAT_BUFFER;
	}

	if (gFat12RootDirBase == 0)
	{
		gFat12RootDirBase = (DWORD)FLOPPY_ROOT_BUFFER;
	}
#ifdef FLOPPY_INT13_READWRITE
	iret = vm86ReadFloppy(getCylinder(gFat12FatSecOff), getHeader(gFat12FatSecOff),
		getSector(gFat12FatSecOff), gFat12Dbr->BPB_FATSz16, (char*)gFat12FatBase, gFloppyDev);
#else
	iret = readFloppySector(gFloppyDev, (unsigned long)gFat12FatBase, gFat12FatSecOff, gFat12Dbr->BPB_FATSz16);
#endif
	if (iret <= 0)
	{
		__printf(szout, ( char*)"read floppy fat sector error\n");
		return FALSE;
	}

#ifdef FLOPPY_INT13_READWRITE
	iret = vm86ReadFloppy(getCylinder(gFat12RootDirSecOff), getHeader(gFat12RootDirSecOff),
		getSector(gFat12RootDirSecOff), gFat12RootSecCnt, (char*)gFat12RootDirBase, gFloppyDev);
#else
	iret = readFloppySector(gFloppyDev, (unsigned long)gFat12RootDirBase, gFat12RootDirSecOff, gFat12RootSecCnt);
#endif
	if (iret <= 0)
	{
		__printf(szout, ( char*)"read floppy root dir sector error\n");
		return FALSE;
	}

	int cnt = 0;

	LPFAT32DIRECTORY dir = (LPFAT32DIRECTORY)gFat12RootDirBase;

	for (int i = 0; i < gFat12Dbr->BPB_RootEntCnt; i ++)
	{
		if (dir->attr != 0x0f /*&& ( (dir->attr & FILE_ATTRIBUTE_ARCHIVE) || (dir->attr & FILE_ATTRIBUTE_DIRECTORY) )*/ && *dir->mainName)
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

			DWORD clsnum = dir->clusterHigh;
			clsnum = (clsnum << 16) + dir->clusterLow;
			files->secno = clsnum;							/*FAT12_FIRST_CLUSTER_NO*/
			files->attrib = dir->attr;
			if (dir->attr == 0) {
				files->attrib = FILE_ATTRIBUTE_ARCHIVE;
			}
			files->filesize = dir->size;

			files++;

			cnt++;
		}
		dir++;
	}

	return cnt;
}



int fat12FileReader(DWORD clusterno,int filesize, char * lpdata, int readsize) {
	char szout[1024];
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

	int readtimes = readsize / gFat12Dbr->BPB_SecPerClus;
	int readmod = readsize % gFat12Dbr->BPB_SecPerClus;
	if (readmod)
	{
		readtimes++;
	}

	for (int i = 0; i < readtimes; i++)
	{
		DWORD sectorno = gFat12DataSecOff + (clusterno - FAT12_FIRST_CLUSTER_NO) * gFat12Dbr->BPB_SecPerClus;
#ifdef FLOPPY_INT13_READWRITE
		ret = vm86ReadFloppy(getCylinder(sectorno), getHeader(sectorno), getSector(sectorno), gFat12Dbr->BPB_SecPerClus, lpdata, gFloppyDev);
#else
		ret = readFloppySector(gFloppyDev, (unsigned long)FLOPPY_DMA_BUFFER, sectorno, gFat12Dbr->BPB_SecPerClus);
#endif
		if (ret )
		{
			__memcpy(lpdata,(char*) FLOPPY_DMA_BUFFER, gFat12Dbr->BPB_SecPerClus * gFat12Dbr->BPB_BytesPerSec);
			lpdata += gFat12ClusterSize;

			readoksize += gFat12ClusterSize;
		}
		else {
			__printf(szout, ( char*)"fat12 read cluster error\n");
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

	char * next = (clusterno * 12) / 8 + (char*)gFat12FatBase;

	int mod = clusterno * 12 % 8;
	if (mod)
	{
		unsigned int low = (*next) >> 4;
		unsigned int high = (*(next + 1)) << 4;
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