#include "../def.h"
#include "sbPlay.h"
#include "../Utils.h"
#include "../file.h"

//https://blog.csdn.net/weixin_33755847/article/details/93795780
//http://homepages.cae.wisc.edu/~brodskye/sb16doc/sb16doc.html#ResetDSP

int gWavDataSize = 0;
char * gWavDataPtr = 0;
WAVEFORMAT * gWavFormat = 0;
int gSoundBlast = 0;



int writedsp(int value) {
	__asm {
		push edx

		_writedspWait :
		mov dx,SOUNDBLASTER_BASE_PORT
		add dx,0x0c
		
		in al,dx
		test al,80h
		jnz _writedspWait

		mov al,byte ptr value
		out dx,al
		pop edx
	}
}

int readdsp() {
	__asm {
		push edx
		_readdspWait :
		mov dx, SOUNDBLASTER_BASE_PORT
		add dx,0x0e
		
		in al,dx
		test al,80h
		jz _readdspWait

		mov dx,SOUNDBLASTER_BASE_PORT
		add dx,0ah
		in al,dx
		movzx eax,al
		pop edx
	}
}

int initdsp() {
	__asm {
		push edx
		mov dx, SOUNDBLASTER_BASE_PORT
		add dx,6
		mov al,1
		out dx,al

		mov al,0
		out dx,al
		pop edx
	}

	int ret = readdsp();
	if (ret == 0xaa)
	{
		return TRUE;
	}
	else {
		return FALSE;
	}
}


//Write the DMA mode for the transfer 
//The mode selection bits should be set to 01 for single-mode. 
//The address inc/dec bit should be set to 0 for address increment. 
//The auto-initialization bit should be set appropriately. I will discuss auto-initialized DMA later. 
//The transfer bits should be set to 10 for playback and 01 for recording. 
//The channel select should be set to the sound card DMA channel. 

//Some often used modes are: 
//48h+Channel - Single-cycle playback 
//58h+Channel - Auto-initialized playback 
//44h+Channel - Single-cycle record 
//54h+Channel - Auto-initialized recording 
//Be aware that "read" means a read from memory (Write to sound card) and that "write" means a write to system memory. 

/*
if SixteenBit
then
begin
BufOffset := (LinearAddr div 2) mod 65536;
Port[BaseAddrPort] := Lo(BufOffset);
Port[BaseAddrPort] := Hi(BufOffset);
end
else
begin
BufOffset := LinearAddr mod 65536;
Port[BaseAddrPort] := Lo(BufOffset);
Port[BaseAddrPort] := Hi(BufOffset);
end;
*/
int initdma8(int addr,int firstsize,int bits) {
	unsigned char dmaseg = addr / 0x10000;

	__asm {
		push edx

		mov dx, 0ah		; DMA���μĴ���
		mov al, 5		; ѡ��ͨ��1
						; ��ͨ��3ʱ��al = 7
						; һ������Ĭ������ͨ��1
		out dx, al		; mask off dma channel

		mov dx, 0ch		; DMACFFREG
		mov al, 0
		out dx, al		; clear flip - flop

		mov dx, 0bh
		mov al, 59h
		out dx, al		; single mode,increase address,auto set,dma read,channel 1

		mov dx, 2		; dma ƫ��
		mov al, 0		; 8000:0��ƫ����0
		out dx, al		; ��8λ0
		out dx, al		; ��8λ0

		mov dx, 83h		; dma ҳ
		mov al, dmaseg	; 8000:0��ҳ��8
		out dx, al

		mov dx, 3			; dma ����
		mov eax, firstsize	; ���ݿ��С
		dec eax
		out dx, al		; ��8λ
		mov al, ah
		out dx, al		; ���ֽ�

		mov dx, 0ah		; dmac ���μĴ���
		mov al, 1		; ����ͨ��1��������
		out dx, al

		pop edx
	}
}

int initdma16(int addr, int firstsize, int bits) {
	unsigned char dmaseg = addr / 0x10000;

	__asm {
		push edx

		mov dx, 0d4h		; DMA���μĴ���
		mov al, 5			; ѡ��ͨ��1
							; ��ͨ��3ʱ��al = 7
							; һ������Ĭ������ͨ��1
		out dx, al			; mask off dma channel

		mov dx, 0d8h		; DMACFFREG
		mov al, 0
		out dx, al			; clear flip - flop

		mov dx, 0d6h
		mov al, 59h
		out dx, al			; single mode, increase address, auto set, dma read, channel 1

		//BufOffset := (LinearAddr div 2) mod 65536;
		mov dx, 0c4h		; dma ƫ��
		mov al, 0			; 8000:0��ƫ����0
		out dx, al			; ��8λ0
		out dx, al			; ��8λ0

		mov dx, 8bh			; dma ҳ
		mov al, dmaseg		; 8000:0��ҳ��8
		out dx, al

		mov dx, 0c6h			; dma ����
		mov eax, firstsize		; ���ݿ��С
		dec eax
		out dx, al			; ��8λ
		mov al, ah
		out dx, al			; ���ֽ�

		mov dx, 0d4h		; dmac ���μĴ���
		mov al, 1			; ����ͨ��1��������
		out dx, al

		pop edx
	}
}


int dspplay(unsigned short rate,int mode,int firstsize) {
	 
	writedsp(0x41);
	writedsp(rate >> 8);
	writedsp(rate);

// 	writedsp(0x42);
// 	writedsp(rate >> 8);
// 	writedsp(rate);

	if (gWavFormat->wBitsPerSample == 16)
	{
		writedsp(0xb6);
	}else if (gWavFormat->wBitsPerSample == 8)
	{
		writedsp(0xc6);
	}
	
	writedsp(mode);

// 	writedsp((unsigned char)firstsize );
// 	writedsp((unsigned char)(firstsize >> 8));

	writedsp(firstsize - 1);
	writedsp((firstsize - 1) >> 8);

	return 0;
}






void stopdsp() {

	if (gWavFormat->wBitsPerSample == 8)
	{
		writedsp(0xd0);
		writedsp(0xda);
	}else if (gWavFormat->wBitsPerSample == 16)
	{
		writedsp(0xd5);
		writedsp(0xd9);
	}
	
	return;
}


int sbplay(char * filedata, int filesize) {
	WAVFILEHEADER * hdr = (WAVFILEHEADER*)filedata;
	if (__memcmp(hdr->id,"RIFF",4) || __memcmp(hdr->type,"WAVE",4))
	{
		return FALSE;
	}

	gWavFormat = (WAVEFORMAT*)(filedata + sizeof(WAVFILEHEADER));
	if (__memcmp(gWavFormat->id,"fmt ",4) || gWavFormat->wFormatag != 1)
	{
		return FALSE;
	}

	int offset = sizeof(WAVFILEHEADER) + gWavFormat->size + 8;	//gWavFormat->size + 8 == block size
	while (1)
	{
		if (__memcmp(filedata + offset,"data",4) == 0)
		{
			offset += 8;
			break;
		}
		else {
			int datablocksize = *(int*)(filedata + offset + 4);
			offset += (datablocksize + 8);
			if (offset >= filesize || offset <= 0)
			{
				return FALSE;
			}
		}
	}

	int size = filesize - offset;

	gWavDataPtr = filedata + offset;
	gWavDataSize = size;

	int ret = 0;

	ret = initdsp();
	if (ret == FALSE)
	{
		return FALSE;
	}
	else {
		gSoundBlast = TRUE;
	}

	int blocksize = 0;
	if (gWavFormat->wBitsPerSample == 8)
	{
		blocksize = 0x10000;
	}
	else if (gWavFormat->wBitsPerSample == 16)
	{
		blocksize = 0x20000;
	}
	else {
		stopdsp();
		return FALSE;
	}

	int blocks = size / blocksize;
	int mod = size % blocksize;
	if (blocks)
	{
		if (gWavFormat->wBitsPerSample == 8)
		{
			__memcpy((char*)ISA_DMA_BUFFER, (char*)gWavDataPtr, blocksize);
			gWavDataPtr += blocksize;
			gWavDataSize -= blocksize;
			ret = initdma8(ISA_DMA_BUFFER, 0x10000, gWavFormat->wBitsPerSample);
		}
		else if (gWavFormat->wBitsPerSample == 16)
		{
			__memcpy((char*)ISA_DMA_BUFFER, (char*)gWavDataPtr, blocksize);
			gWavDataPtr += blocksize;
			gWavDataSize -= blocksize;
			ret = initdma16(ISA_DMA_BUFFER, 0x10000, gWavFormat->wBitsPerSample);
		}
		
		if (gWavFormat->nChannls == 1)
		{
			ret = dspplay(gWavFormat->nSamplesPerSec, FORMAT_MONO | FORMAT_SIGNED, 0x10000);
		}else if (gWavFormat->nChannls == 2)
		{
			ret = dspplay(gWavFormat->nSamplesPerSec, FORMAT_STEREO | FORMAT_SIGNED, 0x10000);
		}
		else {
			stopdsp();
			return FALSE;
		}
	}
	else if(mod){
		__memcpy((char*)ISA_DMA_BUFFER, (char*)gWavDataPtr, mod);
		gWavDataPtr += mod;
		gWavDataSize -= mod;

		if (gWavFormat->wBitsPerSample == 8) {
			ret = initdma8(ISA_DMA_BUFFER, mod, gWavFormat->wBitsPerSample);
		}
		else if (gWavFormat->wBitsPerSample == 16) {
			ret = initdma16(ISA_DMA_BUFFER, mod, gWavFormat->wBitsPerSample);
		}
		
		if (gWavFormat->nChannls == 1)
		{
			ret = dspplay(gWavFormat->nSamplesPerSec, FORMAT_MONO | FORMAT_SIGNED, mod);
		}
		else if (gWavFormat->nChannls == 2)
		{
			ret = dspplay(gWavFormat->nSamplesPerSec, FORMAT_STEREO | FORMAT_SIGNED, mod);
		}
		else {
			stopdsp();
			return FALSE;
		}
	}
	else {
		stopdsp();

		return FALSE;
	}

	return TRUE;
}


void __kSoundInterruptionProc() {

	int ret = 0;
	int blocksize = 0;
	if (gWavFormat->wBitsPerSample == 8)
	{
		blocksize = 0x10000;
	}
	else if (gWavFormat->wBitsPerSample == 16)
	{
		blocksize = 0x20000;
	}
	else {
		stopdsp();
		return;
	}

	if (gWavDataSize >= blocksize)
	{
		__memcpy((char*)ISA_DMA_BUFFER, gWavDataPtr, blocksize);
		gWavDataPtr += blocksize;
		gWavDataSize -= blocksize;

	}
	else if (gWavDataSize > 0)
	{
		__memcpy((char*)ISA_DMA_BUFFER, gWavDataPtr, gWavDataSize);
		gWavDataPtr += gWavDataSize;
		gWavDataSize -= gWavDataSize;
	}
	else {
		stopdsp();
		return;
	}

	if (gWavFormat->wBitsPerSample == 8)
	{
		__asm {
			mov dx, SOUNDBLASTER_BASE_PORT
			add dx, 0eh
			in al, dx
		}
	}
	else if (gWavFormat->wBitsPerSample == 16)
	{
		__asm {
			mov dx, SOUNDBLASTER_BASE_PORT
			add dx, 0fh
			in al, dx
		}
	}
}