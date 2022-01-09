#pragma once

#include "../def.h"

#define MFTEntrySize			1024
#define MFTEntryFlag			0x454C4946
#define MSF_FILENAME_FLAG		0X30
#define MSF_DATA_FLAG			0X80
#define MSF_INDEXROOT_FLAG		0X90
#define MSF_INDEXALLOC_FLAG		0XA0

typedef enum _ATTRIBUTE_TYPE
{
	AttributeStandardInformation = 0x10,
	AttributeAttributeList = 0x20,
	AttributeFileName = 0x30,
	AttributeObjectId = 0x40,
	AttributeSecurityDescriptor = 0x50,
	AttributeVolumeName = 0x60,
	AttributeVolumeInformation = 0x70,
	AttributeData = 0x80,
	AttributeIndexRoot = 0x90,
	AttributeIndexAllocation = 0xA0,
	AttributeBitmap = 0xB0,
	AttributeReparsePoint = 0xC0,
	AttributeEAInformation = 0xD0,
	AttributeEA = 0xE0,
	AttributePropertySet = 0xF0,
	AttributeLoggedUtilityStream = 0x100
} ATTRIBUTE_TYPE, *PATTRIBUTE_TYPE;

#pragma pack(1)

typedef struct _FILETIME {
	DWORD dwLowDateTime;
	DWORD dwHighDateTime;
} FILETIME, *PFILETIME, *LPFILETIME;

//DPT����
typedef struct _PartTableEntry {
	BYTE bootSignature;					//������־
	BYTE startHead;						//CHSѰַ��ʽ����ʼ��ͷ
	BYTE startSector;					//��ʼ���������ֽڵ���λ
	BYTE startCylinder;					//��ʼ�ŵ�(����)��startSector�߶�λ�ͱ��ֽ�
	BYTE systemSignature;				//�������ͱ�־
	BYTE endHead;						//��ֹ��ͷ
	BYTE endSector;						//��ֹ����
	BYTE endCylinder;					//��ֹ�ŵ�
	unsigned int startSectorNo;			//LBAѰַ����ʼ������
	unsigned int totalSectorsNum;		//�÷�����������
}PartTableEntry, *pPartTableEntry;

//MBR����
typedef struct _MBRSector {
	BYTE MBR[446];
	PartTableEntry ptEntrys[4];
	BYTE endSignature[2];
}MBRSector, *pMBRSector;

//NTFS DBR����
typedef struct _NTFSDBR {
	BYTE JMP[3];						//0 ��תָ��
	BYTE FsID[8];						//3 �ļ�ϵͳID
	unsigned short bytePerSector;		//11 ÿ�����ֽ���
	BYTE secPerCluster;					//13 ÿ��������
	BYTE reservedBytes[2];				//14 2�������ֽ�
	BYTE zeroBytes[3];					//16 ����0�ֽ�
	BYTE unusedBytes1[2];				//19 2��δ���ֽ�
	BYTE mediaType;						//21 ý������
	BYTE unusedBytes2[2];				//22 2��δ���ֽ�
	unsigned short secPerTrack;			//24 ÿ�ŵ�������
	unsigned short Heads;				//26 ��ͷ��
	unsigned int hideSectors;			//28 ����������
	BYTE unusedBytes3[4];				//32 4��δ���ֽ�
	BYTE usedBytes[4];					//36 4���̶��ֽ�0x800080
	unsigned __int64 totalSectors;		//40 ��������
	unsigned __int64 MFT;				//48 MFT��ʼ�غ�
	unsigned __int64 MFTMirror;			//56 MFTMirror�ļ���ʼ�غ�
	char fileRecord;					//64 �ļ���¼
	BYTE unusedBytes4[3];				//65 3��δ���ֽ�
	char indexSize;						//68 ������������С
	BYTE unusedBytes5[3];				//69 δ���ֽ�
	BYTE volumeSerialID64[8];			//72 �����к�
	unsigned int checkSum;				//80 У���
	BYTE bootCode[426];					//84 ��������
	BYTE endSignature[2];				//510 ������־
}NTFSDBR, *LPNTFSDBR;

//MFT����Ľṹ
//�ļ���¼ͷ
typedef struct _FILE_RECORD_HEADER
{
	/*+0x00*/	BYTE Type[4];					//0 �̶�ֵ'FILE'
	/*+0x04*/	UINT16 USNOffset;				//4 �������к�ƫ��, �����ϵͳ�й�
	/*+0x06*/	UINT16 USNCount;				//6 �̶��б��СSize in words of Update Sequence Number & Array (S)
	/*+0x08*/	UINT64 Lsn;						//8 ��־�ļ����к�(LSN)
	/*+0x10*/	UINT16  SequenceNumber;			//16 ���к�(���ڼ�¼�ļ�������ʹ�õĴ���)
	/*+0x12*/	UINT16  LinkCount;				//18 Ӳ������
	/*+0x14*/	UINT16  AttributeOffset;		//20 ��һ������ƫ��
	/*+0x16*/	UINT16  Flags;					//22 flags, 00��ʾɾ���ļ�,01��ʾ�����ļ�,02��ʾɾ��Ŀ¼,03��ʾ����Ŀ¼
	/*+0x18*/	UINT32  BytesInUse;				//24 �ļ���¼ʵʱ��С(�ֽ�) ��ǰMFT�����,��FFFFFF�ĳ���+4
	/*+0x1C*/	UINT32  BytesAllocated;			//28 �ļ���¼�����С(�ֽ�)
	/*+0x20*/	UINT64  BaseFileRecord;			//32 = 0 �����ļ���¼ File reference to the base FILE record
	/*+0x28*/	UINT16  NextAttributeNumber;	//40 ��һ������ID��
	/*+0x2A*/	UINT16  Pading;					//42 �߽�
	/*+0x2C*/	UINT32  MFTRecordNumber;		//44 windows xp��ʹ��,��MFT��¼��
	/*+0x30*/	UINT16  USN;					//48 �������к�
	/*+0x32*/	BYTE  UpdateArray[6];			//50 ��������
} FILE_RECORD_HEADER, *LPFILE_RECORD_HEADER;

//��פ���Ժͷǳ�פ���ԵĹ��ò���
typedef struct _CommonAttributeHeader {
	UINT32 ATTR_Type;		//0 ��������
	UINT32 ATTR_Size;		//4 ����ͷ����������ܳ���
	BYTE ATTR_ResFlag;		//8 �Ƿ��ǳ�פ���ԣ�0��פ 1�ǳ�פ��
	BYTE ATTR_NamSz;		//9 �������ĳ���
	UINT16 ATTR_NamOff;		//10 ��������ƫ�� ���������ͷ
	UINT16 ATTR_Flags;		//12 ��־��0x0001ѹ�� 0x4000���� 0x8000ϡ�裩
	UINT16 ATTR_Id;			//14 ����ΨһID
}CommonAttributeHeader, *LPCommonAttributeHeader;

//��פ���� ����ͷ
typedef struct _ResidentAttributeHeader {
	CommonAttributeHeader	ATTR_Common;
	UINT32 ATTR_DatSz;		//0 16 �������ݵĳ���
	UINT16 ATTR_DatOff;		//4 20 �����������������ͷ��ƫ��
	BYTE ATTR_Indx;			//6 22 ����
	BYTE ATTR_Resvd;		//7 23 ����
	//BYTE ATTR_AttrNam;	//8 24 ��������Unicode����β��0
}ResidentAttributeHeader, *LPResidentAttributeHeader;

//�ǳ�פ���� ����ͷ
typedef struct _NonResidentAttributeHeader {
	CommonAttributeHeader		ATTR_Common;
	UINT64 ATTR_StartVCN;		//0 16����������������ʼ����غ�
	UINT64 ATTR_EndVCN;			//8 24����������������ֹ����غ�
	UINT16 ATTR_DatOff;			//16 32�����б����������ͷ��ƫ��
	UINT16 ATTR_CmpSz;			//18 34ѹ����λ 2��N�η�
	UINT32 ATTR_Resvd;			//20 36
	UINT64 ATTR_AllocSz;		//24 40���Է���Ĵ�С
	UINT64 ATTR_ValidSz;		//32 48���Ե�ʵ�ʴ�С
	UINT64 ATTR_InitedSz;		//40 56���Եĳ�ʼ��С
	//BYTE ATTR_AttrNam[0];		//48 64
}NonResidentAttributeHeader, *LPNonResidentAttributeHeader;

/*�����������ṹ�Ķ���*/

//��׼����ͷ�Ľṹ
typedef struct _STD_INDEX_HEADER {
	BYTE SIH_Flag[4];				//0 �̶�ֵ "INDX"
	UINT16 SIH_USNOffset;			//4 �������к�ƫ��
	UINT16 SIH_USNSize;				//6 �������кź͸��������С
	UINT64 SIH_Lsn;					//8 ��־�ļ����к�(LSN)
	UINT64 SIH_IndexCacheVCN;		//16 �����������������������е�VCN
	UINT32 SIH_IndexEntryOffset;	//24 �������ƫ�� ����ڵ�ǰλ��
	UINT32 SIH_IndexEntrySize;		//28 ������Ĵ�С
	UINT32 SIH_IndexEntryAllocSize;	//32 ���������Ĵ�С
	UINT8 SIH_HasLeafNode;			//36 ��һ ��ʾ���ӽڵ�
	BYTE SIH_Fill[3];				//37 ���
	UINT16 SIH_USN;					//40 �������к�
	BYTE SIH_USNArray[46];			//42 ������������
}STD_INDEX_HEADER, *LPSTD_INDEX_HEADER;

//��׼������Ľṹ
typedef struct _STD_INDEX_ENTRY {
	//��6�ֽ���Ŀ¼�����ļ���Ӧ���ļ���¼�ı�ţ�����MFT��˳��洢�ģ����ݸñ�ſ��Զ�λ�����ļ���¼��MFT�е�λ��
	UINT64 SIE_MFTReferNumber;			//0 �ļ���MFT�ο���
	UINT16 SIE_IndexEntrySize;			//8 ������Ĵ�С
	UINT16 SIE_FileNameAttriBodySize;	//10 �ļ���������Ĵ�С
	UINT16 SIE_IndexFlag;				//12 ������־
	BYTE SIE_Fill[2];					//14 ���
	UINT64 SIE_FatherDirMFTReferNumber;	//16 ��Ŀ¼MFT�ļ��ο���
	FILETIME SIE_CreatTime;				//24 �ļ�����ʱ��
	FILETIME SIE_AlterTime;				//32 �ļ�����޸�ʱ��
	FILETIME SIE_MFTChgTime;			//40 �ļ���¼����޸�ʱ��
	FILETIME SIE_ReadTime;				//48 �ļ�������ʱ��
	UINT64 SIE_FileAllocSize;			//56 �ļ������С
	UINT64 SIE_FileRealSize;			//64 �ļ�ʵ�ʴ�С
	UINT64 SIE_FileFlag;				//72 �ļ���־
	UINT8 SIE_FileNameSize;				//80 �ļ�������
	UINT8 SIE_FileNamespace;			//81 �ļ������ռ�
	BYTE SIE_FileNameAndFill;			//82 �ļ��������
}STD_INDEX_ENTRY, *LPSTD_INDEX_ENTRY;

typedef struct _INDEX_ENTRY {
	UINT64 IE_MftReferNumber;			/*0 ���ļ���MFT�ο��š�ע�⣺��ֵ�ĵ�6�ֽ���MFT��¼�ţ���2�ֽ��Ǹ�MFT��¼�����к�*/
	UINT16 IE_Size;						//8 ������Ĵ�С ����������ʼ��ƫ����
	UINT16 IE_FileNAmeAttriBodySize;	//10 �ļ���������Ĵ�С
	UINT16 IE_Flags;					//12
										/*��־����ֵ����������ֵ֮һ
										0x00       ��ͨ�ļ���
										0x01       ������
										0x02       ��ǰ�������һ��Ŀ¼��
										�ڶ�ȡ����������ʱӦ�����ȼ��ó�Ա��ֵ��ȷ����ǰ�������*/

	UINT16 IE_Fill;						//14 ��� ������
	UINT64 IE_FatherDirMftReferNumber;	//16 ��Ŀ¼��MFT�ļ��ο���
	FILETIME IE_CreatTime;				//24 �ļ�����ʱ��
	FILETIME IE_AlterTime;				//32 �ļ�����޸�ʱ��
	FILETIME IE_MFTChgTime;				//40 �ļ���¼����޸�ʱ��
	FILETIME IE_ReadTime;				//48 �ļ�������ʱ��

	UINT64 IE_FileAllocSize;			//56 �ļ������С
	UINT64 IE_FileRealSize;				//64 �ļ�ʵ�ʴ�С
	UINT64 IE_FileFlag;					//72 �ļ���־
	UINT8 IE_FileNameSize;				//80 �ļ�������
	UINT8 IE_FileNamespace;				//81 �ļ������ռ�
	BYTE IE_FileNameAndFill;			//82 �ļ��������
										//BYTE IE_Stream[0];//Ŀ¼�����ݣ��ṹ���ļ������Ե�������ͬ
										//UINT64 IE_SubNodeFR;//����ļ�¼��������ֵ�ĵ�6�ֽ���MFT��¼�ţ���2�ֽ��Ǹ�MFT��¼�����к�
}INDEX_ENTRY, *LPINDEX_ENTRY;

/****���涨��ľ���������Ľṹ ����������ͷ****/

//STANDARD_INFORMATION 0X10������
/*
SI_DOSAttrȡֵ��
0x0001    ֻ��
0x0002    ����
0x0004    ϵͳ
0x0020    �鵵
0x0040    �豸
0x0080    ����
0x0100    ��ʱ�ļ�
0x0200    ϡ���ļ�
0x0400    �ؽ�����
0x0800    ѹ��
0x1000    ����
0x2000    ����������
0x4000    ����
*/

typedef struct _STANDARD_INFORMATION {
	FILETIME SI_CreatTime;		//����ʱ��
	FILETIME SI_AlterTime;		//����޸�ʱ��
	FILETIME SI_MFTChgTime;		//�ļ���MFT�޸ĵ�ʱ��
	FILETIME SI_ReadTime;		//������ʱ��
	UINT32 SI_DOSAttr;			//DOS�ļ�����
	UINT32 SI_MaxVer;			//�ļ����õ����汾�� 0��ʾ����
	UINT32 SI_Ver;				//�ļ��汾�� �����汾��Ϊ0 ��ֵΪ0
	UINT32 SI_ClassId;			//??
								//UINT64 SI_OwnerId;//�ļ�ӵ����ID
								//UINT64 SI_SecurityId;//��ȫID
								//UINT64 SI_QuotaCharged;//�ļ�����ʹ�õĿռ���� 0��ʾ������
								//UINT64 SI_USN;//�ļ����һ�θ��µļ�¼��
#if 0  
	uint32 QuotaId;
	uint32 SecurityId;
	uint64 QuotaCharge;
	USN Usn;
#endif  
}STANDARD_INFORMATION, *LPSTANDARD_INFORMATION;


//ATTRIBUTE_LIST 0X20������
typedef struct _ATTRIBUTE_LIST {
	UINT32 AL_RD_Type;
	UINT16 AL_RD_Len;
	BYTE AL_RD_NamLen;
	BYTE AL_RD_NamOff;
	UINT64 AL_RD_StartVCN;		//����������������ʼ�Ĵغ�
	UINT64 AL_RD_BaseFRS;		/*�����Լ�¼������MFT��¼�ļ�¼��
								ע�⣺��ֵ�ĵ�6�ֽ���MFT��¼�ţ���2�ֽ��Ǹ�MFT��¼�����к�*/
	UINT16 AL_RD_AttrId;
	//BYTE AL_RD_Name[0];
	UINT16 AlignmentOrReserved[3];
}ATTRIBUTE_LIST, *LPATTRIBUTE_LIST;

//FILE_NAME 0X30������
typedef struct _FILE_NAME {
	UINT64 FN_ParentFR;		/*0 ��Ŀ¼��MFT��¼�ļ�¼������ע�⣺��ֵ�ĵ�6�ֽ���MFT��¼�ţ���2�ֽ��Ǹ�MFT��¼�����к�*/
	FILETIME FN_CreatTime;	//8
	FILETIME FN_AlterTime;	//16
	FILETIME FN_MFTChg;		//24
	FILETIME FN_ReadTime;	//32
	UINT64 FN_AllocSz;		//40
	UINT64 FN_ValidSz;		//48 �ļ�����ʵ�ߴ�
	UINT32 FN_DOSAttr;		//56 DOS�ļ�����
	UINT32 FN_EA_Reparse;	//60 ��չ����������
	BYTE FN_NameSz;			//64 �ļ������ַ���
	BYTE FN_NamSpace;		//65
					/*�����ռ䣬��ֵ��Ϊ����ֵ�е�����һ��
					 0��POSIX������ʹ�ó�NULL�ͷָ�����/��֮�������UNICODE�ַ���������ʹ��255���ַ���ע�⣺�������ǺϷ��ַ�����Windows������ʹ�á�
					 1��Win32��Win32��POSIX��һ���Ӽ��������ִ�Сд������ʹ�ó�������������������?��������������/������<������>������/������|��֮�������UNICODE�ַ��������ֲ����ԡ�.����ո��β��
					 2��DOS��DOS�����ռ���Win32���Ӽ���ֻ֧��ASCII����ڿո��8BIT��д�ַ����Ҳ�֧�������ַ�������������������?��������������/������<������>������/������|������+������,������;������=����ͬʱ���ֱ��밴���¸�ʽ������1~8���ַ���Ȼ���ǡ�.����Ȼ������1~3���ַ���
					 3��Win32&DOS����������ռ���ζ��Win32��DOS�ļ����������ͬһ���ļ��������С�*/
	BYTE FN_FileName;		//66;
}FILE_NAME, *LPFILE_NAME;

//VOLUME_VERSION 
typedef struct _VOLUME_VERSION {
	//??
}VOLUME_VERSION, *pVOLUME_VERSION;

//OBJECT_ID 0X40������
typedef struct _OBJECT_ID {
	BYTE OID_ObjID[16];			//�ļ���GUID
	BYTE OID_BirthVolID[16];	//�ļ�����ʱ���ھ��ID
	BYTE OID_BirthID[16];		//�ļ���ԭʼID
	BYTE OID_DomainID[16];		//����������ʱ�������ID
}OBJECT_ID, *LPOBJECT_ID;

//SECRUITY_DESCRIPTOR 0X50������
typedef struct _SECRUITY_DESCRIPTOR {
	//??
}SECRUITY_DESCRIPTOR, *LPSECRUITY_DESCRIPTOR;

//VOLUME_NAME 0X60������
typedef struct _VOLUME_NAME {
	BYTE VN_Name;
}VOLUME_NAME, *LPVOLUME_NAME;

//VOLUME_INFORMATION 0X70������
typedef struct _VOLUME_INFORMATION {
	UINT64 VI_Resvd;
	BYTE VI_MajVer;//�����汾��
	BYTE VI_MinVer;//���Ӱ汾��
	UINT16 VI_Flags;/*��־λ�����������¸�ֵ���
					0x0001    ��λ������ֵ������ʱWindows�������´�����ʱ����chkdsk/F���
					0x0002    ��־�ļ��ı�ߴ�
					0x0004    ��ҽ�ʱ����
					0x0008    ��Windows NT 4�ҽ�
					0x0010    ����ʱɾ��USN
					0x0020    �޸�����ID
					0x8000    ��chkdsk�޸Ĺ�*/
}VOLUME_INFORMATION, *LPVOLUME_INFORMATION;

//DATA 0X80������
typedef struct _DATA {
	//??
	///*+0x10*/   UINT64 StartVcn;			//0 16 LowVcn ��ʼVCN  ��ʼ�غ�  
	///*+0x18*/   UINT64 LastVcn;			//8 24 HighVcn  ����VCN  �����غ�  
	///*+0x20*/   UINT16 RunArrayOffset;    //16 32 �������е�ƫ��  
	///*+0x22*/   UINT16 CompressionUnit;   //18 34 ѹ������  
	///*+0x24*/   UINT32  Padding0;			//20 36 ���  
	///*+0x28*/   UINT32  IndexedFlag;		//24 40 Ϊ����ֵ�����С(������Ĵص��ֽ�������)  
	///*+0x30*/   UINT64 AllocatedSize;		//28 44 ����ֵʵ�ʴ�С  
	///*+0x38*/   UINT64 DataSize;			//36 52 ����ֵѹ����С  
	///*+0x40*/   UINT64 InitializedSize;   //44 60 ʵ�����ݴ�С  
	///*+0x48*/   UINT64 CompressedSize;    //52 68 ѹ�����С 
	BYTE D_data;							//60 76
}DATA, *LPDATA;

typedef struct _INDEX_HEADER {
	UINT32 IH_EntryOff;				//0 ��һ��Ŀ¼���ƫ��
	UINT32 IH_TalSzOfEntries;		//4 Ŀ¼����ܳߴ�(��������ͷ�������������)
	UINT32 IH_AllocSize;			//8 Ŀ¼�����ĳߴ�
	BYTE IH_Flags;					//12
									/*��־λ����ֵ���������º�ֵ֮һ��
									0x00       СĿ¼(���ݴ���ڸ��ڵ����������)
									0x01       ��Ŀ¼(��ҪĿ¼��洢����������λͼ)*/
	BYTE IH_Resvd[3];				//13
}INDEX_HEADER, *LPINDEX_HEADER;

//INDEX_ROOT 0X90������
typedef struct _INDEX_ROOT {
	//������
	UINT32 IR_AttrType;			//0 ���Ե�����
	UINT32 IR_ColRule;			//4 �������
	UINT32 IR_EntrySz;			//8 Ŀ¼�����ߴ�
	BYTE IR_ClusPerRec;			//12 ÿ��Ŀ¼��ռ�õĴ���
	BYTE IR_Resvd[3];			//13
	INDEX_HEADER IH;			//16 ����ͷ				
	//BYTE IR_IndexEntry;		//32 ������ ���ܲ�����
}INDEX_ROOT, *LPINDEX_ROOT;



//INDEX_ALLOCATION 0XA0������
typedef struct _INDEX_ALLOCATION {
	//UINT64 IA_DataRuns;
	BYTE IA_DataRuns;
}INDEX_ALLOCATION, *LPINDEX_ALLOCATION;

//BITMAP
typedef struct _MFT_ATTR_BITMAP {
	//??
}MFT_ATTR_BITMAP, *LPMFT_ATTR_BITMAP;

//SYMBOL_LINK
typedef struct _SYMBOL_LINK {
	//??
}SYMBOL_LINK, *LPSYMBOL_LINK;

//REPARSE_POINT
typedef struct _REPARSE_POINT {
	UINT32 RP_Type;/*�ؽ����������ͣ���ֵ����������ֵ֮һ
				   0x20000000    ����
				   0x40000000    ��ߵȴ�ʱ��
				   0x80000000    ΢��ʹ��
				   0x68000005    NSS
				   0x68000006    NSS�ָ�
				   0x68000007    SIS
				   0x68000008    DFS
				   0x88000003    ��ҽӵ�
				   0xA8000004   HSM
				   0xE8000000   Ӳ����*/
	UINT16 RP_DatSz;		//�ؽ������ݳߴ�
	UINT16 RP_Resvd;
	BYTE RP_Data;		//	�ؽ�������
}REPARSE_POINT, *LPREPARSE_POINT;

//EA_INFORMATION
typedef struct _EA_INFORMATION {
	UINT16 EI_PackedSz;				//	ѹ����չ���Գߴ�
	UINT16 EI_NumOfEA;				//ӵ��NEED_EA��¼����չ���Ը���
	UINT32 EI_UnpackedSz;			//δѹ����չ���Գߴ�
}EA_INFORMATION, *LPEA_INFORMATION;

//EA
typedef struct _EA {
	UINT32 EA_Next;			//��һ����չ���Ե�ƫ��(����¼�ĳߴ�)
	BYTE EA_Flags;			//��־λ��ֵȡ0x80��ʾ��ҪEA
	BYTE EA_NamLen;			//�������ݵĳ���(M)
	UINT16 EA_ValLen;		//ֵ���ݵĳ���
	BYTE EA_NameVal;		//�������ݺ�ֵ����
}EA, *LPEA;

//PROPERTY_SET
typedef struct _PROPERTY_SET {
	//??
}PROPERTY_SET, *LPPROPERTY_SET;

//LOGGED_UNTILITY_STREAM
typedef struct _LOGGED_UNTILITY_STREAM {
	//??
}LOGGED_UNTILITY_STREAM, *LPLOGGED_UNTILITY_STREAM;



#pragma pack()

int initNTFS();




#ifdef DLL_EXPORT

extern "C" __declspec(dllexport) NTFSDBR gNtfsDbr;
extern "C" __declspec(dllexport) char gMsfRoot[MFTEntrySize];
extern "C" __declspec(dllexport) unsigned __int64 gMsfOffset;

extern "C" __declspec(dllexport) int getNtfsDBR();
extern "C" __declspec(dllexport) int readMSFRoot();

#else
extern "C" __declspec(dllimport) NTFSDBR gNtfsDbr;
extern "C" __declspec(dllimport) char gMsfRoot[MFTEntrySize];
extern "C" __declspec(dllimport) unsigned __int64 gMsfOffset;

extern "C" __declspec(dllimport) int getNtfsDBR();
extern "C" __declspec(dllimport) int readMSFRoot();

#endif





#define MSF_ROOTDIR_OFFSET 10
