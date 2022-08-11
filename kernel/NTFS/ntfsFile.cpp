#include "ntfsFile.h"
#include "ntfs.h"
#include "../fat32/fileutils.h"
#include "../Utils.h"
#include "../satadriver.h"
#include "../file.h"
#include "../video.h"
#include "../slab.h"




//1 ���dataruns�µĲ���
//2 �����ļ�������
//3 �����ļ�ʧ�ܵ�����


int getDataRuns(char* data) {

	return 0;
}



unsigned long long getIdxNextDir(char* filename, char* buf) {
	int ret = 0;

	//char szout[1024];

	if (__memcmp(buf, "INDX", 4))
	{
		__drawGraphChars((unsigned char*)"getIdxNextDir format error\n", 0);
		return FALSE;
	}

	LPSTD_INDEX_HEADER hdr = (LPSTD_INDEX_HEADER)buf;
	char* baseaddr = (char*)&hdr->SIH_IndexEntryOffset;
	LPSTD_INDEX_ENTRY idxentry = (LPSTD_INDEX_ENTRY)(baseaddr + hdr->SIH_IndexEntryOffset);
	while (1)
	{
		if (idxentry->SIE_IndexEntrySize >= hdr->SIH_IndexEntryAllocSize)
		{
			break;
		}

		if ((unsigned int)idxentry - (unsigned int)buf >= hdr->SIH_IndexEntryAllocSize)
		{
			break;
		}

		int fnlen = idxentry->SIE_FileNameSize;
		char* fn = (char*)((unsigned int)idxentry + sizeof(STD_INDEX_ENTRY) - 1);
		char ascfn[256];
		int asclen = unicode2asc((short*)fn, fnlen, ascfn);

		ret = upper2lower(ascfn, __strlen(ascfn));

		if (__strcmp(ascfn, filename) == 0)
		{
			return (idxentry->SIE_MFTReferNumber & NTFS_MFT_CLUSTER_NUMBER_MASK);
		}

		if ((idxentry->SIE_MFTReferNumber & NTFS_MFT_CLUSTER_NUMBER_MASK) == 0 &&
			(idxentry->SIE_IndexEntrySize < sizeof(STD_INDEX_ENTRY) /*|| idxentry->SIE_IndexEntrySize >= 0x200*/))
		{
			break;
			//idxentry = (LPSTD_INDEX_ENTRY)((unsigned int)idxentry + 0x60);
		}
		else {
			idxentry = (LPSTD_INDEX_ENTRY)((unsigned int)idxentry + idxentry->SIE_IndexEntrySize);
		}

	}

	// 	__printf(szout, "getIdxNextDir not found file:%s\n", filename);
	// 	__drawGraphChars((unsigned char*)szout, 0);
	return FALSE;
}



unsigned long long getRootNextDir(LPCommonAttributeHeader hdr, char* filename) {
	int ret = 0;
	//char szout[1024];

	LPResidentAttributeHeader rhdr = (LPResidentAttributeHeader)hdr;

	LPINDEX_ROOT lproot = (LPINDEX_ROOT)((unsigned int)rhdr + rhdr->ATTR_DatOff);
	LPINDEX_HEADER ih = (LPINDEX_HEADER)&lproot->IH;
	LPINDEX_ENTRY entry = (LPINDEX_ENTRY)(ih->IH_EntryOff + (unsigned int)ih);
	LPINDEX_ENTRY lpentry = entry;
	unsigned int entrysize = ih->IH_TalSzOfEntries - sizeof(INDEX_HEADER);
	while (1)
	{
		if ((entry->IE_MftReferNumber & NTFS_MFT_CLUSTER_NUMBER_MASK) == 0 || entry->IE_Size < sizeof(STD_INDEX_ENTRY))
		{
			break;
		}

		if ((unsigned int)entry - (unsigned int)lpentry >= entrysize)
		{
			break;
		}

		int fnlen = entry->IE_FileNameSize;
		char* fn = (char*)((unsigned int)entry + sizeof(INDEX_ENTRY) - 1);
		char ascfn[256];
		int asclen = unicode2asc((short*)fn, fnlen, ascfn);

		ret = upper2lower(ascfn, __strlen(ascfn));

		if (__strcmp(ascfn, filename) == 0)
		{
			return (entry->IE_MftReferNumber & NTFS_MFT_CLUSTER_NUMBER_MASK);
		}

		entry = (LPINDEX_ENTRY)((unsigned int)entry + entry->IE_Size);
	}

	//__printf(szout, "getRootNextDir not found file:%s\n", filename);
	//__drawGraphChars((unsigned char*)szout, 0);

	return FALSE;
}




unsigned long long getNtfsDir(unsigned long long secoff, char* filename)
{
	int ret = 0;

	char szout[1024];

	char msfinfo[MFTEntrySize * 2];

	DWORD low = secoff & 0xffffffff;
	DWORD high = (secoff >> 32);
	ret = readSector(low, high, 2, (char*)msfinfo);
	if (ret <= 0)
	{
		__printf(szout, "getNtfsDir readSector mft sector low:%x,high:%x error\n", low, high);
		__drawGraphChars((unsigned char*)szout, 0);
		return FALSE;
	}

	if (__memcmp(msfinfo, "FILE", 4))
	{
		__printf(szout, "getNtfsDir mft format error in file:%s sector:%x\n", filename, low);
		__drawGraphChars((unsigned char*)szout, 0);
		return FALSE;
	}

	unsigned long long nextdirsec = 0;

	FILE_RECORD_HEADER* frh = (FILE_RECORD_HEADER*)msfinfo;
	LPCommonAttributeHeader attr = (LPCommonAttributeHeader)(msfinfo + sizeof(FILE_RECORD_HEADER));
	while (1)
	{
		//��Ҫ�Ǵ�MSF_INDEXALLOC_FLAG�в����ļ���������������MSF_INDEXROOT_FLAG
		if (attr->ATTR_Type == MSF_INDEXALLOC_FLAG)
		{
			if (attr->ATTR_ResFlag)
			{
				LPNonResidentAttributeHeader res = (LPNonResidentAttributeHeader)attr;
				unsigned char* data = (unsigned char*)attr + res->ATTR_DatOff;

				/*
				Run List��ֵΪ��32 CC 26 00 00 0C������Ϊ������00H������֪���Ѿ��ǽ�β��
				��һ���ֽ���ѹ���ֽڣ���λ�͵�λ��ӣ�3 + 2 = 5����ʾ���Data Run��Ϣռ������ֽڣ�
				���и�λ��ʾ��ʼ�غ�ռ�ö��ٸ��ֽڣ���λ��ʾ��Сռ�õ��ֽ�����
				��ʼ�غ�ռ��3���ֽڣ�ֵΪ0C 00 00��
				��Сռ��2���ֽڣ�ֵΪ26 CC��
				�����󣬵õ������������ʼ�غ�ΪC0000����СΪ26cc��
				*/

				unsigned int i = 0;
				unsigned __int64 clsno = 0;

				while (*(data + i))
				{
					int clscntbytes = (*(data + i) & 0xf);
					int clsnobytes = ((*(data + i) & 0xf0) >> 4);
					if (clscntbytes == 0 || clscntbytes > 4 || clsnobytes == 0 || clsnobytes > 4)
					{
						return FALSE;
						break;
					}

					i++;

					unsigned __int64 clscnt = 0;
					char* lpclscnt = (char*)&clscnt;
					for (int j = 0; j < clscntbytes; j++)
					{
						lpclscnt[j] = data[i + j];
					}
					i += clscntbytes;


					unsigned __int64 nextclsno = 0;
					char* lpclsno = (char*)&nextclsno;
					for (int k = 0; k < clsnobytes; k++)
					{
						lpclsno[k] = data[i + k];
					}
					i += clsnobytes;

					if (clscnt == 2 && (nextclsno & 0x8000))
					{
						nextclsno = nextclsno | 0xffffffffffff0000;
					}
					else if (clscnt == 3 && (nextclsno & 0x800000))
					{
						nextclsno = nextclsno | 0xffffffffff000000;
					}
					else if (clscnt == 4 && (nextclsno & 0x80000000))
					{
						nextclsno = nextclsno | 0xffffffff00000000;
					}

					clsno += nextclsno;

					__printf(szout, "searching filename:%s,previous path sector:%x,current cluster:%x,cluster total:%x in mft 0xA0\n",
						filename, secoff, clsno, clscnt);
					__drawGraphChars((unsigned char*)szout, 0);

					//����ֵ�ļ���:����������+ÿ�������� *�����غ�
					//�ļ����ң�A0�в���
					unsigned long long idxsecoff = gNtfsDbr.hideSectors + clsno * g_SecsPerCluster;

					DWORD low = idxsecoff & 0xffffffff;
					DWORD high = idxsecoff >> 32;

					int buffer_size = (int)(g_SecsPerCluster * clscnt * g_bytesPerSec);

					char* buffer = (char*)__kMalloc(buffer_size);

					ret = readSector(low, high, (DWORD)(g_SecsPerCluster * clscnt), (char*)buffer);
					if (ret <= 0)
					{
						__kFree((DWORD)buffer);

						__printf(szout, "getNtfsDir readSector error in file:%s,sector:%x,sector count:%x in 0xA0\n",
							filename, (DWORD)idxsecoff, g_SecsPerCluster * clscnt);
						__drawGraphChars((unsigned char*)szout, 0);
						break;
					}

					char* dirdata = (char*)buffer;
					for (int i = 0; i < clscnt; i++)
					{
						nextdirsec = getIdxNextDir(filename, (char*)dirdata);
						if (nextdirsec)
						{
							__kFree((DWORD)buffer);
							return nextdirsec;
						}
						dirdata += g_ClusterSize;
					}

					__kFree((DWORD)buffer);
				}

				break;
			}
			else {
				//break;
			}
		}
		//some times the "." directory is empty in attribute 0x90
		else if (attr->ATTR_Type == MSF_INDEXROOT_FLAG)
		{
			nextdirsec = getRootNextDir(attr, filename);
			if (nextdirsec)
			{
				return nextdirsec;
			}
			//can not continue here,need to add pointer to next block
// 			else {
// 				continue;
// 			}
		}
		else if (attr->ATTR_Type == 0xffffffff)	//end with flag == 0xffffffff and len == 0
		{
			break;
		}

		attr = (LPCommonAttributeHeader)((unsigned int)attr + attr->ATTR_Size);
		if ((char*)attr - msfinfo >= frh->BytesInUse || (char*)attr - msfinfo >= MFTEntrySize || (char*)attr - msfinfo <= 0)
		{
			break;
		}
	}

	return 0;
}





unsigned long long getNtfsFileData(unsigned long long secoff, char** buf) {
	int ret = 0;
	char szout[1024];

	char msfinfo[MFTEntrySize];

	DWORD low = secoff & 0xffffffff;
	DWORD high = secoff >> 32;
	ret = readSector((DWORD)low, high, 2, (char*)msfinfo);
	if (ret <= 0)
	{
		__printf((char*)szout, "getNtfsFileData read sector:%x error\n", (DWORD)secoff);
		__drawGraphChars((unsigned char*)szout, 0);
		return FALSE;
	}

	if (__memcmp(msfinfo, "FILE", 4))
	{
		__printf((char*)szout, "getNtfsFileData mft sector:%x format error\n", (DWORD)secoff);
		__drawGraphChars((unsigned char*)szout, 0);
		return FALSE;
	}

	FILE_RECORD_HEADER* frh = (FILE_RECORD_HEADER*)msfinfo;
	LPCommonAttributeHeader attr = (LPCommonAttributeHeader)(msfinfo + sizeof(FILE_RECORD_HEADER));
	while (1)
	{
		if (attr->ATTR_Type == MSF_DATA_FLAG)
		{
			if (attr->ATTR_ResFlag)
			{
				LPNonResidentAttributeHeader hdr = (LPNonResidentAttributeHeader)attr;
				unsigned char* runlist = hdr->ATTR_DatOff + (unsigned char*)hdr;

				if (*buf == 0)
				{
					*buf = (char*)__kMalloc((DWORD)hdr->ATTR_ValidSz);
				}
				char* lpdata = (char*)*buf;

				//�ʼ������Ϊ32,���и�4bits��ʾ������ʼ�ص�ַռ���ֽ���,��4bitsΪ���ݴ�С(�ظ���)ռ���ֽ���
				//����Ҫ�ر�ע�⣬��һ����ʼ����Ϣ���޷��ŵģ�����ڶ�����ʼ���������ǰ��һ���ص�ƫ�ƣ�����������
				int i = 0;
				unsigned __int64 clsno = 0;

				while (*(runlist + i))
				{
					unsigned char count = *(runlist + i);
					i++;

					int clscntbytes = (count & 0xf);
					if (clscntbytes <= 0 || clscntbytes > 4)
					{
						break;
					}

					int clsnobytes = ((count & 0xf0) >> 4);
					if (clsnobytes <= 0 || clsnobytes > 4)
					{
						break;
					}

					unsigned __int64 clscnt = 0;
					char* lpclscnt = (char*)&clscnt;
					for (int j = 0; j < clscntbytes; j++)
					{
						*lpclscnt = *(runlist + i);
						lpclscnt++;
						i++;
					}

					__int64 nextclsno = 0;
					char* lpclsno = (char*)&nextclsno;
					for (int j = 0; j < clsnobytes; j++)
					{
						*lpclsno = *(runlist + i);
						lpclsno++;
						i++;
					}

					if (clscnt == 2 && (nextclsno & 0x8000))
					{
						nextclsno = nextclsno | 0xffffffffffff0000;
					}
					else if (clscnt == 3 && (nextclsno & 0x800000))
					{
						nextclsno = nextclsno | 0xffffffffff000000;
					}
					else if (clscnt == 4 && (nextclsno & 0x80000000))
					{
						nextclsno = nextclsno | 0xffffffff00000000;
					}
					clsno += nextclsno;

					//to search directory in INDEX,the cluster offset is not relative to mft offset,but relative to hidden sector offset
					unsigned long long datasecoff = (clsno * g_SecsPerCluster + gNtfsDbr.hideSectors);
					DWORD low = datasecoff & 0xffffffff;
					DWORD high = datasecoff >> 32;

					ret = readSector(low, high, (DWORD)(clscnt * g_SecsPerCluster), (char*)*buf);
					if (ret <= 0)
					{
						__printf((char*)szout, "getNtfsFileData readSector:%x,count:%x error\n", (DWORD)datasecoff, clscnt * g_SecsPerCluster);
						__drawGraphChars((unsigned char*)szout, 0);
						return FALSE;
					}
					lpdata += clscnt * g_ClusterSize;

					//__printf((char*)szout,"getNtfsFileData read file sector:%x,count:%x ok\n",(DWORD)datasecoff,clscnt*g_SecsPerCluster);
					//__drawGraphChars((unsigned char*)szout, 0);
				}

				return hdr->ATTR_ValidSz;
			}
			else {
				LPResidentAttributeHeader hdr = (LPResidentAttributeHeader)attr;
				char* dataoff = hdr->ATTR_DatOff + (char*)hdr;
				if (*buf == 0)
				{
					*buf = (char*)__kMalloc(hdr->ATTR_DatSz);
				}
				__memcpy((char*)*buf, dataoff, hdr->ATTR_DatSz);
				return hdr->ATTR_DatSz;
			}
		}
		else if (attr->ATTR_Type == 0xffffffff)
		{
			break;
		}

		attr = (LPCommonAttributeHeader)((unsigned int)attr + attr->ATTR_Size);
		if ((char*)attr - msfinfo >= frh->BytesInUse || (char*)attr - msfinfo >= MFTEntrySize || (char*)attr - msfinfo <= 0)
		{
			break;
		}
	}

	return FALSE;
}




int writeNtfsFile(char* filename, char* buf, int size, int writemode) {
	return 0;
}



int readNtfsFile(char* filename, char** buf) {

	if (buf == 0)
	{
		return 0;
	}

	int ret = 0;
	unsigned char szout[1024];

	char subpath[MAX_PATH_SIZE];
	int result = removeDriver((char*)filename, subpath);

	char nextpath[MAX_PATH_SIZE];

	unsigned long long dirclsno = 0;

	unsigned long long secoff = gNtfsDbr.hideSectors + gNtfsDbr.MFT * g_SecsPerCluster + MSF_ROOTDIR_OFFSET;

	while (__strlen(subpath) > 0)
	{
		result = getNextPath(nextpath, subpath);

		if (__strlen(nextpath) > 0)
		{
			ret = upper2lower(nextpath, __strlen(nextpath));
			dirclsno = getNtfsDir(secoff, nextpath);
			if (dirclsno == FALSE)
			{
				__printf((char*)szout, "ntfs not found file:%s in mft,previous sector:%x\n", nextpath, (DWORD)secoff);
				__drawGraphChars((unsigned char*)szout, 0);
				break;
			}
			else {
				//��6�ֽ���Ŀ¼�����ļ���Ӧ���ļ���¼�ı��
				secoff = dirclsno * 2 + gNtfsDbr.hideSectors + gNtfsDbr.MFT * g_SecsPerCluster;
				if (*subpath == 0)
				{
					ret = (int)getNtfsFileData(secoff, buf);
					return ret;
				}
				else {
					//__printf((char*)szout, "found file:%s at sector:%x\n", nextpath, (DWORD)secoff);
					//__drawGraphChars((unsigned char*)szout, 0);
				}
			}
		}
		else {
			break;
		}
	}

	return FALSE;
}