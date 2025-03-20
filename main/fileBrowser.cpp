
#include "Utils.h"
#include "Kernel.h"
#include "mouse.h"
#include "keyboard.h"
#include "video.h"
#include "task.h"
#include "Pe.h"
#include "console.h"

#include "fileBrowser.h"
#include "window.h"
#include "file.h"
#include "fat32/FAT32.h"
#include "fat32/Fat32File.h"
#include "NTFS/ntfs.h"
#include "NTFS/ntfsFile.h"
#include "NTFS/ntfsDirs.h"
#include "fat32/fat32dirs.h"
#include "guihelper.h"
#include "ios9660/ios9660.h"

#include "graph.h"
#include "soundBlaster/sbPlay.h"
#include "fat12/fat12.h"
#include "malloc.h"
#include "Thread.h"
#include "fileWindow.h"




int getPartitionInfo() {
	int ret = 0;
	int partitionType = 0;

	partitionType = getMBR();
	if (partitionType == FAT32_FILE_SYSTEM)
	{
		ret = getFat32DBR();
		if (ret <= 0)
		{
			return FALSE;
		}

		g_bytesPerSec = gFat32Dbr.BPB_BytesPerSec;
		g_SecsPerCluster = gFat32Dbr.BPB_SecPerClus;
		g_ClusterSize = g_bytesPerSec * gFat32Dbr.BPB_SecPerClus;
		g_FirstClusterNO = gFat32Dbr.BPB_RootClus;
		g_SecsBeforeRootDir = getRootDirOffset();
		gMaxDirsInPath = gFat32Dbr.BPB_SecPerClus * g_bytesPerSec / sizeof(FAT32DIRECTORY);

		ret = getFsinfo();

		fat32Init();
	}
	else if (partitionType == NTFS_FILE_SYSTEM)
	{
		ret = getNtfsDBR();

		g_bytesPerSec = gNtfsDbr.bytePerSector;
		g_SecsPerCluster = gNtfsDbr.secPerCluster;
		g_ClusterSize = g_SecsPerCluster * g_bytesPerSec;
		gMsfOffset = gNtfsDbr.hideSectors + gNtfsDbr.MFT * g_SecsPerCluster + MSF_ROOTDIR_OFFSET;

		ret = readMSFRoot();
	}
	else {

	}

	return partitionType;
}



int readFileDirs(int partitionType,unsigned __int64 secno, LPFILEBROWSER files, unsigned __int64 ntfslast) {
	if (partitionType == 2)
	{
		unsigned __int64 secoff = secno * 2 + gNtfsDbr.hideSectors + gNtfsDbr.MFT * g_SecsPerCluster;
		return getNtfsDirs(secoff, files, ntfslast);
	}
	else if (partitionType == 1)
	{
		return getFat32NextDirs((DWORD)secno, files);
	}
	else if (partitionType == 3)
	{
		return readIso9660Dirs((DWORD)secno, files);
	}
	else if (partitionType == 4)
	{
		return readFat12Dirs((DWORD)secno, files);
	}

	return 0;
}


int readFileData(int partitionType, unsigned __int64 secno, unsigned __int64 filesize, char* databuf, unsigned __int64 readsize) {

	if (partitionType == NTFS_FILE_SYSTEM)
	{
		unsigned __int64 secoff = secno * 2 + gNtfsDbr.hideSectors + gNtfsDbr.MFT * g_SecsPerCluster;
		return (DWORD)getNtfsFileData(secoff, &databuf);
	}
	else if (partitionType == FAT32_FILE_SYSTEM)
	{
		return fat32FileReader((DWORD)secno, (DWORD)filesize, databuf, (DWORD)readsize);
	}
	else if (partitionType == CDROM_FILE_SYSTEM)
	{
		int seccnt =(DWORD)( filesize / ATAPI_SECTOR_SIZE);
		int mod = filesize % ATAPI_SECTOR_SIZE;
		if (mod)
		{
			seccnt++;
		}
		return readIso9660File((DWORD)secno, seccnt, &databuf);
	}
	else if (partitionType == FLOPPY_FILE_SYSTEM)
	{
		return fat12FileReader((DWORD)secno, (DWORD)filesize, databuf, (DWORD)readsize);
	}
	else {
		return FALSE;
	}
}


int doOpenFile(int partitionType,LPFILEBROWSER files) {
	int result = 0;
	char szout[1024];
	__printf(szout, "doFileAction readFileData:%s size:%I64x\n", files->pathname, files->filesize);

	if (files->filesize > 0x10000000)
	{
		__printf(szout, "doFileAction filename:%s size:%I64x error\n", files->pathname, files->filesize);
		return FALSE;
	}

	char* buffer = (char*)__kMalloc((DWORD)files->filesize);
	int readsize = readFileData(partitionType,files->secno, files->filesize, (char*)buffer, files->filesize);
	if (readsize <= 0)
	{
		__printf(szout, "doFileAction readFileData:%s size:%I64x error\n", files->pathname, files->filesize);
		return FALSE;
	}

	int fnlen = __strlen(files->pathname);

	upper2lower(files->pathname, fnlen);

	TASKCMDPARAMS cmd;
	__memset((char*)&cmd, 0, sizeof(TASKCMDPARAMS));
	cmd.addr = (DWORD)buffer;
	cmd.filesize = (DWORD)files->filesize;
	__strcpy(cmd.filename, files->pathname);
	if (__memcmp(files->pathname + fnlen - 4, ".bmp", 4) == 0)
	{
		cmd.cmd = SHOW_WINDOW_BMP;

		DWORD thread = getAddrFromName(MAIN_DLL_BASE, "__kShowWindow");
		return __kCreateThread((DWORD)thread, MAIN_DLL_BASE, (DWORD)&cmd, "__kShowWindow_bmp");

		//return __kCreateProcess(VSMAINDLL_LOAD_ADDRESS, 0x100000, "main.dll", "__kShowWindow", 3, (DWORD)&cmd);
		//return __kCreateProcess(MAIN_DLL_BASE, 0x100000, "main.dll", "__kShowWindow", 3, (DWORD)&cmd);
	}
	else if (__memcmp(files->pathname + fnlen - 4, ".wav", 4) == 0)
	{
		result = sbplay((char*)buffer, readsize);

		//return playWavFile(files->pathname);
		__kFree((DWORD)buffer);
	}
	else if (isTxtFile(files->pathname, fnlen))
	{
		cmd.cmd = SHOW_WINDOW_TXT;

		//return __kCreateProcess(VSMAINDLL_LOAD_ADDRESS, 0x100000, "main.dll", "__kShowWindow", 3, (DWORD)&cmd);

		DWORD thread = getAddrFromName(MAIN_DLL_BASE, "__kShowWindow");
		return __kCreateThread((DWORD)thread, MAIN_DLL_BASE, (DWORD)&cmd, "__kShowWindow_txt");

		//return __kCreateThread((DWORD)__kShowWindow, (DWORD)&cmd, "__kShowWindow_txt");
	}
	else if (__memcmp(files->pathname + fnlen - 4, ".jpg", 4) == 0 || __memcmp(files->pathname + fnlen - 5, ".jpeg", 5) == 0)
	{
		cmd.cmd = SHOW_WINDOW_JPEG;

		//return __kCreateProcess(VSMAINDLL_LOAD_ADDRESS, 0x100000, "main.dll", "__kShowWindow", 3, (DWORD)&cmd);

		DWORD thread = getAddrFromName(MAIN_DLL_BASE, "__kShowWindow");
		return __kCreateThread((DWORD)thread, MAIN_DLL_BASE, (DWORD)&cmd, "__kShowWindow_jpg");
	}
	else if (__memcmp(files->pathname + fnlen - 4, ".exe", 4) == 0 || __memcmp(files->pathname + fnlen - 4, ".com", 4) == 0)
	{
		cmd.filesize = (DWORD)files->filesize;
		//return __kCreateThread((DWORD)__kShowWindow, (DWORD)&cmd, "__kShowWindow");
		return __kCreateProcess((unsigned int)cmd.addr, cmd.filesize, files->pathname, files->pathname, 3, (DWORD)&cmd);
	}
	return 0;
}



//�ļ���С�����Ա���ͳһ ���ļ��д�СͳһΪ0���ļ������Զ���0x10,�ļ����Զ���0x20
int __kFileManager(unsigned int retaddr, int tid, char* filename, char* funcname, DWORD param) {
	int ret = 0;
	char szout[1024];

	int partitionType = 0;

	LPTASKCMDPARAMS cmd = (LPTASKCMDPARAMS)param;
	//__printf(szout, "__kFileManager task tid:%x,name:%s,cmd:%d\n", tid, filename, cmd->cmd);

	if (cmd->cmd == UNKNOWN_FILE_SYSTEM)
	{
		partitionType = getPartitionInfo();
		if (partitionType <= 0)
		{
			__printf(szout, "__kFileManager preparePartitionInfo error\n");
			return 0;
		}
	}
	else {
		partitionType = cmd->cmd;
	}

	char fullpath[MAX_PATH_SIZE];
	__memset(fullpath, 0, MAX_PATH_SIZE);

	unsigned __int64 ntfsprevs[MAX_PATH_SIZE];
	DWORD ntfsseq = 0;

	int filetotal = 0;
	LPFILEBROWSER files = (LPFILEBROWSER)__kMalloc(sizeof(FILEBROWSER)* MAX_PATH_SIZE);
	if (partitionType == NTFS_FILE_SYSTEM)
	{
		unsigned __int64 ntfssecno = gNtfsDbr.hideSectors + gNtfsDbr.MFT * g_SecsPerCluster;
		ntfsprevs[ntfsseq] = MSF_ROOTDIR_OFFSET / 2;
		//__printf(szout, "ntfs root dir sector:%i64x\r\n", ntfssecno + MSF_ROOTDIR_OFFSET);
		filetotal = getNtfsDirs(ntfssecno + MSF_ROOTDIR_OFFSET, files, 0);
	}
	else if (partitionType == FAT32_FILE_SYSTEM) {

		filetotal = getFat32RootDirs((LPFAT32DIRECTORY)glpRootDir, files);
	}
	else if (partitionType == CDROM_FILE_SYSTEM)
	{
		filetotal = browseISO9660File(files);
	}
	else if (partitionType == FLOPPY_FILE_SYSTEM)
	{
		filetotal = browseFat12File(files);
	}

	if (filetotal <= 0)
	{
		__printf(szout, "not found any files\r\n");
		__kFree((DWORD)files);
		return FALSE;
	}
	else {
		for (int i = 0; i < filetotal; i++) {
			//__printf(szout, "num:%d file:%s type:%i64x size:%i64d\r\n",i, files[i].pathname, files[i].attrib, files[i].filesize);
		}
	}

	FMWINDOW window;
	window.window.tid = tid;
	LPPROCESS_INFO p = (LPPROCESS_INFO)CURRENT_TASK_TSS_BASE;
	window.window.pid = p->pid;
	__strcpy(window.window.caption, cmd->filename);
	drawFileManager(&window);

	//__printf(szout, "filetotal:%x,first:%s sector:%I64x size:%I64x,"
	//	"second:%s sector:%I64x size:%I64x,third:%s sector:%I64x size:%I64x,fourth:%s sector:%I64x size:%I64x\n",
	//	filetotal, files[0].pathname, files[0].secno, files[0].filesize, files[1].pathname, files[1].secno, files[1].filesize,
	//	files[2].pathname, files[2].secno, files[2].filesize, files[3].pathname, files[3].secno, files[3].filesize);

	int rowlimit = (gWindowHeight - window.window.capHeight - window.window.frameSize) / window.fsheight;
	int fpagecnt = 0;
	if (filetotal <= rowlimit)
	{
		fpagecnt = filetotal;
	}
	else {
		fpagecnt = rowlimit;
	}

	int pagecnt = filetotal / rowlimit;
	int pagemod = filetotal % rowlimit;
	if (pagemod)
	{
		pagecnt++;
	}
	pagecnt--;
	if (pagecnt < 0)
	{
		pagecnt = 0;
	}

	int number = 0;

	while (TRUE)
	{
		POINT p;
		p.x = window.window.showX;
		p.y = window.window.showY;
		ret = __drawRectWindow(&p, window.window.width, window.window.height, window.window.color, 0);
		for (number = 0; number < fpagecnt; number++)
		{
			//calc positon of char
			int y = (((number + 1) % rowlimit) * window.cpl* GRAPHCHAR_HEIGHT - GRAPHCHAR_HEIGHT - (window.cpl* GRAPHCHAR_HEIGHT / 2)) 
				+ window.window.showY;
			DWORD pos = __getpos(window.window.showX, y);

			char szinfo[4096];
			if (files[number].attrib & FILE_ATTRIBUTE_DIRECTORY)
			{
				int len = __sprintf(szinfo, "%s        DIR(%I64x)        %I64d(bytes)",
					files[number].pathname, files[number].attrib, files[number].filesize);

				__drawGraphChar(( char*)szinfo, FILE_DIR_FONT_COLOR, pos, window.window.fontcolor);
			}
			else if (files[number].attrib & FILE_ATTRIBUTE_ARCHIVE)
			{
				int len = __sprintf(szinfo, "%s        FILE(%I64x)       %I64d(bytes)",
					files[number].pathname, files[number].attrib, files[number].filesize);

				__drawGraphChar(( char*)szinfo, FILE_FILE_FONT_COLOR, pos, window.window.fontcolor);
			}
			else {
				int len = __sprintf(szinfo, "%s        UNKNOWN(%I64x)    %I64d(bytes)",
					files[number].pathname, files[number].attrib, files[number].filesize);

				__drawGraphChar(( char*)szinfo, FILE_UNKNOWN_FONT_COLOR, pos, window.window.fontcolor);
			}
		}

		//number is last dir
		if (number)
		{
			number--;
		}

		if (number < 0)
		{
			number = 0;
		}

		//number�����һ��Ŀ¼�����
		while (TRUE)
		{
			unsigned int asc = __kGetKbd(window.window.id) & 0xff;
			if (asc)
			{
				if (asc == VK_NEXT || asc == VK_DOWN || asc == VK_RIGHT || asc == VK_END )
				{
					//check if is last page
					if ((number / rowlimit) < pagecnt)
					{
						//to be first of next page
						number = ((number + rowlimit) / rowlimit) * rowlimit;
						if (filetotal - number >= rowlimit)
						{
							fpagecnt = rowlimit;
						}
						else {
							fpagecnt = filetotal - number;
						}
						break;
					}
				}
				else if (asc == VK_UP || asc == VK_PRIOR || asc == VK_LEFT || asc == VK_HOME )
				{
					//check if first page
					if ((number / rowlimit) > 0)
					{
						//to first of previous page
						number = ((number - rowlimit) / rowlimit) * rowlimit;
						if (filetotal - number >= rowlimit)
						{
							fpagecnt = rowlimit;
						}
						else {
							fpagecnt = filetotal - number;
						}
						break;
					}
				}
				else if (asc == 0x1b)
				{
					__kFree((DWORD)files);
					removeFileManager(&window);
					return 0;
				}
			}

			MOUSEINFO mouseinfo;
			__memset((char*)&mouseinfo, 0, sizeof(MOUSEINFO));
			//ret = getmouse(&mouseinfo, window.window.id);
			ret = __kGetMouse(&mouseinfo, window.window.id);
			if (mouseinfo.status & 1)
			{
				//y positon is page
				int y = (mouseinfo.y - window.window.capHeight - window.window.frameSize) / window.fsheight;

				//number positon in page
				int targetno = (number / rowlimit) * rowlimit + y;
				//__printf(szout, "__kFileManager filename:%s\n", files[targetno].pathname);
				if (targetno < filetotal /*&& files[targetno].filesize == 0*/ && files[targetno].attrib & FILE_ATTRIBUTE_DIRECTORY)
				{
					if (__strcmp(files[targetno].pathname, ".") == 0)
					{

					}
					else if (__strcmp(files[targetno].pathname, "..") == 0)
					{
						int len = getPrevPath(fullpath);
						ntfsseq--;
						if (ntfsseq <= 1 || ntfsseq >= MAX_PATH_SIZE)
						{
							ntfsseq = 1;
						}
					}
					else {
						__strcat(fullpath, files[targetno].pathname);
						__strcat(fullpath, "/");
						ntfsseq++;
						if (ntfsseq >= MAX_PATH_SIZE || ntfsseq<= 0)
						{
							ntfsseq = 1;
						}
						ntfsprevs[ntfsseq] = files[targetno].secno;
					}

					//sub root dir has directory ".." but without cluster number in ".."
					if (partitionType == 1 && __memcmp(files[targetno].pathname, "..", 2) == 0 && files[targetno].secno < g_FirstClusterNO)
					{
						files[targetno].secno = g_FirstClusterNO;
					}

					filetotal = readFileDirs(partitionType, files[targetno].secno, (LPFILEBROWSER)files, ntfsprevs[ntfsseq - 1]);
					if (filetotal > 0)
					{
						number = 0;
						pagecnt = filetotal / rowlimit;
						if (filetotal % rowlimit)
						{
							pagecnt++;
						}
						pagecnt--;
						if (pagecnt <= 0)
						{
							pagecnt = 0;
						}

						if (rowlimit > filetotal)
						{
							fpagecnt = filetotal;
						}
						else {
							fpagecnt = rowlimit;
						}
						break;
					}
				}
				else if (targetno < filetotal && files[targetno].attrib & FILE_ATTRIBUTE_ARCHIVE) {
					doOpenFile(partitionType,&files[targetno]);
				}
			}

			__sleep(0);
		}
	}

	__kFree((DWORD)files);
	return 0;
}







