#pragma once

#include "../def.h"
#include "../file.h"



//00000002~FFFFFFEF		�ѷ���Ĵ�
//FFFFFFF0~FFFFFFF6		ϵͳ����
//FFFFFFF7				����
//FFFFFFF8~FFFFFFFF		�ļ�������


#define FAT32_FLAG				0xAA55
#define FAT_MAINNAME_LEN		8
#define FAT_SUFFIXNAME_LEN		3
#define FAT_NAMETOTAL_LEN		(FAT_MAINNAME_LEN + FAT_SUFFIXNAME_LEN)
#define FAT_HEAD_FLAG			"0xf8\xff\xff\x0f\xff\xff\xff\xff"
#define EMPTY_MAINNAME			"\x00\x00\x00\x00\x00\x00\x00\x00"
#define EMPTY_SUFFIXNAME		"\x00\x00\x00"
#define FAT_BAD_CLUSTER			0xFFFFFFF7
#define FAT_END_FLAG			0x0FFFFFFF
#define VALID_MAX_CLUSTER_NO	0XFFFFFFEF
#define FAT_EMPTY_CLUSTER		0
//FAT32 ��0�ű���ֵ���ǡ�F8FFFF0F��
//1�ű�����ܱ����ڼ�¼�����־����˵���ļ�ϵͳû�б�����ж�ػ��ߴ��̱�����ڴ���
//1�ű���ֵ��FFFFFFFF����FFFFFF0F��

#define FAT_DIR_UNICODE_FLAG	0x0f

// #define FILE_OPEN_READONLY		0
// #define FILE_OPEN_HIDDENWRITE	1
// #define FILE_OPEN_HIDDENAPPEND  2
// #define FILE_OPEN_WRITE			3
// #define FILE_OPEN_APPEND		4
// #define DIR_OPEN_ALL			8


//fatȱ��
//1 fat32�ĸ�Ŀ¼�ļ����ܳ���2���صĴ�С,Ϊcluster*2/32���ļ�(����ش�СΪ8KB,��ʼ�غ�Ϊ2,�������510���ļ�������.��..2��Ŀ¼,��512�����ļ���Ŀ¼)
//2 ��������֧�����ޣ����Ŀ¼�������ģ���ô��Ҫ���������Ŀ¼���ֱ�ʾ��������������ÿ��Ŀ¼�е��ļ�����ø�����
//3 fat32�ĸ�ʽֻʶ��4g���ڵĵ����ļ�
//4 ��Ҫ���fat���д��ÿ�����һ�ظ�������Ч�ʴ���Ӱ��


#pragma pack(1)


typedef struct 
{
	unsigned char BS_jmpBoot[3];		//��תָ��			offset: 0

	unsigned char BS_OEMName[8];		//ԭʼ������		offset: 3		MSDOS5.0

	unsigned short BPB_BytesPerSec;		//ÿ�����ֽ���		offset:11

	unsigned char BPB_SecPerClus; 		//ÿ��������		offset:13

	unsigned short BPB_RsvdSecCnt; 		//����������Ŀ		offset:14

	unsigned char BPB_NumFATs; 			//�˾���FAT����		offset:16

	unsigned short BPB_RootEntCnt;		//FAT32Ϊ0			offset:17

	unsigned short BPB_TotSec16; 		//FAT32Ϊ0			offset:19

	unsigned char BPB_Media;			//�洢����			offset:21

	unsigned short BPB_FATSz16;			//FAT32Ϊ 0			offset:22

	unsigned short BPB_SecPerTrk;		//�ŵ�������		offset:24

	unsigned short BPB_NumHeads;		//��ͷ��			offset:26

	unsigned int BPB_HiddSec; 			//FAT��ǰ��������	offset:28

	unsigned int BPB_TotSec32;			//�þ���������		offset:32

	unsigned int BPB_FATSz32; 			//һ��FAT��������	offset:36

	unsigned short BPB_ExtFlags;		//FAT32����			offset:40

	unsigned short BPB_FSVer;			//FAT32����			offset:42

	unsigned int BPB_RootClus;			//��Ŀ¼�غ�		offset:44

	unsigned short FSInfo;				//��������FSINFO������ offset:48

	unsigned short BPB_BkBootSec;		//����ͨ��Ϊ6		offset:50

	unsigned char BPB_Reserved[12];		//��չ��			offset:52

	unsigned char BS_DrvNum;			//offset:64

	unsigned char BS_Reserved1;			//offset:65

	unsigned char BS_BootSig;			//offset:66

	unsigned int BS_VolID;				//offset:67

	unsigned char BS_FilSysType[11];	//offset:71

	unsigned char BS_FilSysType1[8];	//"FAT32 " offset:82

	unsigned char runCode[420];
	unsigned short systemFlag;
}FAT32_DBR,*LPFAT32_DBR;



typedef struct
{
	unsigned char extFlag[4];		//RRaA

	unsigned char unused[480];

	unsigned char signature[4];		//rrAa

	unsigned int freeClusterCnt;

	unsigned int nextCluster;		//��ʼ����ɺ�׼ȷ��ֻ��д���ļ��������һ���մغ�

	unsigned char unused2[14];

	unsigned short systemFlag;

}FAT32_FSINFO, *LPFAT32_FSINFO;

//ע��1��01111 000010 11100
//1)�����5λ����Сʱ������2^5 = 32���㹻��ʾ24Сʱ�����01111(2����) = 15(10����)
//2)��6λ������ӣ�ͬ��2 ^ 6 = 64���㹻��ʾ60���ӣ����000010(2����) = 2
//3)��5λ��ʾ���1 / 2�� ��������Ҫ���Ϻ���λ�ϵĽ�λ�����11100(2����) = 28(10����)���������� = 28 * 2 = 56��
//�ټ��Ϻ����ϵĽ�λ1���Խ��Ϊ57��

//ע��2��0100010 1000 01000
//1)�����7λ�����1980�꿪ʼ�����������߼������¿��Ե�2108�꣬��֮����90�������ʹ�ã����0100010(2����) = 34��������� = 1980 + 34 = 2014
//2)��4λ�����·ݣ�2 ^ 4 = 16�����Ա�ʾ12���·ݣ���� 1000(2����) = 8(10����)
//3)��5λ�������ڣ�2 ^ 5 = 32�����Ա�ʾ28~31�죬��� 01000(2����) = 8(10����)

//������һ�����ļ����ļ�ʱ��ϵͳ���Զ����϶�Ӧ�Ķ��ļ�������ԭ�����£�
//(1)ȡ���ļ�����ǰ6���ַ�����"~1"�γɶ��ļ�������չ�����䡣
//(2)����Ѵ�������ļ����������"~"������ֵ�����ֱ��5��
//(3)����ļ����С�~����������ִﵽ5������ļ���ֻʹ�ó��ļ�����ǰ������ĸ��
//ͨ����ѧ���ݳ��ļ�����ʣ����ĸ���ɶ��ļ����ĺ��ĸ���ĸ��Ȼ��Ӻ�׺��~1��ֱ�����(����б�Ҫ���������������Ա����ظ����ļ���)�� 
//(4)�����������OS������޷���ȡ���ַ������ԡ�_��
//Ŀ¼��ĵ�12���ֽڵ�ֵ�����Ϊ0x0Fʱ��ϵͳ��Ϊ�ǳ�Ŀ¼��

//3
//һ��Ŀ¼���Ƿ񱻷���ʹ�������ĵ�һ���ֽ���������
//�����Ѿ�����ʹ�õ�Ŀ¼����ĵ�һ���ֽ����ļ����ĵ�һ���ַ���
//���ļ���Ŀ¼��ɾ����������Ӧ��Ŀ¼��ĵ�һ���ֽڽ�����Ϊ0xE5

//4
//ϵͳ�ڴ洢���ļ���ʱ�������Ȱ�������䳤�ļ���Ŀ¼�Ȼ��������Ӧ�Ķ��ļ�����
//һ�����ļ������Ǻ�����Ӧ�Ķ��ļ���һһ��Ӧ�����ļ���û���˳��ļ��������Զ���
//�����ļ������û�ж�Ӧ�Ķ��ļ���������ʲôϵͳ������������ڡ����Զ��ļ�����������Ҫ�ġ�
//�ڲ�֧�ֳ��ļ����Ļ����жԶ��ļ����е��ļ�������չ���ֶ�������(����ɾ������Ϊɾ���Ƕ����ַ���дE5H)������ʹ���ļ�����ͬ����

// ���Ŀ¼��0x0B �ֽڴ�������ֵΪ 0x08

//�ļ���Сд����չ��Сд, ��aaa.aaa        ��00011000 = 18h
//�ļ�����д����չ����д, ��AAA.AAA  ��00000000 = 00h
//�ļ�����д����չ��Сд, ��AAA.aaa     ��00010000 = 10h
//�ļ���Сд����չ����д, ��aaa.AAA     ��00001000 = 08h
//���ļ���������LDE������֮��Ӧ��SDE�е�0C�ֽڶ���0

typedef struct 
{
	unsigned char mainName[FAT_MAINNAME_LEN];
	unsigned char suffixName[FAT_SUFFIXNAME_LEN];
	unsigned char attr;			//0=read andr write,1=readonly,2=hidden,4=system,8=volume,16=dir,32=archive,15=long file name
	unsigned char lowercase;
	unsigned char mseconds;
	unsigned short creationTime;
	unsigned short creationDate;
	unsigned short accessDate;
	unsigned short clusterHigh;
	unsigned short modifyTime;
	unsigned short modifyDate;
	unsigned short clusterLow;
	unsigned int size;
}FAT32DIRECTORY,*LPFAT32DIRECTORY;


typedef struct
{
	unsigned char flag;			//��6Ϊ1�������һ�������5Ϊ������ţ��������µ���,���0x1f(1-1f,5f - 1)���������ļ���13*31=403���ַ�
	unsigned short firstUnicode[5];
	unsigned char attr;
	unsigned char reserved;
	unsigned char checksum;
	unsigned short midUnicode[6];
	unsigned short clusterno;
	unsigned short lastUnicode[2];
}FAT32_UNICODE_DIRECTORY, *LPFAT32_UNICODE_DIRECTORY;

#pragma pack()







#ifdef DLL_EXPORT
extern "C" __declspec(dllexport) int fat32Init();
extern "C" __declspec(dllexport) FAT32_DBR			gFat32Dbr;
extern "C" __declspec(dllexport) FAT32_FSINFO		gFsInfo;
extern "C" __declspec(dllexport) unsigned int *		glpFAT;
extern unsigned int *		glpFAT2;
extern "C" __declspec(dllexport) LPFAT32DIRECTORY	glpRootDir;

extern "C" __declspec(dllexport) int				g_FirstClusterNO;
extern "C" __declspec(dllexport) DWORD				g_SecsBeforeRootDir;
extern "C" __declspec(dllexport) int				gMaxDirsInPath;

extern "C" __declspec(dllexport) int getDBR();
extern "C" __declspec(dllexport) int getFsinfo();
extern "C" __declspec(dllexport) DWORD getRootDirOffset();

extern "C" __declspec(dllexport) int getNextFAT32Cluster(DWORD no);
#else
extern "C" __declspec(dllimport) int fat32Init();
extern "C" __declspec(dllimport) FAT32_DBR gFat32Dbr;
extern "C" __declspec(dllimport) FAT32_FSINFO gFsInfo;
extern "C" __declspec(dllimport) unsigned int * glpFAT;
extern "C" __declspec(dllimport) LPFAT32DIRECTORY glpRootDir;

extern "C" __declspec(dllimport) int g_FirstClusterNO;
extern "C" __declspec(dllimport) int g_SecsBeforeRootDir;
extern "C" __declspec(dllimport) int gMaxDirsInPath;

extern "C" __declspec(dllimport) int getDBR();
extern "C" __declspec(dllimport) int getFsinfo();
extern "C" __declspec(dllimport) DWORD getRootDirOffset();
extern "C" __declspec(dllimport) int getNextFAT32Cluster(DWORD no);
#endif



int checkFlag(unsigned char * sector);

int getFSINFOOffset();

int isFAT32(unsigned char * fat);

int isFAT32DBR(LPFAT32_DBR);

DWORD getFATOffset();

DWORD getFAT2Offset();

int getFAT();

int getFAT2();

int getRootDir();

unsigned int getUnicodeDirectoryChecksum(unsigned char * shortname);

int isValidCluster(unsigned int no);

unsigned int getFirstClusterNo(LPFAT32DIRECTORY);

unsigned int getLastClusterNo(LPFAT32DIRECTORY);

int getFsinfoFreeCluster();

int getNextFAT32EmptyCluster(DWORD no);

unsigned short getClusterNoLow(unsigned int clusterno);
unsigned short getClusterNoHigh(unsigned int clusterno);

int updateFSINFO(int leastclusters, int nextfreecluster);

int updateFAT(int clusterno);

int updateSecsTotal(unsigned int secs);

int getKey(FAT32_DBR dbr, FAT32_FSINFO info, unsigned char * key);