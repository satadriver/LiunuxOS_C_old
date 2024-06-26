#include "floppy.h"
#include "hardware.h"
#include "video.h"
#include "Utils.h"

// typedef struct {
// 	unsigned int size, sect, head, track, stretch;
// 	unsigned char gap, rate, spec1;
// }floppy_struct;
floppy_struct floppy_type ={ 2880,18,2,80,0,0x1B,0x00,0xCF }; /* 1.44MB diskette */

unsigned int current_dev = 0;



void output_byte(unsigned char byte)
{
	int counter;
	unsigned char status;

	for (counter = 0; counter < 256; counter++) {

		status = inportb(FD_STATUS) & (STATUS_READY | STATUS_DIR);
		if (status == STATUS_READY) {
			outportb(FD_DATA, byte);
			return;
		}
	}

	unsigned char szout[1024];
	__printf((char*)szout,"Unable to send byte to FDC\n");

}



extern "C" __declspec(dllexport) void floppyInit() {

	outportb(FLOPPY_REG_DOR, 0x08); // 重启

	for (int i = 0; i < 0x10; i++)
	{
		__sleep(0);

		__asm {
			nop
		}
	}

	outportb(FLOPPY_REG_DOR, 0xc); // 选择DMA模式，选择软驱A


	outportb(FD_DCR, 0); // 500kpbs


	output_byte(FD_SPECIFY);

	output_byte(0xCF); /* 马达步进速度、磁头卸载时间=32ms */

	output_byte(6); /* Head load time =6ms, DMA */
}



/* (2 * 18 * 80 * 512) */
void FloppyReadSector(unsigned int sectNo,int seccnt, unsigned char *buf)
{
	char szout[1024];

	unsigned int head, track, block, sector, seek_track;

	if (NULL == buf)
	{
		__printf(szout,"FloppyReadSector buf error.\n");
		return;
	}

	if (sectNo >= (floppy_type.head * floppy_type.track * floppy_type.sect))
	{
		__printf(szout,"FloppyReadSector sectNo error: %x.\n", sectNo);
		return;
	}

	/* 计算参数 */
	sector = sectNo % floppy_type.sect + 1;
	block = sectNo / floppy_type.sect;
	track = block / floppy_type.head;
	head = block % floppy_type.head;
	seek_track = track << floppy_type.stretch;

	__printf(szout, "FloppyReadSector sector:%d,block:%d,track:%d,head:%d,seek_track:%d\r\n", 
		sector,block,track,head,seek_track);

	/* 软盘重新校正 */

	output_byte(FD_RECALIBRATE);

	output_byte(current_dev);

	/* 寻找磁道 */
	output_byte(FD_SEEK);

	output_byte(current_dev);

	output_byte(seek_track);

	/* 设置DMA，准备传送数据 */
	SetDMA(buf,seccnt, 0);

	/* 发送读扇区命令 */

	output_byte(FD_READ); /* command */

	output_byte(current_dev); /* driver no. */

	output_byte(track); /* track no. */

	output_byte(head); /* head */

	output_byte(sector); /* start sector */

	//output_byte(2); /* sector size = 512 */
	output_byte(seccnt); /* sector size = 512 */

	output_byte(floppy_type.sect); /* Max sector */

	output_byte(floppy_type.gap); /* sector gap */

	output_byte(0xFF); /* sector size (0xff when n!=0 ?) */
}


/* DMA commands */
#define DMA_READ 0x46
#define DMA_WRITE 0x4A


void SetDMA(unsigned char *buf,int seccnt, unsigned char cmd)
{
	unsigned long addr = (unsigned long)buf;

	int dmareadsize = seccnt * BYTES_PER_SECTOR - 1;

	__asm {
		//cli
	}

	/* mask DMA 2 */
	outportb(10,4 | 2 );

	/* output command byte. I don't know why, but everyone (minix, */
	/* sanches & canton) output this twice, first to 12 then to 11 */

	outportb(12, 0);	//first low,then high address

	if (cmd == 0)
	{
		outportb(11, 0x46);	//single,increase,do not auto reset,dma write,channel 2
	}
	else {
		outportb(11, 0x4a);	//single,increase,do not auto reset,dma read,channel 2
	}
	

	/* 8 low bits of addr */
	outportb(4,(unsigned char)addr);
	addr >>= 8;

	/* bits 8-15 of addr */
	outportb(4, (unsigned char)addr);
	addr >>= 8;

	/* bits 16-19 of addr */
	outportb(0x81, (unsigned char)addr);

	/* low 8 bits of count-1 (1024-1=0x3ff) */
	//outportb(5,0xff);
	outportb(5, (unsigned char)dmareadsize);

	/* high 8 bits of count-1 */
	//outportb(5,3);
	outportb(5, dmareadsize >> 8);

	/* activate DMA 2 */
	outportb(10,0 | 2);

	__asm{
		//sti
	}
}


extern "C" __declspec(dllexport) void __kFloppyIntrProc() {

	unsigned char szshow[4096];
	__drawGraphChars((unsigned char*)"floppy read sector data:\r\n", 0);
	__memset((char*)szshow, 0, 4096);
	__dump((char*)FLOPPY_DMA_BUFFER, BYTES_PER_SECTOR, 0, szshow);
	__drawGraphChars((unsigned char*)szshow, 0);
}


//80 18 2
int getCylinder(int lba) {
	return lba / (SPT * HPC);
}

int getHeader(int lba) {
	return  (lba / SPT) % HPC;
}


int getSector(int lba) {
	return lba % SPT + 1;
}
