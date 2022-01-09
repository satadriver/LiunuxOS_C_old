#include "ntfsFile.h"
#include "ntfs.h"
#include "../fat32/fileutils.h"
#include "../Utils.h"
#include "../satadriver.h"
#include "../file.h"
#include "../video.h"
#include "../slab.h"



//1 扇区读写64位的问题
//2 dataruns多个的处理测试
//3 中文文件名的处理
//4 小文件的处理是否有错误

int getDataRuns(char * data){

	return 0;
}



DWORD getIdxNextDir(char * filename,char * buf) {
	int ret = 0;

	//char szout[1024];

	if (__memcmp(buf,"INDX",4))
	{
		__drawGraphChars((unsigned char*)"getIdxNextDir format error\n", 0);
		return FALSE;
	}

	LPSTD_INDEX_HEADER hdr = (LPSTD_INDEX_HEADER)buf;
	char * baseaddr =  (char*)&hdr->SIH_IndexEntryOffset;
	LPSTD_INDEX_ENTRY idxentry = (LPSTD_INDEX_ENTRY)(baseaddr + hdr->SIH_IndexEntryOffset);
	while (1)
	{
		if (idxentry->SIE_IndexEntrySize >= hdr->SIH_IndexEntryAllocSize || idxentry->SIE_IndexEntrySize <= 0)
		{
			break;
		}

		if ((unsigned int)idxentry - (unsigned int)buf >= hdr->SIH_IndexEntryAllocSize)
		{
			break;
		}

		int fnlen = idxentry->SIE_FileNameSize;
		char * fn = (char*)((unsigned int)idxentry + sizeof(STD_INDEX_ENTRY) - 1);
		char ascfn[256];
		int asclen = unicode2asc((short*)fn, fnlen, ascfn);

		ret = upper2lower(ascfn, __strlen(ascfn));

		if (__strcmp(ascfn, filename) == 0)
		{
			return (DWORD)idxentry->SIE_MFTReferNumber ;
		}

		if ((DWORD)idxentry->SIE_MFTReferNumber == 0 && 
			(idxentry->SIE_IndexEntrySize < sizeof(STD_INDEX_ENTRY) || idxentry->SIE_IndexEntrySize >= 0x200) )
		{
			idxentry = (LPSTD_INDEX_ENTRY)((unsigned int)idxentry + 0x60);
		}
		else {
			idxentry = (LPSTD_INDEX_ENTRY)((unsigned int)idxentry + idxentry->SIE_IndexEntrySize);
		}
	}

// 	__printf(szout, "getIdxNextDir not found file:%s\n", filename);
// 	__drawGraphChars((unsigned char*)szout, 0);
	return FALSE;
}



DWORD getRootNextDir(LPCommonAttributeHeader hdr,char * filename) {
	int ret = 0;
	//char szout[1024];

	if (hdr->ATTR_ResFlag)
	{
		return FALSE;
	}

	LPResidentAttributeHeader rhdr = (LPResidentAttributeHeader)hdr;
	if (rhdr->ATTR_DatSz > 0 && rhdr->ATTR_DatOff > 0)
	{
		LPINDEX_ROOT lproot = (LPINDEX_ROOT)((unsigned int)rhdr + rhdr->ATTR_DatOff);
		LPINDEX_HEADER ih = (LPINDEX_HEADER)&lproot->IH;
		LPINDEX_ENTRY entry = (LPINDEX_ENTRY)(ih->IH_EntryOff + (unsigned int)ih);
		LPINDEX_ENTRY lpentry = entry;
		unsigned int entrysize = ih->IH_TalSzOfEntries- sizeof(INDEX_HEADER);
		while (1)
		{
			if (entry->IE_MftReferNumber == 0 || entry->IE_Size < sizeof(STD_INDEX_ENTRY))
			{
				break;
			}

			if ((unsigned int)entry - (unsigned int)lpentry  >= entrysize )
			{
				break;
			}

			int fnlen = entry->IE_FileNameSize;
			char * fn = (char*)((unsigned int)entry + sizeof(INDEX_ENTRY) - 1);
			char ascfn[256];
			int asclen = unicode2asc((short*)fn, fnlen, ascfn);

			ret = upper2lower(ascfn, __strlen(ascfn));

			if (__strcmp(ascfn, filename) == 0)
			{
				return (DWORD)entry->IE_MftReferNumber;
			}

			entry = (LPINDEX_ENTRY)((unsigned int)entry + entry->IE_Size);
		}
	}

	//__printf(szout, "getRootNextDir not found file:%s\n", filename);
	//__drawGraphChars((unsigned char*)szout, 0);

	return FALSE;
}

DWORD getNtfsDir(DWORD secoff, char * filename)
{
	int ret = 0;

	char szout[1024];

	char msfinfo[MFTEntrySize*2];

	DWORD low = secoff & 0xffffffff;
	DWORD high = 0;
	ret = readSector(low, high, 2, (char*)msfinfo);
	if (ret <= 0)
	{
		__printf(szout, "getNtfsDir readSector mft low:%x,high:%x error\n", low, high);
		__drawGraphChars((unsigned char*)szout, 0);
		return FALSE;
	}

	if (__memcmp(msfinfo, "FILE", 4))
	{
		__printf(szout, "getNtfsDir mft format error at file:%s,sector:%x\n", filename, low);
		__drawGraphChars((unsigned char*)szout, 0);
		return FALSE;
	}

	DWORD nextdirsec = 0;
	char * buffer[0x8000];

	FILE_RECORD_HEADER * frh = (FILE_RECORD_HEADER*)msfinfo;
	LPCommonAttributeHeader attr = (LPCommonAttributeHeader)(msfinfo + sizeof(FILE_RECORD_HEADER));
	while (1)
	{
		if (attr->ATTR_Type == MSF_INDEXALLOC_FLAG)
		{
			if (attr->ATTR_ResFlag)
			{
				LPNonResidentAttributeHeader res = (LPNonResidentAttributeHeader)attr;
				unsigned char * data = (unsigned char*)attr + res->ATTR_DatOff;

				unsigned int clsno = 0;

				unsigned int i = 0;

				int flag = 0;

				while (*(data + i))
				{
					int clscnt = 0;

					int clscntbytes = (*(data + i) & 0xf);

					int clsnobytes = ((*(data + i) & 0xf0) >> 4);

					i++;

					char * lpclscnt = (char*)&clscnt;

					short tmpclsno = 0;
					char * lpclsno = 0;
					if (flag)
					{
						lpclsno = (char*)&tmpclsno;
					}
					else {
						lpclsno = (char*)&clsno;
						flag = 1;
					}

					for (int j = 0; j < clscntbytes; j++)
					{
						*lpclscnt = *(data + i);
						lpclscnt++;
						i++;
					}

					for (int j = 0; j < clsnobytes; j++)
					{
						*lpclsno = *(data + i);
						lpclsno++;
						i++;
					}

					clsno += (int)tmpclsno;

					__printf(szout, "filename:%s,sector:%x,cluster no:%x,count:%x\n", filename, secoff, clsno, clscnt);
					__drawGraphChars((unsigned char*)szout, 0);

					//__printf(szout,"searching path:%s cluster:%x,cluster size:%x in mft 0xA0...\n", filename,clsno,clscnt);
					//__drawGraphChars((unsigned char*)szout, 0);

					DWORD idxsecoff = gNtfsDbr.hideSectors + clsno*g_SecsPerCluster;
					//why must translate unsigned __int64 to dword?else error?
					DWORD low = idxsecoff & 0xffffffff;
					DWORD high = 0;
					ret = readSector((DWORD)low, high, g_SecsPerCluster*clscnt, (char*)buffer);
					if (ret <= 0)
					{
						__printf(szout, "getNtfsDir readSector at file:%s,sector:%x,count:%x error in index\n",
							filename, (DWORD)idxsecoff, g_SecsPerCluster*clscnt);
						__drawGraphChars((unsigned char*)szout, 0);
						break;
					}

					char * dirdata = (char*)buffer;
					for (int i = 0; i < clscnt; i++)
					{
						DWORD nextdirsec = getIdxNextDir(filename, (char*)dirdata);
						if (nextdirsec)
						{
							return nextdirsec;
						}
						dirdata += g_ClusterSize;
					}
				}

				break;
			}
			else {
				//break;
			}
		}
		else if (attr->ATTR_Type == MSF_INDEXROOT_FLAG)
		{
			nextdirsec = getRootNextDir(attr, filename);
			if (nextdirsec)
			{
				break;
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

	return nextdirsec;
}





DWORD getNtfsFileData(DWORD secoff,char ** buf) {
	int ret = 0;
	char szout[1024];

	char msfinfo[MFTEntrySize];

	//why need to change sector no from 64bits to 32bits?
	DWORD low = secoff & 0xffffffff;
	//注意这种写法是循环移位，右移32位后还是原来的值
	//DWORD high = secoff >> 32;
	DWORD high = 0;
	ret = readSector((DWORD)low,high, 2, (char*)msfinfo);
	if (ret <= 0)
	{
		__printf((char*)szout,"getNtfsFileData read sector:%s error\n",(DWORD)secoff);
		__drawGraphChars((unsigned char*)szout, 0);
		return FALSE;
	}

	if (__memcmp(msfinfo,"FILE",4))
	{
		__printf((char*)szout, "getNtfsFileData mft sector:%x format error\n", (DWORD)secoff);
		__drawGraphChars((unsigned char*)szout, 0);
		return FALSE;
	}

	FILE_RECORD_HEADER *frh = (FILE_RECORD_HEADER*)msfinfo;
	LPCommonAttributeHeader attr = (LPCommonAttributeHeader)(msfinfo + sizeof(FILE_RECORD_HEADER));
	while (1)
	{
		if (attr->ATTR_Type == MSF_DATA_FLAG)
		{
			if (attr->ATTR_ResFlag)
			{
				LPNonResidentAttributeHeader hdr = (LPNonResidentAttributeHeader)attr;
				unsigned char * runlist = hdr->ATTR_DatOff + (unsigned char*)hdr;
				if (*buf == 0)
				{
					*buf = (char*)__kMalloc((DWORD)hdr->ATTR_ValidSz);
				}
				char * lpdata = (char*)*buf;

				int flag = 0;

				unsigned int clsno = 0;
				//最开始的数据为32,其中高4bits表示数据起始簇地址占用字节数,后4bits为数据大小(簇个数)占用字节数
				//这里要特别注意，第一个起始簇信息是无符号的，后面第二个开始就是相对于前面一个簇的偏移，是有正负的
				int i = 0;

				while (*(runlist + i))
				{
					int clscnt = 0;

					unsigned char count = *(runlist + i);
					i ++;

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

					short clsoff = 0;
					char * lpclsno = 0;
					if (flag == 0)
					{
						lpclsno = (char*)&clsno;
						flag = 1;
					}
					else {
						lpclsno = (char*)&clsoff;
					}

					char * lpclscnt = (char*)&clscnt;
					for (int j = 0; j < clscntbytes; j++)
					{
						*lpclscnt = *(runlist + i);
						lpclscnt++;
						i ++;
					}

					for (int j = 0; j < clsnobytes; j++)
					{
						*lpclsno = *(runlist + i);
						lpclsno++;
						i ++;
					}

					clsno += (int)clsoff;

					//not mft offset,buf hidden sector offset
					DWORD datasecoff = clsno*g_SecsPerCluster + gNtfsDbr.hideSectors;
					DWORD low = datasecoff & 0xffffffff;
					//注意这种写法是循环移位，右移32位后还是原来的值
					//DWORD high = datasecoff >> 32;
					DWORD high = 0;
					ret = readSector((DWORD)low,high, clscnt*g_SecsPerCluster, (char*)lpdata);
					if (ret <= 0)
					{
						__printf((char*)szout,"getNtfsFileData readSector:%x,count:%x error\n",(DWORD)datasecoff,clscnt*g_SecsPerCluster);
						__drawGraphChars((unsigned char*)szout, 0);
						return FALSE;
					}
					lpdata += clscnt*g_ClusterSize;

					//__printf((char*)szout,"getNtfsFileData read file sector:%x,count:%x ok\n",(DWORD)datasecoff,clscnt*g_SecsPerCluster);
					//__drawGraphChars((unsigned char*)szout, 0);
				}

				return (DWORD)hdr->ATTR_ValidSz;
			}
			else {
				LPResidentAttributeHeader hdr = (LPResidentAttributeHeader)attr;
				char * dataoff = hdr->ATTR_DatOff + (char*)hdr;
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




int writeNtfsFile(char * filename, char * buf, int size,  int writemode) {
	return 0;
}


//以后有机会改写为64位分区操作!!!
int readNtfsFile(char * filename, char ** buf) {

	if (buf == 0)
	{
		return 0;
	}

	int ret = 0;
	unsigned char szout[1024];

	char subpath[MAX_PATH_SIZE];
	int result = removeDriver((char*)filename, subpath);

	char nextpath[MAX_PATH_SIZE];

	DWORD dirclsno =0;

	DWORD secoff = gNtfsDbr.hideSectors + (DWORD)gNtfsDbr.MFT * g_SecsPerCluster + MSF_ROOTDIR_OFFSET;

	while (__strlen(subpath) > 0)
	{
		result = getNextPath(nextpath, subpath);

		if (__strlen(nextpath) > 0)
		{
 			ret = upper2lower(nextpath, __strlen(nextpath));
			dirclsno = getNtfsDir(secoff, nextpath);
			if (dirclsno == FALSE)
			{
				__printf((char*)szout, "ntfs not found file:%s at mft sector:%x\n", nextpath, (DWORD)secoff);
				__drawGraphChars((unsigned char*)szout, 0);
				break;
			}
			else {
				//低6字节是目录或者文件对应的文件记录的编号
				secoff = dirclsno * 2 + gNtfsDbr.hideSectors + (DWORD)gNtfsDbr.MFT * g_SecsPerCluster;
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