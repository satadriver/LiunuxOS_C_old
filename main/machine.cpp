#include "machine.h"
#include "def.h"
#include "video.h"
#include "Utils.h"

//MSR 是CPU 的一组64 位寄存器，可以分别通过RDMSR 和WRMSR 两条指令进行读和写的操作，前提要在ECX 中写入MSR 的地址。
//MSR 的指令必须执行在level 0 或实模式下。
//RDMSR    读模式定义寄存器。对于RDMSR 指令，将会返回相应的MSR 中64bit 信息到(EDX：EAX)寄存器中
//WRMSR    写模式定义寄存器。对于WRMSR 指令，把要写入的信息存入(EDX：EAX)中，执行写指令后，即可将相应的信息存入ECX 指定的MSR 中

//通过DTS获取温度并不是直接得到CPU的实际温度，而是两个温度的差。
//第一个叫做Tjmax，这个Intel叫TCC activation temperature，
//意思是当CPU温度达到或超过这个值时，就会触发相关的温度控制电路，系统此时会采取必要的动作来降低CPU的温度，或者直接重启或关机。
//所以CPU的温度永远不会超过这个值。这个值一般是100℃或85℃（也有其他值），对于具体的处理器来说就是一个固定的值。
//第二个就是DTS获取的CPU温度相对Tjmax的偏移值，暂且叫Toffset，那CPU的实际温度就是：currentTemp=Tjmax-Toffset
int __readTemperature(DWORD * tjunction) {
	unsigned int Tjunction = 0;
	DWORD temp = 0;
	__asm {
		mov eax, 0
		cpuid
		cmp eax, 6
		jb _tmpQuit

		mov eax, 6
		cpuid
		test eax, 2
		jz _tmpQuit

		mov ecx, 0x1A2		//eax中16~23位就是Tjmax的值
		rdmsr
		test eax, 0x40000000
		jnz _tmp85
		//mov eax, 100
		mov Tjunction, eax
		jmp _getdts
		_tmp85 :
		//mov eax, 85
		mov Tjunction, eax

		_getdts :
		mov ebx, eax
		and ebx, 0x00ff0000
		shr ebx, 16

		//mov ebx,eax

		mov ecx, 0x19C		//eax中16~22（注意这里是7位）位就是Toffset的值
		rdmsr
		and eax, 0x007f0000
		shr eax, 16
		sub ebx, eax
		mov eax, ebx
		mov temp,eax
		_tmpQuit :
	}

	*tjunction = Tjunction;

	char szout[1024];
	__printf(szout, "tjmax:%x,temprature:%x\r\n", Tjunction,temp);


	return temp;
}





