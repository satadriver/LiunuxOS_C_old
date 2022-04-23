#include "sectorReader.h"
#include "video.h"
#include "def.h"
#include "Utils.h"


int __rmSectorReader(DWORD secno,DWORD seccnt,char * buf,int bufsize) {
	DWORD params[8];
	params[0] = secno;
	params[1] = seccnt;
	params[2] = (DWORD)buf;
	params[3] = bufsize;
	__asm {
		lea edi,params
		int 0feh
	}

	unsigned char szshow[4096];
	__memset((char*)szshow, 0, 4096);
	__dump(buf, 512, 0, szshow);
	__drawGraphChars((unsigned char*)szshow, 0);

	__asm {
		//_emit 0x90
	}
	return 0;
}