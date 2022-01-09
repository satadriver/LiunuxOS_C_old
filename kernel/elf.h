#pragma once
#include "def.h"

#define Elf32_Addr    unsigned int
#define Elf32_Half    unsigned short
#define Elf32_Off     unsigned int
#define Elf32_SWord   unsigned int
#define Elf32_Word    unsigned int

#define ET_REL			1	//���ض�λ�ļ�
#define ET_EXEC			2	//��ִ���ļ�
#define ET_DYN			3	//����Ŀ���ļ�

#define SHT_NULL		0	//�ǻ�ģ�û�ж�Ӧ�Ľ�����
#define SHT_PROGBITS	1	//��������Ϣ�����ʽ�ͺ��嶼�ɳ��������͡�
#define SHT_SYMTAB		2	//���ű��������ӱ༭��ָ ld ���ԣ��ķ��ţ�Ҳ������ʵ�ֶ�̬���ӡ�
#define SHT_STRTAB		3	//�ַ�����Ŀ���ļ����ܰ�������ַ����������
#define SHT_RELA		4	//�ض�λ���Elf32_Rela ���͡�
#define SHT_HASH		5	//���Ź�ϣ�����в��붯̬���ӵ�Ŀ�궼�������һ�����Ź�ϣ��һ��Ŀ���ļ�ֻ�ܰ���һ����ϣ��
#define SHT_DYNAMIC		6	//same to PT_DYNAMIC
#define SHT_NOTE		7	//�����Ϣ
#define SHT_NOBITS		8	//�������κ��ֽ�
#define SHT_REL			9	//�ض�λ��Elf32_rel ����
#define SHT_SHLIB		10	//����
#define SHT_DYNSYM		11	//���ű�
#define SHT_INITARRAY	14	//init array
#define SHT_FINALARRAY	15	//final array

#define SHF_WRITE		0x1			//����ִ�е�ʱ��section�ڵ����ݿ�д��
#define SHF_ALLOC		0x2			//����ִ�е�ʱ��section��Ҫռ���ڴ档
#define SHF_EXECINSTR	0x4			//���ڰ�������ִ�еĻ���ָ�
#define SHF_STRINGS		0x20		//����0��β���ַ�����
#define SHF_MASKOS		0x0ff00000	//���maskΪOS�ض������屣��8λ��
#define SHF_MASKPROC	0xf0000000	//���mask����������λ������Ҳ��������ֽڵĸ�4λ����Ϊ��������ص�����ʹ�á�

#define PT_NULL			0
#define PT_LOAD			1
#define PT_DYNAMIC		2		
#define PT_INTERP		3
#define PT_NOTE			4
#define PT_SHLIB		5
#define PT_PHDR			6
#define PT_TLS			7
#define PT_LOOS			0X60000000
#define PT_HIOS			0X6FFFFFFF
#define PT_LOPROC		0X70000000
#define PT_HIPROC		0X7FFFFFFF
#define PT_GNU_STACK	0X6474E551
#define PT_GNU_RELRO	0X6474E552

/*d_tag ��ȡֵ*/
/* Dynamic Array Tags - d_tag */
#define DT_NULL		0		/* marks end of _DYNAMIC array */
#define DT_NEEDED	1		/* string table offset of needed lib */
#define DT_PLTRELSZ	2		/* size of relocation entries in PLT */
#define DT_PLTGOT	3		/* address PLT/GOT */
#define DT_HASH		4		/* address of symbol hash table */
#define DT_STRTAB	5		/* address of string table */
#define DT_SYMTAB	6		/* address of symbol table */
#define DT_RELA		7		/* address of relocation table */
#define DT_RELASZ	8		/* size of relocation table */
#define DT_RELAENT	9		/* size of relocation entry */
#define DT_STRSZ	10		/* size of string table */
#define DT_SYMENT	11		/* size of symbol table entry */
#define DT_INIT		12		/* address of initialization func. */
#define DT_FINI		13		/* address of termination function */
#define DT_SONAME	14		/* string table offset of shared obj */
#define DT_RPATH	15		/* string table offset of library
search path */
#define DT_SYMBOLIC	16		/* start sym search in shared obj. */
#define DT_REL		17		/* address of rel. tbl. w addends */
#define DT_RELSZ	18		/* size of DT_REL relocation table */
#define DT_RELENT	19		/* size of DT_REL relocation entry */
#define DT_PLTREL	20		/* PLT referenced relocation entry */
#define DT_DEBUG	21		/* bugger */
#define DT_TEXTREL	22		/* Allow rel. mod. to unwritable seg */
#define DT_JMPREL	23		/* add. of PLT's relocation entries */
#define DT_BIND_NOW	24		/* Bind now regardless of env setting */
#define DT_NUM		25		/* Number used. */
#define DT_LOPROC	0x70000000	/* reserved range for processor */
#define DT_HIPROC	0x7fffffff	/*  specific dynamic array tags */

#define EI_NIDENT		16

#pragma pack(1)

typedef struct elfhdr {
	unsigned char e_ident[EI_NIDENT];		/* ELF Identification */
	Elf32_Half    e_type;					/* object file type */
	Elf32_Half    e_machine;				/* machine */
	Elf32_Word    e_version;				/* object file version */
	Elf32_Addr    e_entry;					//0x18	/* virtual entry pointָ����Ǳ��������������start�����ǿ����ߴ�����main����*/ 
	Elf32_Off	  e_phoff;					//0x1c	/* program header table offset */
	Elf32_Off     e_shoff;					//0x20	/* section header table offset */
	Elf32_Word    e_flags;					/* processor-specific flags */
	Elf32_Half    e_ehsize;					//0x28	/* ELF header size */
	Elf32_Half    e_phentsize;				//0x2a	/* program header entry size */
	Elf32_Half    e_phnum;					//0x2c	/* number of program header entries */
	Elf32_Half    e_shentsize;				//0x2e	/* section header entry size */
	Elf32_Half    e_shnum;					//0x30	/* number of section header entries */
	Elf32_Half    e_shstrndx;				//0x32	/* section header table's "section header string table" entry offset */
} Elf32_Ehdr;

typedef struct  
{
	unsigned char elfFlag[4];
	unsigned char elfClass;		//1
	unsigned char data;			//1=little endian,2=big endian
	unsigned char version;		//1
	unsigned char os_abi;		//0 = unix
	unsigned char abi_version;
	unsigned char pad[7];
}Elf32_Ident;

typedef struct {
	Elf32_Word    p_type;       /* segment type */
	Elf32_Off     p_offset;		/* segment offset �����ݳ�Ա���������������ļ��е�λ��,�������ݵĿ�ʼλ��������ļ���ͷ��ƫ������*/
	Elf32_Addr    p_vaddr;		/* virtual address of segment */
	Elf32_Addr    p_paddr;		/* physical address - ignored? */
	Elf32_Word    p_filesz;		/* number of bytes in file for seg. */
	Elf32_Word    p_memsz;		/* number of bytes in mem. for seg. */
	Elf32_Word    p_flags;		/* flags */
	Elf32_Word    p_align;		/* memory alignment */
} Elf32_Phdr;

typedef struct {
	Elf32_Word    sh_name;		/* name - index into section header string table section */
	Elf32_Word    sh_type;		/* type */
	Elf32_Word    sh_flags;		/* flags */
	Elf32_Addr    sh_addr;		/* address */
	Elf32_Off     sh_offset;    /* file offset */
	Elf32_Word    sh_size;		/* section size */
	Elf32_Word    sh_link;		/* section header table index link */
	Elf32_Word    sh_info;		/* extra information */
	Elf32_Word    sh_addralign; /* address alignment */
	Elf32_Word    sh_entsize;   /* section entry size */
} Elf32_Shdr;

#define ELF32_R_SYM(i) ((i)>>8)
#define ELF32_R_TYPE(i) ((unsigned char)(i))
#define ELF32_R_INFO(s, t) (((s)<<8) + (unsigned char)(t))

/* Relocation entry with implicit addend */
typedef struct {
	Elf32_Addr    r_offset;		/* offset of relocation */
	Elf32_Word    r_info;        /* symbol table index and type */
	//.rel.dyn �ض�λ����һ��ΪR_386_GLOB_DAT��R_386_COPY
	//.rel.pltΪR_386_JUMP_SLOT
} Elf32_Rel;

/* Relocation entry with explicit addend */
typedef struct {
	Elf32_Addr		r_offset;		/* offset of relocation */
	Elf32_Word		r_info;			/* symbol table index and type */
	Elf32_SWord		r_addend;
} Elf32_Rela;



typedef struct {
	Elf32_Word st_name;			//���ű������ơ������ֵ��0�����ʾ���������ַ���������(offset)��������ű���û�����ơ�
	Elf32_Addr st_value;		//���ڿ�ִ�к�so�ļ�st_value��һ�������ַ��android linker ͨ���� st_value ����ļ����ص��ڴ�Ļ�ַ��ӣ��Ӷ��õ��÷��ŵĶ����ַ��
	Elf32_Word st_size;         //���ŵĳߴ��С������һ�����ݶ���Ĵ�С�Ƕ����а������ֽ�����
	unsigned char st_info;		//���ŵ����ͺͰ����ԡ�
	unsigned char st_other;		//�ó�Ա��ǰ���� 0���京��û�ж��塣
	Elf32_Half st_shndx;        //ÿ�����ű���Ժ����������Ĺ�ϵ�ķ�ʽ�������塣�˳�Ա������صĽ���ͷ����������
} Elf32_sym;


//.got������SHT_PROGBITS
//.plt �������ӱ�Procedure Linkage Table��������SHT_PROGBITS
//.dynamic ָ��Elf32_Dyn
//.dynsymָ��Elf32_sym

//http://www.360doc.com/content/15/1126/20/7377734_516130511.shtml

typedef struct {
	Elf32_SWord d_tag;
	union {
		Elf32_Word d_val;
		Elf32_Addr d_ptr;
	} d_un;
} Elf32_Dyn;



#pragma pack()

int initElf(DWORD file, int filesize, DWORD base);

int mapFile(DWORD file, int filesize,DWORD dst);

int realoc(DWORD elf,DWORD base);

Elf32_Shdr * getShstrtabSection(DWORD file);

Elf32_Shdr * getStrtabSection(DWORD elf);

Elf32_Shdr * getSymtabSection(DWORD file);

Elf32_Shdr * getDynamicSection();

Elf32_Shdr * getDynsymSection();


#ifdef DLL_EXPORT
extern "C"  __declspec(dllexport) int runElfFunction(char * filename,char * funcname);
#else
extern "C"  __declspec(dllimport) int runElfFunction(char * filename, char * funcname);
#endif

Elf32_Shdr * getSectionByType(DWORD file, int type);

Elf32_Shdr * getSectionByName(DWORD file, char * sectionname);

char * checkStrExist(DWORD file, char * sectionname);

DWORD getSymAddrByName(DWORD file, char * symname,DWORD base);
DWORD getSymAddrByIdx(DWORD file,int idx, DWORD base);

DWORD getFiniSection(DWORD file, DWORD base);
DWORD getInitSection(DWORD file, DWORD base);
DWORD getFiniArraySection(DWORD file, DWORD base);
DWORD getInitArraySection(DWORD file, DWORD base);

DWORD getInit(DWORD file, DWORD base);
DWORD getFini(DWORD file, DWORD base);

void showAllSegs(DWORD file);
