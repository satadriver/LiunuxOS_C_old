#pragma once

#define COM1PORT 0x3f8   
#define COM2PORT 0x2f8
#define COM3PORT 0x3e8
#define COM4PORT 0x2e8

#define COM_BUFFER_SIZE 256

void initRS232Com1();

void initRS232Com2();

#ifdef DLL_EXPORT
extern "C"  __declspec(dllexport) void __kCom1Proc();
extern "C"  __declspec(dllexport) void __kCom2Proc();

int getCom1Data(unsigned char * data);
 int sendCom1Data(unsigned char *data,int size);

int getCom2Data(unsigned char * data);
int sendCom2Data(unsigned char *data,int size);
#else
extern "C"  __declspec(dllimport) void __kCom1Proc();
extern "C"  __declspec(dllimport) void __kCom2Proc();

extern "C"  __declspec(dllimport) int getCom1Data(unsigned char * data);
extern "C"  __declspec(dllimport) int sendCom1Data(unsigned char *data, int size);

extern "C"  __declspec(dllimport) int getCom2Data(unsigned char * data);
extern "C"  __declspec(dllimport) int sendCom2Data(unsigned char *data, int size);
#endif