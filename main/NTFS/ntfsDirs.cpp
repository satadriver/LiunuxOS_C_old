
#include "../def.h"
#include "ntfs.h"
#include "ntfsDirs.h"
#include "ntfsFile.h"
#include "../Utils.h"
#include "../video.h"
#include "../satadriver.h"
#include "../file.h"
#include "../FileManager.h"
#include "../fat32/fileutils.h"
#include "../file.h"
#include "../slab.h"



DWORD getIndexDirs( char * buf, LPFILEBROWSER pathname) {

	int cnt = 0;

	int ret = 0;

	if (__memcmp(buf, "INDX", 4))
	{
		__drawGraphChars((unsigned char*)"getIdxNextDir format error\n", 0);
		return cnt;
	}

	LPSTD_INDEX_HEADER hdr = (LPSTD_INDEX_HEADER)buf;
	char * baseaddr = (char*)&hdr->SIH_IndexEntryOffset;
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

		__strcpy(pathname->pathname, ascfn);
		pathname->secno = (DWORD)idxentry->SIE_MFTReferNumber;
		pathname->filesize = (DWORD)idxentry->SIE_FileRealSize;
		pathname->attrib = (DWORD)idxentry->SIE_FileFlag;
		if (pathname->attrib & 0x10000000)
		{
			pathname->attrib |= FILE_ATTRIBUTE_DIRECTORY;
		}
		else {
			pathname->attrib |= FILE_ATTRIBUTE_ARCHIVE;
		}
		pathname++;
		cnt++;

		if ((DWORD)idxentry->SIE_MFTReferNumber == 0 &&
			(idxentry->SIE_IndexEntrySize < sizeof(STD_INDEX_ENTRY) || idxentry->SIE_IndexEntrySize >= 0x200))
		{
			idxentry = (LPSTD_INDEX_ENTRY)((unsigned int)idxentry + 0x60);
		}
		else {
			idxentry = (LPSTD_INDEX_ENTRY)((unsigned int)idxentry + idxentry->SIE_IndexEntrySize);
		}
		
	}

	return cnt;
}



DWORD getDirsIndexRoot(LPCommonAttributeHeader hdr, LPFILEBROWSER pathname) {
	int ret = 0;
	

	if (hdr->ATTR_ResFlag)
	{
		return FALSE;
	}

	int cnt = 0;

	LPResidentAttributeHeader rhdr = (LPResidentAttributeHeader)hdr;
	if (rhdr->ATTR_DatSz > 0 && rhdr->ATTR_DatOff > 0)
	{
		LPINDEX_ROOT lproot = (LPINDEX_ROOT)((unsigned int)rhdr + rhdr->ATTR_DatOff);
		LPINDEX_HEADER ih = (LPINDEX_HEADER)&lproot->IH;
		LPINDEX_ENTRY entry = (LPINDEX_ENTRY)(ih->IH_EntryOff + (unsigned int)ih);
		LPINDEX_ENTRY lpentry = entry;
		unsigned int entrysize = ih->IH_TalSzOfEntries - sizeof(INDEX_HEADER);
		while (1)
		{
			if ( entry->IE_Size <= sizeof(STD_INDEX_ENTRY))
			{
				break;
			}

			if ((unsigned int)entry - (unsigned int)lpentry >= entrysize)
			{
				break;
			}

			int fnlen = entry->IE_FileNameSize;
			char * fn = (char*)((unsigned int)entry + sizeof(INDEX_ENTRY) - 1);
			char ascfn[256];
			int asclen = unicode2asc((short*)fn, fnlen, ascfn);

			ret = upper2lower(ascfn, __strlen(ascfn));

			__strcpy(pathname->pathname, ascfn);
			pathname->secno = (DWORD)entry->IE_MftReferNumber;

			pathname->filesize = (DWORD)entry->IE_FileRealSize;
			pathname->attrib = (DWORD)entry->IE_FileFlag;
			if (pathname->attrib & 0x10000000)
			{
				pathname->attrib |= FILE_ATTRIBUTE_DIRECTORY;
			}
			else {
				pathname->attrib |= FILE_ATTRIBUTE_ARCHIVE;
			}

			pathname++;
			cnt++;

			entry = (LPINDEX_ENTRY)((unsigned int)entry + entry->IE_Size);
		}
	}

	return cnt;
}


int getNtfsDirs(DWORD secoff, LPFILEBROWSER pathname,DWORD father) {
	int ret = 0;

	int cnt = 0;

	//if (father)
	{
		__strcpy(pathname->pathname, "..");
		pathname->secno = father;
		pathname->attrib = FILE_ATTRIBUTE_DIRECTORY;
		pathname->filesize = 0;
		pathname++;
		cnt++;
	}

	char szout[1024];

	DWORD low = secoff & 0xffffffff;
	DWORD high = 0;
	char msfinfo[MFTEntrySize * 2];
	ret = readSector(low, high, 2, (char*)msfinfo);
	if (ret <= 0)
	{
		__printf(szout, "getNtfsDirs readSector mft low:%x,high:%x error\n", low, high);
		__drawGraphChars((unsigned char*)szout, 0);
		return cnt;
	}

	char * buffer = (char*)__kMalloc(g_ClusterSize<<1);

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

				int i = 0;

				int flag = 0;

				while (*(data + i))
				{
					int clscntbytes = (*(data + i) & 0xf);

					int clsnobytes = ((*(data + i) & 0xf0) >> 4);

					i++;

					int clscnt = 0;

					char * lpclscnt = (char*)&clscnt;

					short nextclsno = 0;

					char * lpclsno = 0;
					if (flag)
					{
						lpclsno = (char*)&nextclsno;
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

					clsno += (int)nextclsno;

					__printf(szout, "filename:%s,sector:%x,cluster no:%x,count:%x\n", pathname, secoff, clsno, clscnt);
					__drawGraphChars((unsigned char*)szout, 0);

					DWORD idxsecoff = gNtfsDbr.hideSectors + clsno*g_SecsPerCluster;
					DWORD low = idxsecoff & 0xffffffff;
					DWORD high = 0;
					
					ret = readSector((DWORD)low, high, g_SecsPerCluster*clscnt, (char*)buffer);
					if (ret <= 0)
					{
						__printf(szout, "getNtfsDirs readSector at sector:%x,count:%x error in index\n", (DWORD)idxsecoff, g_SecsPerCluster*clscnt);
						__drawGraphChars((unsigned char*)szout, 0);
						break;
					}
					else {
						char * dirdata = (char*)buffer;
						for (int i = 0; i < clscnt; i++)
						{
							cnt += getIndexDirs((char*)dirdata, pathname);
							pathname += cnt;
							dirdata += g_ClusterSize;
						}

						__kFree((DWORD)buffer);
						return cnt;
					}
				}
			}
		}
		else if (attr->ATTR_Type == MSF_INDEXROOT_FLAG)
		{
			cnt += getDirsIndexRoot(attr, pathname);
			pathname += cnt;
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

	__kFree((DWORD)buffer);
	return cnt;
}