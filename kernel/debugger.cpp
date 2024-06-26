#include "debugger.h"
#include "Utils.h"
#include "video.h"


//VME bit0
//虚拟8086模式扩展（CR4中的位0）置1时则在虚拟8086模式下，启用中断和异常处理扩展。置0时禁用扩展功能。
//虚拟模式扩展的应用是通过减少虚拟8086监控程序对8086程序执行过程中出现的中断和异常的处理，并且重定向中断和异常到8086程序的处理程序，
//从而改进虚拟8086模式下应用程序的性能。对于虚拟中断标志（VIF vip）它也提供了硬件支持来改进在多任务及多处理器环境下执行8086程序的可靠性

// Protected - Mode Virtual Interrupts(PVI) Bit.Bit1.
//Setting PVI to 1 enables support for protected -mode virtual interrupts.Clearing PVI to 0 disablesthis support.
//When PVI = 1, hardware support of two bits in the rFLAGS register, VIF and VIP, isenabled.
// Only the STI and CLI instructions are affected by enabling PVI.Unlike the case when CR0.VME = 1,
//the interrupt - redirection bitmap in the TSS cannot be used for selective INTn interception.
// PVI enhancements are also supported in long mode.See“Virtual Interrupts” on page 251 for more
// information on using PVI.


//tsd stamp disable bit

//de debugging extensions bit 



// PSE bit4
// 页尺寸扩展(CR4中的位4)置1时页大小为4M字节，置0时页大小为4K字节

// PAE bit5
// 物理地址扩展(CR4中的位5)置1时启用分页机制来引用36位物理地址；置0时只可引用32位地址。

// MCE bit6
// 启用机器检测(CR4中的位6)置1时启用机器检测(machine - check)异常，置0时禁用机器检测异常

// PGE bit7
// 启用全局页(CR4中的位7)(在P6系列处理器中引入)置1时启用全局页，置0时禁用全局页。
// 全局页这一特征能够使那些经常被使用或共享的页对所有的用户标志为全局的(通过页目录或者页表项中的第8位 - 全局标志来实现)。
// 在任务切换或者往CR3寄存器写时，全局页并不从TLB中刷新。当启用全局页这一特征时，在设置PGE标志之前，
// 必须先启用分页机制(通过设置CR0中的PG标志)。如果将这个顺序颠倒了，可能会影响程序的正确性以及处理器的性能会受损

// PCE bit8
// 启用性能监测计数器(CR4中的位8)置1时，允许RDPMC指令执行，不论程序运行哪个特权级别。置0时RDPMC指令只能运行在0级特权上。

//bit 9
// OSFXSR 操作系统对FXSAVE和FXRSTOR指令的支持(CR4中的位9)置1时，这一标志具有下列
// 功能：(1)表明操作系统支持FXSAVE和FXRSTOR指令
// (2)启用FXSAVE和FXRSTOR指令来保存和恢复XMM和MXCSR寄存器连同x87 FPU和MMX寄存器的内容
// (3)允许处理器执行除了PAUSE、PREFETCHh、SFENCE、LFENCE，MFENCE、MOVNTI和CLFLUSH指令之外的任何SSE和SSE2指令。
// 如果这一标志置0，则FXSAVE和FXRSTOR指令保存和恢复x87 FPU和MMX寄存器的内容，但可能不保存和恢复XMM和MXCSR寄存器的内容。
// 另外，如果这一标志置0，当处理器企图执行除了PAUSE、PREFETCHh、SFENCE、LFENCE、MFENCE、 MOVNTI和CLFLUSH指令之外的任何SSE和SSE2指令时，
// 都将会产生一个非法操作码异常(#UD)，操作系统必须正确地设置这一标志。
// 注意：
// CPUID特征标志FXSR、SSE和SSE2(位24、25、26)分别表示在特定的IA - 32处理器上，是否具有FXSAVE / FXRESTOR指令，SSE扩展以及SSE2扩展。
// OSFXSR位则为操作系统启用这些特征提供了途径以及并指明了操作系统是否支持这些特征。

// OSXMMEXCPT bit10
// 操作系统支持未屏蔽的SIMD浮点异常(CR4中的位10)，表明操作系统通过异常处理程序支持非屏蔽的SIMD浮点异常的处理，
// 该异常处理程序在SIMD浮点异常产生时被调用。
// 操作系统必须正确的设置这一标志，如果这一标志没有设置，当处理器检测到非屏蔽SIMD浮点异常时，将会产生一个非法操作码异常(#UD)

//bit10 OSXMMEXCPT 操作系统支持未屏蔽的SIMD浮点异常

// bit 18 XSAVE and Extended States(OSXSAVE) Bit.Bit18.If this bit is set to 1 then the operating system
// supports the XGETBV, XSETBV, XSAVE and XRSTOR instructions.The processor will also be able
// to execute XGETBV and XSETBV instructions in order toread and write XCR0.Also, if set, the
// XSAVE and XRSTOR instructions can save and restore thex87 FPU state(including MMX registers),
// the SSE state(YMM / XMM registers and MXCSR), alongwith other processor extended states
// enabled in XCR0.


// 从启动到进入长模式
// 设置实模式下的栈，注意启动扇区被bios读取到0x7c00位置
// 利用int13, ah = 02h读取内核到指定位置，注意设备chs的限制
// 关闭中断，开启a20，设置32位gdt
// cr0.pe[0] = 1，进入保护模式
// 长跳转至32位启动代码，16位实模式结束
// 设置32位段，设置保护模式下的栈
// 准备4kbx(2 - 4)大小的空间，存放页表，分别对应1gb、2mb、4kb的页面大小，存放pml4e、pdpe、pde和pte。注意2mb下pde是页表，而1gb下pdpe就是页表了。
// 所有的页表项和目录项都是64位，共512项，正好4kb。其内容就是64位物理地址，而低n位被挪作他用。
// 禁用cr0.pg[31]、cr0.em[2]，启用cr0.mp[1]，禁用分页，并启用sse
// 启用cr4.osfxsr[9]、cr4.osxmmexcpt[10]、cr4.pae[5]、cr4.pge[7]，启用sse，pae和pge
// 将pml4装入cr3
// 启用efer.lme[8]开启长模式
// 启用cr0.pg[31]开启分页，进入64位兼容模式
// 设置64位gdt，进入64位长模式
/*长跳转至64位启动代码，保护模式结束*/


void initDebugger() {
	__asm {
		//mov eax,cr4
		__emit 0x0f
		__emit 0x20
		__emit 0xe0

		or eax, 8

		or eax,100h		//pce enable rdpmc

		//mov cr4,eax
		__emit 0x0f
		__emit 0x22
		__emit 0xe0

		mov eax,0
		mov dr7,eax
		mov dr6,eax
	}
}


void __kBreakPoint( unsigned int * regs) {

	char szout[1024];
	int len = 0;

	unsigned short segDs = 0;
	unsigned short segEs = 0;
	unsigned short segFs = 0;
	unsigned short segGs = 0;
	unsigned short segSs = 0;
	DWORD eflags = 0;
	__asm {
		mov ax, ds
		mov segDs, ax

		mov ax, es
		mov segEs, ax

		mov ax, fs
		mov segFs, ax

		mov ax, gs
		mov segGs, ax

		mov ax, ss
		mov segSs, ax

		//cpu not clear IF when enter interruptions
		pushfd
		pop ss:[eflags]
	}

	if (regs[10] & 0x20000)
	{
		len = __printf(szout,
			"eax:%x,ecx:%x,edx:%x,ebx:%x,kernel esp:%x,ebp:%x,esi:%x,edi:%x,eip:%x,v86 cs:%x,current eflags:%x,eflags:%x,v86 esp:%x,v86 ss:%x,v86 ds:%x,v86 es:%x,v86 fs:%x,v86 gs:%x\n",
			regs[0], regs[1], regs[2], regs[3], regs[4], regs[5], regs[6], regs[7], regs[8], regs[9], eflags, regs[10], regs[11], regs[12],
			regs[14], regs[13], regs[15], regs[16]);
	}
	else if (regs[9] & 3)
	{
		len = __printf(szout,
			"eax:%x,ecx:%x,edx:%x,ebx:%x,kernel esp:%x,ebp:%x,esi:%x,edi:%x,eip:%x,cs:%x,current eflags:%x,eflags:%x,user esp:%x,user ss:%x,ds:%x,es:%x,fs:%x,gs:%x,kernel ss:%x\n",
			regs[0], regs[1], regs[2], regs[3], regs[4], regs[5], regs[6], regs[7], regs[8], regs[9], eflags, regs[10], regs[11], regs[12],
			segDs, segEs, segFs, segGs, segSs);
	}
	else {
		len = __printf(szout,
			"eax:%x,ecx:%x,edx:%x,ebx:%x,esp:%x,ebp:%x,esi:%x,edi:%x,eip:%x,cs:%x,current eflags:%x,eflags:%x,ds:%x,es:%x,fs:%x,gs:%x,kernel ss:%x\n",
			regs[0], regs[1], regs[2], regs[3], regs[4], regs[5], regs[6], regs[7], regs[8], regs[9],eflags, regs[10],
			segDs, segEs, segFs, segGs, segSs);
	}

	__drawGraphChars((unsigned char*)szout, 0);
	return;
}



void __kDebugger(unsigned int * regs) {

	char szout[1024];
	int len = 0;

	DWORD reg_dr6=0;
	__asm {
		mov eax,dr6
		mov [reg_dr6],eax
	}

	if (reg_dr6 & 0x2000)	//BD
	{
		__asm {
			mov eax,dr7
			and eax,0xffffdfff
			mov dr7,eax
		}
		len = __printf(szout, "BD debugger\r\n");

	}
	
	if (reg_dr6 & 0x4000)	//BS
	{
		len = __printf(szout, "BS debugger\r\n");

		__kBreakPoint(regs);
	}
	
	if (reg_dr6 & 0x8000)	//BT
	{
		len = __printf(szout, "BT debugger\r\n");

	}

	DWORD addr = 0;
	DWORD bptype = 0;
	DWORD bplen = 0;
	if (reg_dr6 & 1)
	{
		__asm {
			mov eax, dr0
			mov[addr], eax

			mov eax, dr7
			and eax,0x30000
			mov [bptype],eax

			mov eax,dr7
			and eax,0xc0000
			mov [bplen],eax
		}
	}

	if (reg_dr6 & 2)
	{
		__asm {
			mov eax, dr1
			mov[addr], eax

			mov eax, dr7
			and eax, 0x300000
			mov[bptype], eax

			mov eax, dr7
			and eax, 0xc00000
			mov[bplen], eax
		}
	}

	if (reg_dr6 & 4)
	{
		__asm {
			mov eax, dr2
			mov[addr], eax

			mov eax, dr7
			and eax, 0x3000000
			mov[bptype], eax

			mov eax, dr7
			and eax, 0xc000000
			mov[bplen], eax
		}
	}

	if (reg_dr6 & 8)
	{
		__asm {
			mov eax, dr3
			mov[addr], eax

			mov eax, dr7
			and eax, 0x30000000
			mov[bptype], eax

			mov eax, dr7
			and eax, 0xc0000000
			mov[bplen], eax
		}
	}

	if ( (reg_dr6 & 0x2000) || (reg_dr6 & 0x4000) || (reg_dr6 & 0x8000) )
	{

	}
	
	if((reg_dr6 & 1) || (reg_dr6 & 2) || (reg_dr6 & 4) || (reg_dr6 & 8)){

		len = __printf(szout, "breakpoint address:%x,type:%d,bplen:%d\r\n",addr, bptype,bplen);

	}

	__asm {
		mov eax, 0
		mov[reg_dr6], eax
	}

	DWORD eflags = regs[10];
	if (eflags & 0x10000)
	{
		eflags = eflags & 0xfffeffff;
		regs[10] = eflags;
	}
}

/*
dr7:

bit 0:L0
bit 1:G0

bit 2:L1
bit 3:G1

bit 4:L2
bit 5:G2

bit 6:L3
bit 7:G3

bit 13:GD

bit 17-16: rw/0
bit 19-18:len/0

bit 21-20: rw/1
bit 23-22:len/1

bit 25-24: rw/2
bit 27-26:len/2

bit 29-28: rw/3
bit 31-30:len/3
*/

//rw:
//00: exe
//01: data write
//10: I/O
//11: read write data

//len:
//00: 1 byte
//01: 2 byte
//10: 8 byte
//11: 4 byte

int dataWBreakPoint(unsigned int * addr,int len) {
	
	DWORD reg_dr7 = 0;
	__asm {
		mov eax,dr7
		mov reg_dr7,eax
	}

	DWORD mask = 0;
	DWORD value = 0;
	if ( (reg_dr7 & 1) == 0)
	{
		mask  = 0xf0000;
		value = 0x10001;
		__asm {
			mov eax, addr
			mov dr0, eax
		}
	}else if ((reg_dr7 & 4) == 0)
	{
		mask  = 0xf00000;
		value = 0x100004;
		__asm {
			mov eax, addr
			mov dr1, eax
		}
	}
	else if ((reg_dr7 & 0x10) == 0)
	{
		mask  = 0xf000000;
		value = 0x1000010;
		__asm {
			mov eax, addr
			mov dr2, eax
		}
	}
	else if ((reg_dr7 & 0x40) == 0)
	{
		mask  = 0xf0000000;
		value = 0x10000040;
		__asm {
			mov eax, addr
			mov dr3, eax
		}
	}

	__asm {
		mov eax, dr7
		mov ecx,mask
		not ecx
		and eax,ecx

		or eax,value
		mov dr7, eax
	}
}

int dataRWBreakPoint(unsigned int * addr, int len) {
	DWORD reg_dr7 = 0;
	__asm {
		mov eax, dr7
		mov reg_dr7, eax
	}

	DWORD mask = 0;
	DWORD value = 0;
	if ((reg_dr7 & 1) == 0)
	{
		mask  = 0xf0000;
		value = 0x30001;
		__asm {
			mov eax, addr
			mov dr0, eax
		}
	}
	else if ((reg_dr7 & 4) == 0)
	{
		mask  = 0xf00000;
		value = 0x300004;
		__asm {
			mov eax, addr
			mov dr1, eax
		}
	}
	else if ((reg_dr7 & 0x10) == 0)
	{
		mask  = 0xf000000;
		value = 0x3000010;
		__asm {
			mov eax, addr
			mov dr2, eax
		}
	}
	else if ((reg_dr7 & 0x40) == 0)
	{
		mask  = 0xf0000000;
		value = 0x30000040;
		__asm {
			mov eax, addr
			mov dr3, eax
		}
	}

	__asm {
		mov eax, dr7
		mov ecx, mask
		not ecx
		and eax, ecx
		or eax, value
		mov dr7, eax
	}
}

int ioBreakPoint(unsigned int * addr, int len) {
	DWORD reg_dr7 = 0;
	__asm {
		mov eax, dr7
		mov reg_dr7, eax
	}

	int mask = 0;
	DWORD value = 0;
	if ((reg_dr7 & 1) == 0)
	{
		mask  = 0xf0000;
		value = 0x20001;
		__asm {
			mov eax, addr
			mov dr0, eax
		}
	}
	else if ((reg_dr7 & 4) == 0)
	{
		mask  = 0xf00000;
		value = 0x200004;
		__asm {
			mov eax, addr
			mov dr1, eax
		}
	}
	else if ((reg_dr7 & 0x10) == 0)
	{
		mask  = 0xf000000;
		value = 0x2000010;
		__asm {
			mov eax, addr
			mov dr2, eax
		}
	}
	else if ((reg_dr7 & 0x40) == 0)
	{
		mask  = 0xf0000000;
		value = 0x20000040;
		__asm {
			mov eax, addr
			mov dr3, eax
		}
	}

	__asm {
		mov eax, dr7
		mov ecx, mask
		not ecx
		and eax, ecx
		or eax, value
		mov dr7, eax
	}
}

int codeBreakPoint(unsigned int * addr,int len) {
	DWORD reg_dr7 = 0;
	__asm {
		mov eax, dr7
		mov reg_dr7, eax
	}

	DWORD mask = 0;
	DWORD value = 0;
	if ((reg_dr7 & 1) == 0)
	{
		mask  = 0xf0000;
		value = 0x00001;
		__asm {
			mov eax, addr
			mov dr0, eax
		}
	}
	else if ((reg_dr7 & 4) == 0)
	{
		mask  = 0xf00000;
		value = 0x000004;
		__asm {
			mov eax, addr
			mov dr1, eax
		}
	}
	else if ((reg_dr7 & 0x10) == 0)
	{
		mask  = 0xf000000;
		value = 0x0000010;
		__asm {
			mov eax, addr
			mov dr2, eax
		}
	}
	else if ((reg_dr7 & 0x40) == 0)
	{
		mask  = 0xf0000000;
		value = 0x00000040;
		__asm {
			mov eax, addr
			mov dr3, eax
		}
	}

	__asm {
		mov eax, dr7
		mov ecx, mask
		not ecx
		and eax, ecx
		or eax, value
		mov dr7, eax
	}
}



void __kdBreakPoint() {
	__asm {
		int 3
	}
}

int enterSingleStep() {
	//single step
	__asm {
		pushfd
		bts dword ptr ss : [esp], 8
		popfd
	}
}


int clearSingleStep() {
	//single step
	__asm {
		pushfd
		btr dword ptr ss : [esp], 8
		popfd
	}
}


int enterGdDebugger() {

	__asm {
		mov eax, dr7
		bts eax, 13
		mov dr7, eax
	}
}

int clearGdDebugger() {

	__asm {
		mov eax, dr7
		btr eax, 13
		mov dr7, eax
	}
}

