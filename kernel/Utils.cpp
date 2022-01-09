
#include "Utils.h"
#include "video.h"
#include "cmosAlarm.h"

int __memset(char * dst, int value, int len) {
	__asm {
		mov ecx,len

		mov edi,dst

		mov al,byte ptr value

		cld

		rep stosb
		mov eax,len
	}
}

int __memset4(char * dst, int value, int len) {
	__asm {
		push ecx
		push edx
		push ebx
		push esi
		push edi

		mov ecx, len
		shr ecx,2

		mov edi, dst

		mov eax,value

		cld

		rep stosd

		lea esi,value
		mov ecx,len
		and ecx,3
		rep movsb

		pop edi
		pop esi
		pop ebx
		pop edx
		pop ecx
		mov eax, len
	}
}

int __strlen(char * s) {

		__asm {
			push esi
			mov esi, s
			cld
			_checkZero :
			lodsb
				cmp al, 0
				jnz _checkZero
				dec esi
				sub esi, s
				mov eax, esi
				pop esi
		}
}


int unicode2asc(short * unicode, int unicodelen, char * asc) {
	int i = 0;
	int j = 0;
	for (; i < unicodelen; )
	{
		asc[i] = (char)unicode[j];
		j ++;
		i++;
	}

	*(asc + i) = 0;
	return unicodelen;
}



int asc2unicode(char * asc, int asclen, short * unicode) {
	int i = 0;
	int j = 0;
	for (; i < asclen; )
	{
		unicode[j] = asc[i];
		j ++;
		i++;
	}

	*(WORD*)(unicode + j) = 0;
	return asclen;
}

int __memcpy(char * dst, char * src,int len) {
	__asm {
		mov esi,src

		mov edi,dst

		mov ecx,len

		cld

		rep movsb
		mov eax,len
	}
}


int __strcpy(char * dst, char * src) {
	__asm {
		mov edi,dst
		mov esi,src
		cld
		_copyBytes:
		lodsb
		stosb
		cmp al, 0
		jnz _copyBytes

		dec edi
		sub edi,dst
		mov eax,edi
	}
}


int __strncpy(char * dst, char * src,int limit) {
	if (limit <= 0)
	{
		return FALSE;
	}

	int l = __strlen(src);
	if (l > limit)
	{
		l = limit;
		__memcpy(dst, src, l);
		*(dst + l) = 0;
		return l;
	}
	else {
		
		__asm {
			mov edi, dst
			mov esi, src
			cld
			_copyBytes :
			lodsb
				stosb
				cmp al, 0
				jnz _copyBytes

				dec edi
				sub edi, dst
				mov eax, edi
		}
	}
}


char* __strstr(char * src, char * dst) {

	int dstlen = __strlen(dst);
	int srclen = __strlen(src);
	if (dstlen > srclen)
	{
		return 0;
	}

	__asm {
		mov esi,src

		mov ecx, srclen
		sub ecx,dstlen
		inc ecx
		cld

		_checkStr:
		mov edx,ecx

		push esi

		mov edi, dst

		mov ecx, dstlen
		repz cmpsb

		pop esi

		jz _strstrFound

		mov ecx,edx

		inc esi
		loop _checkStr

		mov eax,0
		jmp __strstrEnd

		_strstrFound:
		mov eax,esi

		__strstrEnd:
	}
}



int __memcmp(char * src, char * dst,int len) {

	__asm {
		mov esi, src
		mov edi, dst
		mov ecx, len
		cld
		repz cmpsb
		jz _strEqual
		mov eax, -1
		jmp _memcmpEnd
		_strEqual :
		mov eax, 0
		_memcmpEnd :
		//不要在嵌入汇编中调用ret，会打乱c编译器的编译
		//ret
	}
}


int __strcmp(char * src, char * dst) {
	int srclen = __strlen(src);
	int dstlen = __strlen(dst);
	if (srclen != dstlen)
	{
		return -1;
	}

	__asm {
		mov esi, src
		mov edi, dst
		mov ecx, srclen
		repz cmpsb
		jz _strEqual	//do not use jecxz,if length equal and first n-1 is equal but last char is not equal,then ecx is 0,error
		mov eax,-1
		jmp _strcmpEnd
		_strEqual:
		mov eax,0
		_strcmpEnd :
		//不要在嵌入汇编中调用ret，会打乱c编译器的编译
		//ret
	}
}



int __strcat(char * src, char * dst) {
	int srclen = __strlen(src);

	int dstlen = __strlen(dst);

	__memcpy(src + srclen, dst, dstlen);

	*(src + srclen + dstlen) = 0;

	return __strlen(src);
}







int __substrLen(char * src, int pos, int len,char * dst) {
	__memcpy(dst, src + pos,len);
	*(dst + len) = 0;
	return len;
}

int __substr(char * src, int pos,char * dst) {
	return __strcpy(dst, src + pos);
}

int __dump(char * src,int len,int lowercase, unsigned char * dstbuf) {
	if (len >= 0x1000)
	{
		*dstbuf = 0;
		return FALSE;
	}

	int no = 55;
	if (lowercase)
	{
		no = 87;
	}

	
	int lineno = 0;
	unsigned char *dst = dstbuf;

	char szlineno[16];
	__memset(szlineno, 0x20, 16);
	int lnl =__printf(szlineno, "%d.", lineno);
	//*(szlineno + lnl) = 0x20;
	//__strcpy((char*)dst, szlineno);
	//dst += lnl;
	for (int k = 0;k < 16; k ++)
	{
		if (szlineno[k] == 0)
		{
			szlineno[k] = 0x20;
		}
	}
	__memcpy((char*)dst, szlineno, 8);
	dst += 8;
	

	for (int i = 0; i < len; i++)
	{
		if ((i != 0) && (i % 16 == 0))
		{
			*dst = '\n';
			dst++;
			lineno++;

			__memset(szlineno, 0x20, 16);
			lnl = __printf(szlineno, "%d.", lineno);
			//*(szlineno + lnl) = 0x20;
			//__strcpy((char*)dst, szlineno);
			//dst += lnl;
			for (int k = 0; k < 16; k++)
			{
				if (szlineno[k] == 0)
				{
					szlineno[k] = 0x20;
				}
			}
			__memcpy((char*)dst, szlineno, 8);
			dst += 8;
		}
		else if ((i != 0) && (i % 8 == 0))
		{
			*dst = ' ';
			dst++;
			*dst = ' ';
			dst++;
		}

		unsigned char c = src[i] ;
		unsigned char h = (c & 0x0f0) >> 4;
		unsigned char l = c & 0x0f;
		if (h >= 0 && h <= 9)
		{
			h += 48;
		}
		else if (h >= 10 && h <= 15)
		{
			h += no;
		}

		*dst = h;
		dst++;

		if (l >= 0 && l <= 9)
		{
			l += 48;
		}
		else if (l >= 10 && l <= 15)
		{
			l += no;
		}

		*dst = l;
		dst++;

		*dst = ' ';
		dst++;
	}

	*dst = '\n';
	dst++;

	*dst = 0;
	dst++;

	return dst - dstbuf;
}


int __h2strh(unsigned int n,int lowercase,unsigned char * dst) {
	int no = 55;
	if (lowercase)
	{
		no = 87;
	}

	int b = 24;

	for (int i = 0; i < 4; i ++)
	{
		unsigned char c = n >> b;
		unsigned char h = (c & 0x0f0) >> 4;
		unsigned char l = c & 0x0f;
		if (h >= 0 && h <= 9)
		{
			h += 48;
		}else if (h >= 10 && h <= 15)
		{
			h += no;
		}

		*dst = h;
		dst++;

		if (l >= 0 && l <= 9)
		{
			l += 48;
		}
		else if (l >= 10 && l <= 15)
		{
			l += no;
		}

		*dst = l;
		dst++;

		b -= 8;
	}

	return TRUE;
}

int __h2strd(unsigned int h, char * strd) {

	__memset(strd, 0, 11);

	unsigned int divid = 1000000000;

	int flag = FALSE;

	int cnt = 0;

	for (int i = 0; i < 10; i++)
	{

		unsigned int d = h / divid;

		if (d)
		{
			*strd = d + 0x30;

			strd++;

			cnt++;

			h = h % divid;;

			flag = TRUE;
		}
		else if (flag) {
			*strd = 0x30;
			strd++;
			cnt++;
		}

		divid = divid / 10;
	}

	if (cnt == 0)
	{
		*strd = 0x30;
		return 1;
	}
	return cnt;
}

int __hstr2i(unsigned char * str) {
	int ret = 0;

	int len = __strlen((char*)str);
	for (int i = 0; i < len ; i++)
	{
		ret = ret << 4;

		unsigned char c = str[i];
		if (c >= '0' && c <= '9')
		{
			c = c - 48;
		}else if (c >= 'A' && c <= 'F')
		{
			c = c - 55;
		}else if (c >= 'a' && c <= 'f')
		{
			c = c - 87;
		}
		else {
			return 0;
		}
		
		ret += c;
	}

	return ret;
}


int __strd2i(char * istr) {
	int len = __strlen(istr);
	if (len <= 0)
	{
		return 0;
	}

	int negtive = 0;
	int k = 0;
	if (istr[0] == '-')
	{
		negtive = 1;
		k++;
	}else if (istr[0] == '+')
	{
		k++;
	}

	int ret = 0;
	for (; k < len; k ++)
	{
		int v = istr[k] - 0x30;
		if (v >= 0 && v <= 9)
		{
			ret = ret * 10 + v;
		}
		else {
			break;
		}
	}

	if (negtive)
	{
		ret = -ret;
	}
	return ret;
}






int __sprintf(char * buf, char * format, ...) {
	int seq = 0;
	int len = 0;

	__asm {
		lea eax, format
		add eax, 4
		mov seq, eax

		mov esi, format
		mov edi, buf
		cld
		_printfGetBytes :
		lodsb
		cmp al, 0
		jz _printfEnd
		cmp al, '%'
		jz _loadNextByte
		stosb
		jmp _printfGetBytes

		_loadNextByte :
		cmp byte ptr[esi], 'd'
		jz _printfInt
		cmp byte ptr[esi], 'x'
		jz _printfHex
		cmp byte ptr[esi], 'p'
		jz _printfHex
		cmp byte ptr[esi], 'u'
		jz _printfInt
		cmp byte ptr[esi], 's'
		jz _printfstr
		cmp dword ptr[esi], 0x64343649		//I64d
		jz _printfInt64
			cmp dword ptr[esi], 0x75343649		//I64u
			jz _printfInt64
		stosb
		jmp _printfGetBytes

		_printfInt :
		inc esi
		push edi
		mov eax, seq
		push[eax]
		add eax, 4
		mov seq, eax
		call __h2strd
		add esp, 8
		add edi,eax
		jmp _printfGetBytes

		_printfHex :
		mov ax, 7830h
		stosw
		inc esi
		push edi
		push 0
		mov eax, seq
		push[eax]
		add eax, 4
		mov seq, eax
		call __h2strh
		add esp, 12
		add edi, 8
		jmp _printfGetBytes

		_printfstr :
		inc esi
		mov eax, seq
		push[eax]
		add eax, 4
		mov seq, eax
		push edi
		call __strcpy
		add esp, 8
		add edi, eax
		jmp _printfGetBytes

		_printfInt64 :
		add esi, 4
			mov eax,edi
			add eax,8
			push eax
			//push edi+8
			push 0
			mov eax, seq
			push[eax]
			add eax, 4
			mov seq, eax
			call __h2strh
			add esp, 12
			//add edi, 8

			push edi
			push 0
			mov eax, seq
			push[eax]
			add eax, 4
			mov seq, eax
			call __h2strh
			add esp, 12

			add edi, 16
			jmp _printfGetBytes

			_printfEnd :
			stosb
			mov eax, edi
			sub eax, buf
			dec eax
			mov len,eax
	}

	int showlen = __drawGraphChars((unsigned char*)buf, 0);
	return len;
}


int __printf(char * buf, char * format,...) {
	int seq = 0;
	 
	int len = 0;
	__asm {
		lea eax, format
		add eax,4	//size of pointer is 4
		mov seq,eax	//seq is address of params after format in esp

		mov esi,format
		mov edi,buf
		cld

		_printfGetBytes:
		lodsb
		cmp al,0
		jz _printfEnd
		cmp al,'%'
		jz _loadNextByte
		stosb
		jmp _printfGetBytes

		_loadNextByte:
		cmp byte ptr [esi], 'd'
		jz _printfInt
		cmp byte ptr [esi], 'x'
		jz _printfHex
		cmp byte ptr [esi], 'p'
		jz _printfHex
		cmp byte ptr [esi], 'u'
		jz _printfInt
		cmp byte ptr [esi], 's'
		jz _printfstr
		cmp dword ptr [esi], 0x64343649		//I64d
		jz _printfInt64
			cmp dword ptr[esi], 0x75343649		//I64u
			jz _printfInt64
		stosb
		jmp _printfGetBytes


		_printfHex :
		mov ax,7830h
		stosw
		inc esi
		push edi
		push 0	//lowercase is 0,else uppercase
		mov eax, seq
		push[eax]
		add eax, 4
		mov seq, eax
		call __h2strh
		add esp, 12
		add edi, 8
		jmp _printfGetBytes

		_printfInt :
		inc esi
		push edi
		mov eax, seq
		push[eax]
		add eax, 4
		mov seq, eax
		call __h2strd
		add esp,8
		add edi, eax
		jmp _printfGetBytes

		_printfstr:
		inc esi
		mov eax, seq
		push[eax]
		add eax, 4
		mov seq, eax
		push edi
		call __strcpy
		add esp,8
		add edi,eax
		jmp _printfGetBytes

		//__int64高位4字节在前，低位4字节在后一个参数中
		_printfInt64:
		add esi,4
			mov eax, edi
			add eax, 8
			push eax
		//push edi
		push 0
		mov eax, seq
		push[eax]
		add eax, 4
		mov seq, eax
		call __h2strh
		add esp, 12
		//add edi, 8

		push edi
		push 0
		mov eax, seq
		push[eax]
		add eax, 4
		mov seq, eax
		call __h2strh
		add esp, 12

		//add edi, 8
		add edi, 16
		jmp _printfGetBytes

		_printfEnd:
		stosb
		mov eax,edi
		sub eax, buf
		dec eax
		mov len,eax
	}

	return len;
}



unsigned int random() {
	__asm {
		mov al, 36h
		out 0x43, al
		in al, 40h
		mov ah,al
		in al, 0x40
		xchg ah,al
		movzx eax,ax
	}
}



int getCpuType(char * name) {
	__asm {
		mov edi,name
		mov eax, 0
		mov [edi],eax

		; must use .586 or above
		cpuid
		; ebx:edx:ecx = intel or else
		mov ds : [edi], ebx
		mov ds : [edi + 4], edx
		mov ds : [edi + 8], ecx
		mov dword ptr ds : [edi + 12], 0
	}

// 	__asm {
// 		mov edi,name
// 		mov eax,9
// 		int 80h
// 	}

	return 0;
}

int getCpuInfo(char * name) {

	__asm {
		mov edi,name
		mov dword ptr [edi],0

		mov     eax, 80000000h
		cpuid
		cmp     eax, 80000004h
		jb      __cpuinfoEnd

		mov     eax, 80000002h
		cpuid
		mov     dword ptr[edi], eax
		mov     dword ptr[edi + 4], ebx
		mov     dword ptr[edi + 8], ecx
		mov     dword ptr[edi + 12], edx

		mov     eax, 80000003h
		cpuid
		mov     dword ptr[edi + 16], eax
		mov     dword ptr[edi + 20], ebx
		mov     dword ptr[edi + 24], ecx
		mov     dword ptr[edi + 28], edx

		mov     eax, 80000004h
		cpuid
		mov     dword ptr[edi + 32], eax
		mov     dword ptr[edi + 36], ebx
		mov     dword ptr[edi + 40], ecx
		mov     dword ptr[edi + 44], edx

		mov     dword ptr[edi + 48], 0

		__cpuinfoEnd:
	}

// 	__asm {
// 		mov edi, name
// 		mov eax, 12
// 		int 80h
// 	}

	return 0;
}

int timer0Wait(unsigned short millisecs) {
	unsigned short delay = millisecs / 10;

	//unsigned short timer0 = 0;
	//unsigned short timer1 = 0;
	__asm {
		mov al, 36h
		out 0x43, al
		in al, 40h
		mov cl, al
		in al, 0x40
		mov ch, al
		//mov timer0, cx

		_waitTimer0Count :
		mov al, 36h
		out 0x43, al
		in al, 40h
		mov dl, al
		in al, 0x40
		mov dh, al
		//mov timer1, dx

		sub dx,cx
		cmp dx,delay
		jb _waitTimer0Count
	}
}


int __shutdownSystem() {

	__asm {
		push edx

		mov dx, 0cf8h
		mov eax, 8000f840h
		out dx, eax

		mov dx, 0cfch
		in eax, dx
		cmp eax, 0ffffffffh
		jz _notSupportICH

		and ax, 0fffeh
		add ax, 4
		mov dx, ax
		in ax, dx
		or ax, 03c00h
		out dx, ax

		_notSupportICH :
		pop edx


// 		mov dx, 0cf8h
// 		mov eax, 8000f840h
// 		out dx, eax
// 
// 		mov dx, 0cfch
// 		in eax, dx
// 		and al, 0feh
// 		mov dx, ax
// 
// 		push dx
// 
// 		add dx, 30h
// 		in ax, dx
// 		and ax, 0ffefh
// 		out dx, ax
// 
// 		pop dx
// 		add dx, 5
// 		in al, dx
// 		or al, 3ch
// 		out dx, al
	}
}


int __reset() {
	__asm {
// 		mov al, 4
// 		mov dx, 0cf9h
// 		out dx, al

		mov al,1
		out 92h,al
	}
}




int __sleep(int millisecs) {
	__asm {
		mov eax, 6
		lea edi, millisecs
		int 80h
	}
}

int __strlwr(char * str) {
	int len = __strlen(str);
	for (int i = 0; i < len; i++)
	{
		if (str[i] >= 'A' && str[i] <= 'Z')
		{
			str[i] += 0x20;
		}
	}
	return len;
}

int __strupr(char * str) {
	int len = __strlen(str);
	for (int i = 0; i < len; i++)
	{
		if (str[i] >= 'a' && str[i] <= 'z')
		{
			str[i] -= 0x20;
		}
	}
	return len;
}


int lower2upper(char *data,int len) {
	int k = 0;
	for (int i = 0;i < len; i ++)
	{
		if (data[i] >= 'a' && data[i] <= 'z')
		{
			data[i] -= 0x20;
			k++;
		}
	}

	return k;
}

int upper2lower(char *data, int len) {
	int k = 0;
	for (int i = 0; i < len; i++)
	{
		if (data[i] >= 'A' && data[i] <= 'Z')
		{
			data[i] += 0x20;
			k++;
		}
	}

	return k;
}

//1 + 3 + 5 + ... + (2n - 1) = (1 + (2n - 1))*(n / 2) = n ^ 2
DWORD __sqrtInteger(DWORD i) {
	DWORD root = 0;
	__asm {
		MOV eax, i
		MOV EBX, 1
		MOV ECX, 1
		_S_LOOP:
		SUB EAX, EBX
			JC _END		; 有借位为止
			INC EBX		; 修改为3、5、7...
			INC EBX
			INC ECX		; n加1
			JMP _S_LOOP
			_END :
			MOV root,ECX
	}
	return root;
}

//bswap oprd1,oprd1:reg
DWORD __ntohl(DWORD v) {

	DWORD result = v;

	__asm {
		mov eax, result
		bswap eax
		mov result,eax
	}

	return result;
}

WORD __ntohs(WORD v) {

	WORD result = 0;

	__asm {
		mov ax, v
		xchg ah,al
		mov result, ax
	}

	return result;
}


//xadd oprd1,oprd2,先将两个数交换，再将二者之和送给第一个数,oprd1:mem or reg,oprd2:reg
DWORD __inc(DWORD *v) {
	DWORD old = 0;
	__asm {
		mov eax, 1
		lock xadd [v],eax
		mov old,eax
	}
	return old;
}

void __initSpinLock(DWORD * v) {
	*v = 0;
}

WORD __enterSpinLock(DWORD * v) {
	__asm {
__getlock:
			lock bts[v], 0
			jnc __enterSpinLockEnd
			pause
			jmp __getlock
			__enterSpinLockEnd :
	}
}

WORD __leaveSpinLock(DWORD * v) {
	__asm {
		//lock and[v], 0
		lock btr[v], 0
		jc __leaveSpinLockEnd
		__leaveSpinLockEnd:
	}
}

//cmpxchg oprd1,oprd2,oprd1:mem or reg,oprd2:reg
//CMPXCHG r/m,r
//CMPXCHG r / m, r 将累加器AL / AX / EAX / RAX中的值与首操作数（目的操作数）比较，如果相等，第2操作数（源操作数）的值装载到首操作数，zf置1。
//如果不等， 首操作数的值装载到AL / AX / EAX / RAX并将zf清0
DWORD __enterlock(DWORD * lockvalue) {
	DWORD result = 0;
	while (result == 0)
	{
		__asm {
			mov eax, 1
			lock cmpxchg[lockvalue], eax
			jz _Busy
			mov result, 1
			jmp _over
			_Busy :
			mov result, 0
			_over :
		}
	}

	return result;
}


DWORD __leavelock(DWORD * lockvalue) {
	DWORD result = 0;
	while (result == 0)
	{
		__asm {
			mov eax, 1
			lock cmpxchg[lockvalue], eax
			jz _Busy
			mov result, 1
			jmp _over
			_Busy :
			mov result, 0
				_over :
		}
	}

	return result;
}

extern "C"  __declspec(dllexport) int __getDateTime(LPDATETIME datetime)
{
	char c = readCmosPort(0x32);
	char y = readCmosPort(9);
	char m = readCmosPort(8);
	char d = readCmosPort(7);
	char hour = readCmosPort(4);
	char minute = readCmosPort(2);
	char second = readCmosPort(0);

	datetime->year = ( bcd2b(c) *100) + bcd2b(y);
	datetime->month = bcd2b(m);
	datetime->dayInMonth = bcd2b(d);
	datetime->hour = bcd2b(hour);
	datetime->minute = bcd2b(minute);
	datetime->second = bcd2b(second);
	return TRUE;
}


extern "C"  __declspec(dllexport) int __getDateTimeStr(void * str) {
	return __strcpy((char*)str,(char*) CMOS_DATETIME_STRING);
}