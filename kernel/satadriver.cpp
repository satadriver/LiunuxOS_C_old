#include "satadriver.h"
#include "def.h"
#include "Utils.h"
#include "pci.h"
#include "task.h"
#include "Utils.h"
#include "video.h"
#include "atapi.h"
#include "Kernel.h"

WORD gHdBasePort = 0;
DWORD gHdBDF = 0;
WORD gCDROMBasePort = 0;

DWORD gMSDev = 0;
DWORD gMimo = 0;
DWORD gAtaIRQ = 0;
DWORD gSecMax = ONCE_READ_LIMIT;



int(__cdecl* readSector)(unsigned int secnolow, DWORD secnohigh, unsigned int seccnt, char* buf) = readPortSector;

int(__cdecl* writeSector)(unsigned int secnolow, DWORD secnohigh, unsigned int seccnt, char* buf) = writePortSector;




//在编译器参数里（项目->属性->配置属性->C/C++->命令行->其他选项）
//说明如下:/Gs 设置堆栈检查字节数. 我们使用/Gs8192设置堆栈检查字节为8MB,经测试,对 __chkstk的调用没有了,成功达到我们的目的.



//376端口控制说明:
//读端口时跟1f6一致，一般为50h
//写入时得低4位:
//bit0: always be 0
//bit1:1关中断，0开中断
//bit2:1复位磁盘,0不复位磁盘
//bit3:always be 0
//bit4-bit7:读取端口得值跟1f7读取得高4位一致
void __initStatusPort(unsigned char master_slave) {
	unsigned short port = 0;
	if (master_slave == 0xe0)
	{
		port = 0x3f6;
	}
	else if (master_slave == 0xf0)
	{
		port = 0x376;
	}
	else {
		return;
	}

	__asm {
		push edx

		mov dx, port

		mov al, 0		//开中断不复位
		mov al, 02h		//关中断不复位
		mov al, 06h		//关中断复位磁盘
		mov al, 04h		//开中断复位

		//这才是真正的磁盘复位命令
		mov al, 4

		out dx, al

		mov al, 0

		out dx, al

		pop edx
	}
}


//1f7读出的是0x58，atapi 170读出的是0x41，含义不同

//1f7
//bit0 之前的命令发生错误 if is 1
//bit1 磁盘每转一周等于1
//bit2 ecc check correctly to read sector data if is 1
//bit3 work complete if is 1
//bit4 磁头停留在磁道上 if is 1
//bit5	write error if 1
//bit6 ready to work if is 1
//bit7 be busy if is 1

//1f6
//bit7:1
//bit6:1 is lba,0 is chs
//bit5:1
//bit4:0 is master,1 is slave
//bit0-bit3:if bit6 is 1,the sector no of 24-27,if bit6 is 0,header number

//柱面（cylinder），磁头（head）,sector
//LBA(逻辑扇区号)=磁头数 × 每磁道扇区数 × 当前所在柱面号 + 每磁道扇区数 × 当前所在磁头号 + 当前所在扇区号 – 1
//CHS=0/0/1，则根据公式LBA=255 × 63 × 0 + 63 × 0 + 1 – 1= 0
//CHS模式支持的硬盘 用8bit来存储磁头地址，用10bit来存储柱面地址，用6bit来存储扇区地址，
//而一个扇区共有512Byte，这样使用CHS寻址一块硬盘最大容量为256 * 1024 * 63 * 512B = 8064 MB
int testHdPort(unsigned short port) {
	__asm {
		mov dx, port
		in al, dx
		//test al,0xa5
		CMP AL, 0X50
		jnz _checkHdPortErr
		mov eax, 1
		jmp _checkHdPortEnd
		_checkHdPortErr :
		mov eax, 0
		_checkHdPortEnd :
	}
}

int testHdPortMimo(unsigned short port) {
	__asm {
		movzx edx, port
		mov al, [edx]
		//test al, 0xa5
		CMP AL, 0X50
		jnz _checkHdPortErr
		mov eax, 1
		jmp _checkHdPortEnd
		_checkHdPortErr :
		mov eax, 0
		_checkHdPortEnd :
	}
}

int getHdPort() {

	unsigned char szshow[1024];

	int ret = 0;

	ret = testHdPort(0x3f7);
	if (ret)
	{
		gHdBasePort = 0x3f0;
		gSecMax = ONCE_READ_LIMIT;
		gMSDev = 0xf0;

		__asm {
			mov dx, 3f7h
			dec dx
			in al, dx
			and al, 0f0h
			movzx eax, al
			mov gMSDev, eax
		}

		__printf((char*)szshow, "get ide hd slave port:%x,device:%x\n", gHdBasePort, gMSDev);


		ret = testHdPort(0x377);
		if (ret)
		{
			gCDROMBasePort = 0x370;

			__printf((char*)szshow, "get ide cdrom slave port:%x\n", gCDROMBasePort);

		}

		readSector = readPortSector;
		writeSector = writePortSector;
		// 		readSector = vm86ReadSector;
		// 		writeSector = vm86WriteSector;

		return TRUE;
	}


	ret = testHdPort(0x1f7);
	if (ret)
	{
		gHdBasePort = 0x1f0;
		gSecMax = ONCE_READ_LIMIT;
		gMSDev = 0xe0;

		__asm {
			mov dx, 1f7h
			dec dx
			in al, dx
			and al, 0f0h
			movzx eax, al
			mov gMSDev, eax
		}

		__printf((char*)szshow, "get ide hd master port:%x,device:%x\n", gHdBasePort, gMSDev);


		ret = testHdPort(0x177);
		if (ret)
		{
			gCDROMBasePort = 0x170;

			__printf((char*)szshow, "get ide cdrom master port:%x\n", gCDROMBasePort);


			// 			ret = checkAtapiPort(gCDROMBasePort + 7);
			// 			if (ret > 0)
			// 			{
			// 				ret = atapiCmd(gAtapiCmdOpen);
			// 
			// 				char szout[8192];
			// 				ret = readAtapiSector(szout, 0, 1);
			// 
			// 				unsigned char buffer[8192];
			// 				__dump((char*)szout, 2048, 0, buffer);
			// 				__drawGraphChars((unsigned char*)buffer, 0);
			// 			}
		}

		readSector = readPortSector;
		writeSector = writePortSector;
		// 		readSector = vm86ReadSector;
		// 		writeSector = vm86WriteSector;
		return TRUE;
	}

	DWORD hdport[16];
	int cnt = getBasePort(hdport, 0x0101, &gHdBDF, &gAtaIRQ);
	for (int i = 0; i < cnt; i++)
	{
		if (hdport[i])
		{
			if (i & 1)
			{
				gMSDev = 0xf0;
			}
			else {
				gMSDev = 0xe0;
			}

			gSecMax = 128;

			if ((hdport[i] & 1) == 0)
			{
				gMimo = 1;

				hdport[i] = hdport[i] & 0xfff0;

				ret = testHdPortMimo(hdport[i] + 7);
				if (ret)
				{
					gHdBasePort = hdport[i];

					__printf((char*)szshow, "get sata hd mimo:%x,master_slave:%x\n", gHdBasePort, gMSDev);


					ret = testHdPortMimo(hdport[i + 1] + 7);
					if (ret)
					{
						gCDROMBasePort = hdport[i + 1];

						__printf((char*)szshow, "get sata cdrom mimo:%x,master_slave:%x\n", gCDROMBasePort, gMSDev);

					}

					return TRUE;
				}
			}
			else {
				ret = testHdPort((hdport[i] & 0xfffe) + 7);
				if (ret)
				{
					gHdBasePort = hdport[i] & 0xfffe;

					__printf((char*)szshow, "get sata hd port:%x,master_slave:%x\n", gHdBasePort, gMSDev);


					ret = testHdPort((hdport[i + 1] & 0xfffe) + 7);
					if (ret)
					{
						gCDROMBasePort = hdport[i + 1] & 0xfffe;

						__printf((char*)szshow, "get sata cdrom port:%x,master_slave:%x\n", gCDROMBasePort, gMSDev);

					}

					return TRUE;
				}
			}
		}
	}

	__drawGraphChars((unsigned char*)"int13h\n", 0);
	readSector = vm86ReadSector;
	writeSector = vm86WriteSector;
	return TRUE;
}




int vm86ReadBlock(unsigned int secno, DWORD secnohigh, unsigned short seccnt, char* buf, int disk, int sectorsize) {

	unsigned int counter = 0;

	LPV86VMIPARAMS params = (LPV86VMIPARAMS)V86VMIPARAMS_ADDRESS;
	while (params->bwork == 1)
	{
		__sleep(0);
		counter++;
		if (counter && (counter % 256 == 0))
		{
			__drawGraphChars((unsigned char*)"bwork is 1,work not start\n", 0);
		}
	}

	params->intno = 0x13;
	params->reax = 0x4200;
	params->recx = 0;
	params->redx = disk;
	params->rebx = 0;
	params->resi = V86VMIDATA_OFFSET;
	params->redi = 0;
	params->res = 0;
	params->rds = V86VMIDATA_SEG;
	params->result = 0;

	LPINT13PAT pat = (LPINT13PAT)V86VMIDATA_ADDRESS;
	pat->len = 0x10;
	pat->reserved = 0;
	pat->seccnt = seccnt;
	pat->segoff = (INT13_RM_FILEBUF_SEG << 16) + INT13_RM_FILEBUF_OFFSET;
	pat->secnolow = secno;
	pat->secnohigh = secnohigh;

	params->bwork = 1;

	while (params->bwork == 1)
	{
		__sleep(0);
		counter++;
		if (counter && (counter % 256 == 0))
		{
			__drawGraphChars((unsigned char*)"bwork is 1,wait to complete\n", 0);
		}
	}

	if (params->result > 0)
	{
		__memcpy(buf, (char*)INT13_RM_FILEBUF_ADDR, seccnt * sectorsize);
		return seccnt * sectorsize;
	}

	return 0;
}


int vm86WriteBlock(unsigned int secno, DWORD secnohigh, unsigned short seccnt, char* buf, int disk, int sectorsize) {

	LPV86VMIPARAMS params = (LPV86VMIPARAMS)V86VMIPARAMS_ADDRESS;
	while (params->bwork == 1)
	{
		__sleep(0);
	}

	params->intno = 0x13;
	params->reax = 0x4300;
	params->recx = 0;
	params->redx = disk;
	params->rebx = 0;
	params->resi = V86VMIDATA_OFFSET;
	params->redi = 0;
	params->res = 0;
	params->rds = V86VMIDATA_SEG;
	params->result = 0;

	__memcpy((char*)INT13_RM_FILEBUF_ADDR, buf, seccnt * sectorsize);

	LPINT13PAT pat = (LPINT13PAT)V86VMIDATA_ADDRESS;
	pat->len = 0x10;
	pat->reserved = 0;
	pat->seccnt = seccnt;
	pat->segoff = (INT13_RM_FILEBUF_SEG << 16) + INT13_RM_FILEBUF_OFFSET;
	pat->secnolow = secno;
	pat->secnohigh = secnohigh;

	params->bwork = 1;

	while (params->bwork == 1)
	{
		__sleep(0);
	}

	if (params->result)
	{
		return seccnt * sectorsize;
	}
	return 0;
}


int vm86ReadSector(unsigned int secno, DWORD secnohigh, unsigned int seccnt, char* buf) {

	int readcnt = seccnt / ONCE_READ_LIMIT;
	int readmod = seccnt % ONCE_READ_LIMIT;
	int ret = 0;
	CHAR* offset = buf;
	for (int i = 0; i < readcnt; i++)
	{
		ret = vm86ReadBlock(secno, secnohigh, ONCE_READ_LIMIT, offset, 0x80, BYTES_PER_SECTOR);

		offset += (BYTES_PER_SECTOR * ONCE_READ_LIMIT);
		secno += ONCE_READ_LIMIT;
	}

	if (readmod)
	{
		ret = vm86ReadBlock(secno, secnohigh, readmod, offset, 0x80, BYTES_PER_SECTOR);
	}
	return ret;
}


int vm86WriteSector(unsigned int secno, DWORD secnohigh, unsigned int seccnt, char* buf) {

	int readcnt = seccnt / ONCE_READ_LIMIT;
	int readmod = seccnt % ONCE_READ_LIMIT;
	int ret = 0;
	CHAR* offset = buf;
	for (int i = 0; i < readcnt; i++)
	{
		ret = vm86WriteBlock(secno, secnohigh, ONCE_READ_LIMIT, offset, 0x80, BYTES_PER_SECTOR);

		offset += BYTES_PER_SECTOR * ONCE_READ_LIMIT;
		secno += ONCE_READ_LIMIT;
	}

	if (readmod)
	{
		ret = vm86WriteBlock(secno, secnohigh, readmod, offset, 0x80, BYTES_PER_SECTOR);
	}
	return ret;
}





int readPortSector(unsigned int secno, DWORD secnohigh, unsigned int seccnt, char* buf) {
	int readcnt = seccnt / gSecMax;
	int readmod = seccnt % gSecMax;
	int ret = 0;
	CHAR* offset = buf;
	for (int i = 0; i < readcnt; i++)
	{
		ret = readSectorLBA48(secno, secnohigh, gSecMax, offset, gMSDev);
		offset += (BYTES_PER_SECTOR * gSecMax);
		secno += gSecMax;
	}

	if (readmod)
	{
		ret = readSectorLBA48(secno, secnohigh, readmod, offset, gMSDev);
	}
	return ret;
}


int writePortSector(unsigned int secno, DWORD secnohigh, unsigned int seccnt, char* buf) {
	int readcnt = seccnt / gSecMax;
	int readmod = seccnt % gSecMax;
	int ret = 0;
	CHAR* offset = buf;
	for (int i = 0; i < readcnt; i++)
	{
		ret = writeSectorLBA48(secno, secnohigh, gSecMax, offset, gMSDev);
		offset += BYTES_PER_SECTOR * gSecMax;
		secno += gSecMax;
	}

	if (readmod)
	{
		ret = writeSectorLBA48(secno, secnohigh, readmod, offset, gMSDev);
	}
	return ret;
}



int waitComplete(WORD port) {
	__asm {
	_waitBusy:
		mov dx, port
			sub dx, 6
			in al, dx
			CMP AL, 0
			jz _nohdErr

			mov eax, -1
			jmp _checkhdCompleteEnd

			_nohdErr :
		mov dx, port
			in al, dx
			// 		test al,0xa5
			// 		jz _checkhdStatus
			// 
			// 		mov eax, -1
			// 		jmp _checkhdCompleteEnd
			// 
			// 		_checkhdStatus:
			and al, 0fdh
			cmp al, 58h
			jnz _waitBusy

			mov eax, 0
			_checkhdCompleteEnd:
	}
}

int waitFree(WORD port) {
	__asm {
	_waitBusy:
		mov dx, port
			in al, dx
			test al, 80h
			jnz _waitBusy
			test al, 40h
			jz _waitBusy
	}
}


int readSectorLBA24(unsigned int secno, unsigned char seccnt, char* buf, int device) {

	waitFree(gHdBasePort + 7);

	__asm {
		cli

		mov eax, secno

		mov dx, gHdBasePort
		add dx, 3		//173
		out dx, al

		inc dx			//174
		shr eax, 8
		out dx, al

		inc dx			//175
		shr eax, 8
		out dx, al

		inc dx			//176
		shr eax, 8
		and al, 0fh
		or al, byte ptr device
		out dx, al

		sub dx, 4		//172
		mov al, seccnt
		out dx, al

		dec dx
		mov al, 0	//dma = 1,pio = 0
		out dx, al

		add dx, 6		//177
		mov al, HD_READ_COMMAND
		out dx, al
	}

	//waitComplete(gHdBasePort + 7);

	__asm {
		mov edi, buf
		cld
		movzx ecx, seccnt

		_readoneSector :
		push ecx

			movzx eax, gHdBasePort
			add eax, 7
			push eax
			//push gHdBasePort + 7
			call waitComplete
			add esp, 4
			cmp eax, 0
			jz _readSectorLBA24

			pop ecx
			jmp _readSectorLBA24End

			_readSectorLBA24 :
		mov dx, gHdBasePort		//170
			mov ecx, BYTES_PER_SECTOR / 4

			rep insd

			pop ecx
			loop _readoneSector

			_readSectorLBA24End :
		mov eax, edi
			sub eax, buf

			sti
	}
}



int writeSectorLBA24(unsigned int secno, unsigned char seccnt, char* buf, int device) {

	waitFree(gHdBasePort + 7);

	__asm {
		cli

		mov eax, secno

		mov dx, gHdBasePort
		add dx, 3		//173
		out dx, al

		inc dx			//174
		shr eax, 8
		out dx, al

		inc dx			//175
		shr eax, 8
		out dx, al

		inc dx			//176
		shr eax, 8
		and al, 0fh
		or al, byte ptr device
		out dx, al

		sub dx, 4		//172
		mov al, seccnt
		out dx, al

		dec dx			//171
		mov al, 0
		out dx, al

		add dx, 6		//177
		mov al, HD_WRITE_COMMAND
		out dx, al
	}

	//waitComplete(gHdBasePort + 7);

	__asm {
		mov esi, buf
		cld
		movzx ecx, seccnt

		_writeoneSector :
		push ecx

			movzx eax, gHdBasePort
			add eax, 7
			push eax
			//push gHdBasePort + 7
			call waitComplete
			add esp, 4
			cmp eax, 0
			jz _writeSectorLBA24

			pop ecx
			jmp _writeSectorLBA24End

			_writeSectorLBA24 :
		mov dx, gHdBasePort		//170
			mov ecx, BYTES_PER_SECTOR / 4

			rep outsd

			pop ecx
			loop _writeoneSector

			_writeSectorLBA24End :
		mov eax, esi
			sub eax, buf

			sti
	}
}


//most 6 bytes sector no
int readSectorLBA48(unsigned int secnoLow, unsigned int secnoHigh, unsigned char seccnt, char* buf, int device) {


	waitFree(gHdBasePort + 7);

	__asm {
		cli

		mov eax, secnoHigh
		rol eax, 16

		rol eax, 8
		mov dx, gHdBasePort
		add dx, 5		//175
		out dx, al		//secno high 8 bit

		dec dx			//174
		rol eax, 8
		out dx, al		//secno low 8 bit

		dec dx			//173
		mov eax, secnoLow
		rol eax, 8
		out dx, al		//secno high

		add dx, 2		//175
		rol eax, 8		//secno high low
		out dx, al

		dec dx			//174
		rol eax, 8		//secno low high
		out dx, al

		dec dx			//173
		rol eax, 8		//secno low
		out dx, al

		add dx, 3		//176
		mov al, byte ptr device
		out dx, al

		sub dx, 5		//171
		mov al, 0
		out dx, al
		out dx, al

		inc dx			//172,first high byte of sector counter,then low byte of counter
		mov al, 0
		out dx, al
		mov al, byte ptr seccnt
		out dx, al

		add dx, 5		//177
		mov al, HD_LBA48READ_COMMAND
		out dx, al
	}

	//waitComplete(gHdBasePort + 7);

	__asm {
		mov edi, buf
		cld
		movzx ecx, seccnt

		_readoneSector :
		push ecx

			movzx eax, gHdBasePort
			add eax, 7
			push eax
			//push gHdBasePort + 7
			call waitComplete
			add esp, 4
			cmp eax, 0
			jz _readSectorLBA48

			pop ecx
			jmp _readSectorLBA48End

			_readSectorLBA48 :

		mov dx, gHdBasePort		//170
			mov ecx, BYTES_PER_SECTOR / 4

			rep insd

			pop ecx
			loop _readoneSector

			_readSectorLBA48End :
		mov eax, edi
			sub eax, buf

			sti
	}
}



int writeSectorLBA48(unsigned int secnoLow, unsigned int secnoHigh, unsigned char seccnt, char* buf, int device) {

	waitFree(gHdBasePort + 7);

	__asm {
		cli

		mov eax, secnoHigh
		rol eax, 16

		rol eax, 8
		mov dx, gHdBasePort
		add dx, 5		//175
		out dx, al		//secno high 8 bit

		dec dx			//174
		rol eax, 8
		out dx, al		//secno low 8 bit

		dec dx			//173
		mov eax, secnoLow
		rol eax, 8
		out dx, al		//secno high

		add dx, 2		//175
		rol eax, 8		//secno high low
		out dx, al

		dec dx			//174
		rol eax, 8		//secno low high
		out dx, al

		dec dx			//173
		rol eax, 8		//secno low
		out dx, al

		add dx, 3		//176
		mov al, byte ptr device
		out dx, al

		sub dx, 5		//171
		mov al, 0
		out dx, al
		out dx, al

		inc dx			//172
		mov al, 0
		out dx, al
		mov al, byte ptr seccnt
		out dx, al

		add dx, 5		//177
		mov al, HD_LBA48WRITE_COMMAND
		out dx, al
	}

	//waitComplete(gHdBasePort + 7);

	__asm {
		mov esi, buf
		cld
		movzx ecx, seccnt

		_writeoneSector :
		push ecx

			//why error? gHdBasePort is word type,push gHdBasePort result in esp = esp -2,not esp = esp - 4
			//push gHdBasePort + 7
			movzx eax, gHdBasePort
			add eax, 7
			push eax
			call waitComplete
			add esp, 4
			cmp eax, 0
			jz _writeSectorLBA48

			pop ecx
			jmp _writeSectorLBA48End

			_writeSectorLBA48 :
		mov dx, gHdBasePort		//170
			mov ecx, BYTES_PER_SECTOR / 4

			rep outsd

			pop ecx
			loop _writeoneSector

			_writeSectorLBA48End :
		mov eax, esi
			sub eax, buf

			sti
	}
}

//bit0:0==memmory address,1== io address
//bit1:size larger than 1MB
//bit2:0 == 32bits address,1 == 64 bits address
//bit4:1== prefetch,0==false
int readSectorLBA24Mimo(unsigned int secno, unsigned char seccnt, char* buf, int device) {
	waitFree(gHdBasePort + 7);
	__asm {
		mov eax, secno
		movzx edx, gHdBasePort
		add dx, 3		//173
		mov[edx], al

		inc dx			//174
		shr eax, 8
		mov[edx], al

		inc dx			//175
		shr eax, 8
		mov[edx], al

		add dx, 1		//176
		shr eax, 8
		and al, 0fh
		or al, byte ptr device
		mov[edx], al

		sub dx, 4		//172
		mov al, seccnt
		mov[edx], al

		add dx, 5		//177
		mov al, HD_READ_COMMAND
		mov[edx], al
	}

	waitComplete(gHdBasePort + 7);

	__asm {
		mov dx, gHdBasePort		//170
		movzx ecx, seccnt
		shl ecx, 8
		mov edi, buf
		cld
		_readPortData :
		mov ax, [edx]
			stosw
			loop _readPortData
			mov eax, edi
			sub eax, buf
	}
}

//most 6 bytes sector no
int readSectorLBA48Mimo(unsigned int secnoLow, unsigned int secnoHigh, unsigned char seccnt, char* buf, int device) {
	waitFree(gHdBasePort + 7);
	__asm {
		mov eax, secnoHigh
		and eax, 0ffffh
		rol eax, 16
		rol eax, 8
		movzx edx, gHdBasePort
		add dx, 5		//175
		mov[edx], al

		dec dx			//174
		rol eax, 8
		mov[edx], al

		dec dx			//173
		mov eax, secnoLow
		rol eax, 8
		mov[edx], al

		add dx, 2		//175
		rol eax, 8
		mov[edx], al

		dec dx			//174
		rol eax, 8
		mov[edx], al

		dec dx			//173
		rol eax, 8
		mov[edx], al

		add dx, 3		//176
		mov al, byte ptr device
		out dx, al

		sub dx, 5		//171
		mov al, 0
		out dx, al

		inc dx			//172
		mov al, byte ptr seccnt
		out dx, al

		add dx, 5		//177
		mov al, HD_READ_COMMAND
		out dx, al
	}

	waitComplete(gHdBasePort + 7);

	__asm {
		mov dx, gHdBasePort
		movzx ecx, seccnt
		shl ecx, 8
		mov edi, buf
		cld
		_readPortData :
		mov ax, [edx]
			stosw
			loop _readPortData
			mov eax, edi
			sub eax, buf
	}
}



int getHarddiskInfo(char* buf) {

	waitFree(gHdBasePort + 7);

	__asm {
		cli

		mov eax, 0

		mov dx, gHdBasePort
		add dx, 3		//173
		out dx, al

		inc dx			//174
		shr eax, 8
		out dx, al

		inc dx			//175
		shr eax, 8
		out dx, al

		inc dx			//176
		shr eax, 8
		and al, 0fh
		or al, byte ptr gMSDev
		out dx, al

		sub dx, 4		//172
		mov al, 0
		out dx, al

		dec dx
		mov al, 0	//dma = 1,pio = 0
		out dx, al

		add dx, 6		//177
		mov al, 0xec
		out dx, al
	}

	//waitComplete(gHdBasePort + 7);

	__asm {
		mov edi, buf
		cld
		mov ecx, 1

		movzx eax, gHdBasePort
		add eax, 7
		push eax
		call waitComplete
		add esp, 4
		cmp eax, 0
		jz _readSectorLBA24

		jmp _readSectorLBA24End

		_readSectorLBA24 :
		mov dx, gHdBasePort		//170
			mov ecx, BYTES_PER_SECTOR / 4

			rep insd

			_readSectorLBA24End :
		mov eax, edi
			sub eax, buf

			sti
	}

	char szout[1024];
	__printf(szout, "harddisk sequence:%s,firmware version:%s,type:%s,type sequence:%s\r\n",
		(char*)HARDDISK_INFO_BASE + 20, (char*)HARDDISK_INFO_BASE + 46, (char*)HARDDISK_INFO_BASE + 54, (char*)HARDDISK_INFO_BASE + 176 * 2);

}




//驱动器读取一个扇区后，自动设置状态寄存器1F7H的DRQ数据请求位，并清除BSY位忙信号。 
//DRQ位通知主机现在可以从缓冲区中读取512字节或更多的数据，同时向主机发INTRQ中断请求信号
void __kDriverIntProc() {


}


//most 6 bytes sector no
int readDmaSectorLBA48(unsigned int secnoLow, unsigned int secnoHigh, unsigned char seccnt, char* buf, int device) {


	waitFree(gHdBasePort + 7);

	__asm {
		cli

		mov eax, secnoHigh
		rol eax, 16

		rol eax, 8
		mov dx, gHdBasePort
		add dx, 5		//175
		out dx, al		//secno high 8 bit

		dec dx			//174
		rol eax, 8
		out dx, al		//secno low 8 bit

		dec dx			//173
		mov eax, secnoLow
		rol eax, 8
		out dx, al		//secno high

		add dx, 2		//175
		rol eax, 8		//secno high low
		out dx, al

		dec dx			//174
		rol eax, 8		//secno low high
		out dx, al

		dec dx			//173
		rol eax, 8		//secno low
		out dx, al

		add dx, 3		//176
		mov al, byte ptr device
		out dx, al

		sub dx, 5		//171
		mov al, 0
		out dx, al
		out dx, al

		inc dx			//172,first high byte of sector counter,then low byte of counter
		mov al, 0
		out dx, al
		mov al, byte ptr seccnt
		out dx, al

		add dx, 5		//177
		mov al, HD_DMAREAD_COMMAND
		out dx, al
	}

	//waitComplete(gHdBasePort + 7);

	__asm {
		mov edi, buf
		cld
		movzx ecx, seccnt

		_readoneSector :
		push ecx

			movzx eax, gHdBasePort
			add eax, 7
			push eax
			//push gHdBasePort + 7
			call waitComplete
			add esp, 4
			cmp eax, 0
			jz _readSectorLBA48

			pop ecx
			jmp _readSectorLBA48End

			_readSectorLBA48 :

		mov dx, gHdBasePort		//170
			mov ecx, BYTES_PER_SECTOR / 4

			rep insd

			pop ecx
			loop _readoneSector

			_readSectorLBA48End :
		mov eax, edi
			sub eax, buf

			sti
	}
}