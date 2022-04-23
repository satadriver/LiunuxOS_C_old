
#include "hardware.h"
#include "video.h"
#include "rs232.h"
#include "Utils.h"


// char gCom1SendBuf[COM_BUFFER_SIZE];
// char * gCom1SendBufPtr = gCom1SendBuf;
// char * gCom1SendBufEnd = gCom1SendBuf;

char gCom1RecvBuf[COM_BUFFER_SIZE];
char * gCom1RecvBufPtr = gCom1RecvBuf;
char * gCom1RecvBufEnd = gCom1RecvBuf;

// char gCom2SendBuf[COM_BUFFER_SIZE];
// char * gCom2SendBufEnd = gCom2SendBuf;
// char * gCom2SendBufPtr = gCom2SendBuf;

char gCom2RecvBuf[COM_BUFFER_SIZE];
char * gCom2RecvBufPtr = gCom2RecvBuf;
char * gCom2RecvBufEnd = gCom2RecvBuf;

void initRS232Com1() {

	outportb(COM1PORT + 1, 0x00);    // Disable all interrupts

	//brd = 1843200/(16*baud)
	outportb(COM1PORT + 3, 0x80);    // Enable DLAB (set baud rate divisor)
	outportb(COM1PORT + 0, 0x03);    // Set divisor to 3 (lo byte) 38400 baud
	outportb(COM1PORT + 1, 0x00);    //                  (hi byte)

	outportb(COM1PORT + 3, 0x03);    // 8 bits, no parity, one stop bit

	outportb(COM1PORT + 2, 0xC7);    // Enable FIFO, clear them, with 14-byte threshold

	//bit3:modem status change
	//bit2:connection status change
	//bit1:send buf empty
	//bit0:recv buf full
	outportb(COM1PORT + 1, 0x0f);    // enable all interrupts

									 
	outportb(COM1PORT + 4, 0x0B);    // IRQs enabled, RTS/DSR set

	//outportb(COM1PORT + 4, 0x1f);    // loop
}

void initRS232Com2() {

	outportb(COM2PORT + 1, 0x00);    // Disable all interrupts

									 //brd = 1843200/(16*baud)
	outportb(COM2PORT + 3, 0x80);    // Enable DLAB (set baud rate divisor)
	outportb(COM2PORT + 0, 0x03);    // Set divisor to 3 (lo byte) 38400 baud
	outportb(COM2PORT + 1, 0x00);    //                  (hi byte)

	outportb(COM2PORT + 3, 0x03);    // 8 bits, no parity, one stop bit

	outportb(COM2PORT + 2, 0xC7);    // Enable FIFO, clear them, with 14-byte threshold

									 //bit3:modem status change
									 //bit2:connection status change
									 //bit1:send buf empty
									 //bit0:recv buf full
	outportb(COM2PORT + 1, 0x0f);    // enable all interrupts


	outportb(COM1PORT + 4, 0x0B);    // IRQs enabled, RTS/DSR set

	//outportb(COM2PORT + 4, 0x1f);    // loop
}



extern "C"  __declspec(dllexport) void __kCom1Proc() {
	//char szout[1024];

	unsigned char status = inportb(COM1PORT + 2);		
	if (status & 1)
	{
		return;
	}
	else if ((status & 0x0e) == 3)				//优先级最高
	{
		unsigned char c = inportb(COM1PORT + 5);
		__drawGraphChars((unsigned char*)"rs232 receive data error,status register:%x\n", c);
	}else if ((status &0x0e)== 2)			//优先级-1
	{
		//timer0Wait(10);
		unsigned char c = inportb(COM1PORT + 5);
		if (c & 1)
		{
			*gCom1RecvBufPtr = inportb(COM1PORT);
			gCom1RecvBufPtr++;
			if (gCom1RecvBufPtr >= gCom1RecvBuf + COM_BUFFER_SIZE)
			{
				gCom1RecvBufPtr = gCom1RecvBuf;
			}
		}
		else {
			//__drawGraphChars((unsigned char*)"rs232 receive data full,but read error\n", 0);
		}
	}else if ((status & 0x0e) == 6)		//优先级-2
	{
		__drawGraphChars((unsigned char*)"rs232 receive fifo timeout\n", 0);
	}else if ((status & 0x0e) == 1)		//优先级-3
	{
// 		if (gCom1SendBufPtr != gCom1SendBufEnd)
// 		{
// 			unsigned char c = inportb(COM1PORT + 5);
// 			if (c & 0x20)
// 			{
// 				outportb(COM1PORT, *gCom1SendBufPtr);
// 
// 				gCom1SendBufPtr++;
// 				if (gCom1SendBufPtr >= gCom1SendBuf + COM_BUFFER_SIZE)
// 				{
// 					gCom1SendBufPtr = gCom1SendBuf;
// 				}
// 			}
// 			else {
// 				__drawGraphChars((unsigned char*)"rs232 send keep register empty,but send error\n", 0);
// 			}
// 		}
	}else if ((status & 0x0e) == 0)		//优先级-4
	{
		unsigned char c = inportb(COM1PORT + 6);
		__drawGraphChars((unsigned char*)"rs232 modem status:%x change\n", c);
	}
}






int getCom1Data(unsigned char * data) {
	if (gCom1RecvBufEnd == gCom1RecvBufPtr)
	{
		return 0;
	}

	int cnt = 0;
	while (gCom1RecvBufEnd != gCom1RecvBufPtr)
	{
		*(data + cnt)= * gCom1RecvBufEnd;
		gCom1RecvBufEnd++;
		if (gCom1RecvBufEnd >= gCom1RecvBuf + COM_BUFFER_SIZE)
		{
			gCom1RecvBufEnd = gCom1RecvBuf;
		}
		cnt++;
	}
	return cnt;
}


int sendCom1Data(unsigned char *data,int size) {
	int cnt = 0;
	while (cnt < size)
	{
		unsigned char c = inportb(COM1PORT + 5);
		if (c & 0x20)
		{
			outportb(COM1PORT, *(data+cnt));

			cnt++;
		}
	}

	return cnt;
}




extern "C"  __declspec(dllexport) void __kCom2Proc() {
	//char szout[1024];

	unsigned char status = inportb(COM2PORT + 2);
	if (status & 1)
	{
		return;
	}
	else if ((status & 0x0e) == 3)				//优先级最高
	{
		unsigned char c = inportb(COM2PORT + 5);
		__drawGraphChars((unsigned char*)"rs232 receive data error,status register:%x\n", c);
	}
	else if ((status & 0x0e) == 2)			//优先级-1
	{
		//timer0Wait(10);
		unsigned char c = inportb(COM2PORT + 5);
		if (c & 1)
		{
			*gCom2RecvBufPtr = inportb(COM2PORT);
			gCom2RecvBufPtr++;
			if (gCom2RecvBufPtr >= gCom2RecvBuf + COM_BUFFER_SIZE)
			{
				gCom2RecvBufPtr = gCom2RecvBuf;
			}
		}
		else {
			//__drawGraphChars((unsigned char*)"rs232 receive data full,but read error\n", 0);
		}
	}
	else if ((status & 0x0e) == 6)		//优先级-2
	{
		__drawGraphChars((unsigned char*)"rs232 receive fifo timeout\n", 0);
	}
	else if ((status & 0x0e) == 1)		//优先级-3
	{
// 		if (gCom2SendBufPtr != gCom2SendBufEnd)
// 		{
// 			unsigned char c = inportb(COM2PORT + 5);
// 			if (c & 0x20)
// 			{
// 				outportb(COM2PORT, *gCom2SendBufPtr);
// 
// 				gCom2SendBufPtr++;
// 				if (gCom2SendBufPtr >= gCom2SendBuf + COM_BUFFER_SIZE)
// 				{
// 					gCom2SendBufPtr = gCom2SendBuf;
// 				}
// 			}
// 			else {
// 				__drawGraphChars((unsigned char*)"rs232 send keep register empty,but send error\n", 0);
// 			}
// 		}
	}
	else if ((status & 0x0e) == 0)		//优先级-4
	{
		unsigned char c = inportb(COM2PORT + 6);
		__drawGraphChars((unsigned char*)"rs232 modem status:%x change\n", c);
	}
}


int getCom2Data(unsigned char * data) {
	
	if (gCom2RecvBufEnd == gCom2RecvBufPtr)
	{
		return 0;
	}

	int cnt = 0;
	while (gCom2RecvBufEnd != gCom2RecvBufPtr)
	{
		*(data + cnt) = *gCom2RecvBufEnd;
		gCom2RecvBufEnd++;
		if (gCom2RecvBufEnd >= gCom2RecvBuf + COM_BUFFER_SIZE)
		{
			gCom2RecvBufEnd = gCom2RecvBuf;
		}
		cnt++;
	}

	return cnt;
}


int sendCom2Data(unsigned char *data,int size) {
	int cnt = 0;
	while (cnt < size)
	{
		unsigned char c = inportb(COM2PORT + 5);
		if (c & 0x20)
		{
			outportb(COM2PORT, *(data+cnt));

			cnt++;
		}
	}
	return cnt;
}