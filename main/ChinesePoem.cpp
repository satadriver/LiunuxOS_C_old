#pragma once

#include "video.h"
#include "Utils.h"

#include "mouse.h"
#include "keyboard.h"
#include "task.h"
#include "file.h"
#include "graph.h"
#include "Pe.h"
#include "gdi/jpeg.h"

#include "window.h"
#include "guiHelper.h"
#include "fileWindow.h"
#include "cmosExactTimer.h"




unsigned char* poem_jjj = (unsigned char*)
"�������ơ�\r\n"
"�ơ����\r\n"
"���������ƺ�֮ˮ���������������������ء�\r\n"
"�������������������׷���������˿ĺ��ѩ��\r\n"
"���������뾡����Īʹ���׿ն��¡�\r\n"
"�����Ҳı����ã�ǧ��ɢ����������\r\n"
"������ţ��Ϊ�֣�����һ�����ٱ���\r\n"
"᯷��ӣ��������������ƣ���Īͣ��\r\n"
"�����һ�������Ϊ���������\r\n"
"�ӹ�������󣬵�Ը�������ѡ�\r\n"
"����ʥ�ͽԼ�į��Ω��������������\r\n"
"������ʱ��ƽ�֣�����ʮǧ���ʡ�\r\n"
"���˺�Ϊ����Ǯ�������ȡ�Ծ��á�\r\n"
"�廨��ǧ���ã��������������ƣ����ͬ����ų\r\n\0";


unsigned char* poem_xln = (unsigned char*)
"����·�ѡ�\r\n"
"�ơ����\r\n"
"������ƶ�ʮǧ����������ֱ��Ǯ��\r\n"
"ͣ��Ͷ�粻��ʳ���ν��Ĺ���ãȻ��\r\n"
"���ɻƺӱ�����������̫��ѩ��ɽ��\r\n"
"����������Ϫ�ϣ������������ձߡ�\r\n"
"��·�ѣ���·�ѣ�����·�����ڣ�\r\n"
"�������˻���ʱ��ֱ���Ʒ��ò׺���\r\n\0";


unsigned char* poem_nnjcbhg = (unsigned char*)
"����ū������ڻ��š�\r\n"
"���Ρ�����\r\n"
"�󽭶�ȥ�����Ծ���ǧ�ŷ������\r\n"
"�������ߣ��˵��ǣ��������ɳ�ڡ�\r\n"
"��ʯ���գ������İ�������ǧ��ѩ��\r\n"
"��ɽ�续��һʱ���ٺ��ܡ�\r\n"
"ң�빫誵��꣬С�ǳ����ˣ�����Ӣ����\r\n"
"�����ڽ�̸Ц�䣬���ֻҷ�����\r\n"
"�ʹ����Σ�����ӦЦ�ң�����������\r\n"
"�������Σ�һ�������¡�\r\n\0";

unsigned char* poem_gjtyqy = (unsigned char*)
"����԰��ӡ���һ��\r\n"
"��������Ԩ��\r\n"
"���������ϣ��Ա�����ɽ��\r\n"
"���䳾���У�һȥ��ʮ�ꡣ\r\n"
"��������֣�����˼��Ԩ��\r\n"
"������Ұ�ʣ���׾��԰�\r\n"
"��լʮ��Ķ�����ݰ˾ż䡣\r\n"
"��������ܣ���������ǰ��\r\n"
"����Զ�˴壬���������̡�\r\n"
"���������У�����ɣ���ߡ�\r\n"
"��ͥ�޳��ӣ����������С�\r\n"
"���ڷ�������÷���Ȼ��\r\n\0";


unsigned char* poem_gldy = (unsigned char*)
"�����㶡��\r\n"
"���Ρ�������\r\n"
"���������һ�����ɸ����������ǡ�\r\n"
"ɽ�������Ʈ���������������Ƽ��\r\n"
"�̿�̲ͷ˵�֣̿��㶡����̾�㶡��\r\n"
"�����Թ�˭��������ȡ�����պ��ࡣ\r\n\0";


unsigned char* poem_mjhxh = (unsigned char*)
"�������졤д����\r\n"
"���Ρ�����\r\n"
"ŭ����ڣ�ƾ������������Ъ��\r\n"
"̧���ۡ����쳤Х��׳�����ҡ�\r\n"
"��ʮ��������������ǧ��·�ƺ��¡�\r\n"
"Ī���С���������ͷ���ձ��С�\r\n"
"�����ܣ���δѩ�����Ӻޣ���ʱ��\r\n"
"�ݳ�����̤�ƺ���ɽȱ��\r\n"
"׳־���ͺ�²�⣬Ц̸������ūѪ��\r\n"
"����ͷ����ʰ��ɽ�ӣ������ڡ�\r\n\0";

unsigned char* poem_dgx = (unsigned char*)
"���̸��С�\r\n"
"�ܲ١���������\r\n"
"�ԾƵ��裬�������Σ�\r\n"
"Ʃ�糯¶��ȥ�տ�ࡣ\r\n"
"�����Կ�����˼������\r\n"
"���Խ��ǣ�Ψ�жſ���\r\n"
"�������ƣ��������ġ�\r\n"
"��Ϊ���ʣ���������\r\n"
"����¹����ʳҰ֮ƻ��\r\n"
"���мα�����ɪ���ϡ�\r\n"
"�������£���ʱ�ɶޣ�\r\n"
"�Ǵ����������ɶϾ���\r\n"
"Խİ���䣬������档\r\n"
"����̸ׅ������ɶ���\r\n"
"������ϡ����ȵ�Ϸɡ�\r\n"
"�������ѣ���֦������\r\n"
"ɽ����ߣ��������\r\n"
"�ܹ��²������¹��ġ�\r\n\0";


unsigned char* poem_gch = (unsigned char*)
"���۲׺���\r\n"
"�ܲ١���������\r\n"
"������ʯ���Թ۲׺���\r\n"
"ˮ���壣�ɽ����š�\r\n"
"��ľ�������ٲݷ�ï��\r\n"
"�����ɪ���鲨ӿ��\r\n"
"����֮�У��������У�\r\n"
"�Ǻ����ã��������\r\n"
"�������գ�����ӽ־��\r\n\0";

unsigned char* poem_qyccs = (unsigned char*)
"����԰������ɳ��\r\n"
"ë��\r\n"
"��������潭��ȥ��������ͷ��\r\n"
"����ɽ��飬���־�Ⱦ��������͸������������\r\n"
"ӥ�����գ�����ǳ�ף�����˪�쾺���ɡ�\r\n"
"���������ʲ�ã��أ�˭��������\r\n"
"Я���������Ρ�������������³�\r\n"
"ǡͬѧ���꣬�绪��ï�������������ӳⷽ�١�\r\n"
"ָ�㽭ɽ���������֣����������򻧺\r\n"
"���Ƿ񣬵�������ˮ���˶����ۣ�\r\n\0";

unsigned char* poem_tjsqs = (unsigned char*)
"���쾻ɳ����˼��\r\n"
"Ԫ������Զ\r\n"
"����������ѻ��С����ˮ�˼ң��ŵ���������\r\n"
"Ϧ�����£��ϳ��������ġ�\r\n\0";

unsigned char* poem_ljx = (unsigned char*)
"���ٽ��ɡ�\r\n"
"��������\r\n"
"������������ˮ���˻��Ծ�Ӣ�ۡ�\r\n"
"�Ƿǳɰ�תͷ�ա�\r\n"
"��ɽ�����ڣ�����Ϧ���졣\r\n"
"�׷����Խ���ϣ��߿����´��硣\r\n"
"һ���Ǿ�ϲ��ꡣ\r\n"
"�Ž�����£�����Ц̸�С�\r\n\0";

unsigned char* poem_ltsl = (unsigned char*)
"������ɳ�\r\n"
"���������\r\n"
"������������������ɺ��\r\n"
"���������������\r\n"
"���ﲻ֪���ǿͣ�һ��̰����\r\n"
"����Īƾ�������޽�ɽ����ʱ���׼�ʱ�ѡ�\r\n"
"��ˮ�仨��ȥҲ�������˼䡣\0";


unsigned char* poem_cjhyy = (unsigned char*)
"����������ҹ��\r\n"
"�ơ�������\r\n"
"������ˮ����ƽ���������¹�������\r\n"
"�����沨ǧ����δ�������������\r\n"
"������ת�Ʒ��飬���ջ��ֽ�������\r\n"
"������˪�����ɣ�͡�ϰ�ɳ��������\r\n"
"����һɫ���˳�������й����֡�\r\n"
"���Ϻ��˳����£����º�������ˣ�\r\n"
"�������������ѣ���������ֻ���ơ�\r\n"
"��֪���´����ˣ�������������ˮ��\r\n"
"����һƬȥ���ƣ�������ϲ�ʤ�\r\n"
"˭�ҽ�ҹ�����ӣ��δ���˼����¥��\r\n"
"����¥�����ǻ���Ӧ������ױ��̨��\r\n"
"�����о�ȥ���������Ϸ�������\r\n"
"��ʱ���������ţ�Ը���»����վ���\r\n"
"���㳤�ɹⲻ�ȣ�����ǱԾˮ���ġ�\r\n"
"��ҹ��̶���仨���������벻���ҡ�\r\n"
"��ˮ����ȥ��������̶���¸���б��\r\n"
"б�³����غ�����ʯ��������·��\r\n"
"��֪���¼��˹飬����ҡ����������\0";

unsigned char* poem_qycx = (unsigned char*)
"����԰����ѩ��\r\n"
"ë�󶫡����ִ�\r\n"
"������⣬ǧ����⣬����ѩƮ��\r\n"
"���������⣬Ω��çç��������£���ʧ���ϡ�\r\n"
"ɽ�����ߣ�ԭ�����������칫�Աȸߡ�\r\n"
"�����գ�����װ�ع���������次�\r\n"
"��ɽ��˶ཿ��������Ӣ�۾�������\r\n"
"ϧ�ػʺ��䣬�����Ĳɣ��������棬��ѷ��ɧ��\r\n"
"һ���콾���ɼ�˼����ֻʶ�乭����\r\n"
"�����ӣ���������������񳯡�\r\n\0";

unsigned char* poem_cyj = (unsigned char*)
"�������ʡ�\r\n"
"�������С����ƴ���\r\n"
"�ְ���������\r\n"
"��ͷ���ˮ���졣\r\n"
"�ĵ��徻��Ϊ����\r\n"
"�˲�ԭ������ǰ��\r\n\0";

unsigned char* poem_qthcx = (unsigned char*)
"��Ǯ�������С�\r\n"
"�Ƴ����׾���\r\n"
"��ɽ�±���ͤ����ˮ���ƽ�ƽŵ͡�\r\n"
"������ݺ��ů����˭�������Ĵ��ࡣ\r\n"
"�һ����������ۣ�ǳ�ݲ���û���㡣\r\n"
"������в��㣬���������ɳ�̡�\r\n\0";


unsigned char* poem_dkh = (unsigned char*)
"���ǿƺ�\r\n"
"�Ƴ����Ͻ�\r\n"
"������������䣬�񳯷ŵ�˼���ġ�\r\n"
"����������㼲��һ�տ�����������\r\n\0";

unsigned char* poem_yqelsg = (unsigned char*)
"�����ض�¦ɽ�ء�\r\n"
"�ִ���ë��\r\n"
"�����ң��������˪���¡�\r\n"
"˪���£��������飬�������ʡ�\r\n"
"�۹�����������������������ͷԽ��\r\n"
"��ͷԽ����ɽ�纣��������Ѫ��\r\n\0";

unsigned char* poem_ltsbdh = (unsigned char*)
"������ɳ�������ӡ�\r\n"
"�ִ���ë��\r\n"
"���������࣬�������죬�ػʵ�����㴬��\r\n"
"һƬ���󶼲�����֪��˭�ߣ�\r\n"
"����Խǧ�꣬κ��ӱޣ�������ʯ����ƪ��\r\n"
"��ɪ�������ǣ������˼䡣\r\n\0";



#define CHINESE_POEM_DELAY 30




DWORD g_cc_color = 0;
DWORD g_cc_idx = 0;
DWORD g_cc_timer = 0;
DWORD g_poem_num = 0;
DWORD g_poem_pos_x = 0;
DWORD g_poem_pos_y = 0;



int initCCFont() {
	int result = 0;
	if (gCCFontBase == 0)
	{
		result = readFile("c:\\liunux\\HZK16", (char**)&gCCFontBase);
	}

	// 	char szout[1024];
	// 	int len = __printf(szout, "initCCFont address:%x,size:%x\r\n",gCCFontBase,result);
	// 	__drawGraphChars((unsigned char*)szout, 0);
	return result;
}


//����*lptest++;�У�++�����ȼ�����*���ᵼ�´�����
void drawCCFontChar(DWORD param1, DWORD param2, DWORD param3, DWORD param4) {

	//__drawGraphChars((unsigned char*)"drawCCFontChar\r\n", 0);

	DWORD* lpcolor = (DWORD*)param2;
	DWORD* idx = (DWORD*)param3;
	DWORD* timer = (DWORD*)param4;

	unsigned short* lpcc = *(unsigned short**)param1;

	if (lpcc[*idx])
	{
		unsigned short unicode[2];
		unicode[0] = lpcc[*idx];
		unicode[1] = 0;
		__drawCCS((unsigned char*)unicode, *lpcolor);
		(*idx)++;
	}
	else {
		__kRemoveExactTimer(*timer);
	}
}


//����*lptest++;�У�++�����ȼ�����*�����´�����
void drawCCFontChar_new(WINDOWCLASS* window, DWORD param2, DWORD param3, DWORD param4) {

	char szout[1024];
	//__printf(szout,"g_poem_num:%d\r\n", g_poem_num);

	unsigned char* g_cc_peoms[] =
	{ poem_jjj,poem_xln, poem_nnjcbhg ,poem_gjtyqy ,poem_gldy ,poem_mjhxh,poem_dgx ,poem_gch ,poem_qyccs,
	poem_tjsqs,poem_ljx ,poem_ltsl,poem_cjhyy,poem_qycx ,poem_cyj ,poem_dkh ,poem_qthcx ,poem_yqelsg ,poem_ltsbdh  };

	unsigned short* lpcc = (unsigned short* )(g_cc_peoms[g_poem_num]);

	if (lpcc[g_cc_idx])
	{
		unsigned short unicode[2];
		unicode[0] = lpcc[g_cc_idx];
		unicode[1] = 0;
		(g_cc_idx)++;
		//__drawCCS((unsigned char*)unicode, g_cc_color);

		unsigned int pos = __getpos(g_poem_pos_x, g_poem_pos_y);
		int resultpos = __drawCC((unsigned char*)unicode, g_cc_color, pos, window->color,window);
		g_poem_pos_y = resultpos / gBytesPerLine;
		g_poem_pos_x = (resultpos % gBytesPerLine) / gBytesPerPixel;

		g_cc_color += 0x0103;

		if (unicode[0] == 0x0a0d || unicode[0] == 0x0d0a) {
			//g_poem_pos_x = window->showX;
			//g_poem_pos_y = window->showY;
			__sleep(100);
		}
	}
	else {
		g_cc_idx = 0;
		g_poem_num++;
		if (g_poem_num >= sizeof(g_cc_peoms) / sizeof(char*)) {
			g_poem_num = 0;
		}

		g_poem_pos_x = window->showX;
		g_poem_pos_y = window->showY+GRAPH_CHINESECHAR_HEIGHT*4;

		__sleep(1000);

		clsClientRect(window);
	}
}


int displayCCPoem() {

	int result = 0;
	//global variable do not need to initialize

	result = initCCFont();
	if (result)
	{
		__drawCCS((unsigned char*)"��ӭ�������ֵ����磡\r\n", 0xff0000);

		g_cc_timer = __kAddExactTimer((DWORD)drawCCFontChar, CHINESE_POEM_DELAY,
			(DWORD)&poem_ljx, (DWORD)&g_cc_color, (DWORD)&g_cc_idx, (DWORD)&g_cc_timer);
	}

	return g_cc_timer;
}





extern "C" __declspec(dllexport) int __kChinesePoem(unsigned int retaddr, int tid, char* filename, char* funcname, DWORD runparam) {
	char szout[1024];

	int retvalue = 0;

	LPTASKCMDPARAMS taskcmd = (LPTASKCMDPARAMS)runparam;
	int cmd = taskcmd->cmd;

	// 	__printf(szout, "cmd:%d,addr:%x,filesize:%d,filename:%s\n", taskcmd->cmd, taskcmd->addr, taskcmd->filesize, taskcmd->filename);

	WINDOWCLASS window;
	__memset((char*)&window, 0, sizeof(WINDOWCLASS));
	__strcpy(window.caption, filename);
	initFullWindow(&window, funcname, tid);

	int result = initCCFont();
	if (result)
	{
		//__drawCCS((unsigned char*)"��ӭ�������ֵ����磡\r\n", 0xff0000);
	}

	g_cc_color = 0x008000;
	g_poem_num = 0;
	g_cc_idx = 0;
	g_poem_pos_x = window.showX;
	g_poem_pos_y = window.showY+ GRAPH_CHINESECHAR_HEIGHT*4;

	while (1)
	{
		unsigned int ck = __kGetKbd(window.id)&0xff;
		if (ck == 0x1b)
		{
			__DestroyWindow(&window);
			return 0;
		}
		else {
			if (ck) {
				while (1) {
					ck = __kGetKbd(window.id) & 0xff;
					if (ck) {
						break;
					}
					else {
						__sleep(0);
					}
				}
			}
		}

		MOUSEINFO mouseinfo;
		__memset((char*)&mouseinfo, 0, sizeof(MOUSEINFO));
		retvalue = __kGetMouse(&mouseinfo, window.id);
		if (mouseinfo.status & 1)	
		{
			if (mouseinfo.x >= window.shutdownx && mouseinfo.x <= window.shutdownx + window.capHeight)
			{
				if (mouseinfo.y >= window.shutdowny && mouseinfo.y <= window.shutdowny + window.capHeight)
				{
					__DestroyWindow(&window);
					return 0;
				}
			}

			while (1) {
				__memset((char*)&mouseinfo, 0, sizeof(MOUSEINFO));
				retvalue = __kGetMouse(&mouseinfo, window.id);
				if (mouseinfo.status & 1){
					break;
				}
				else {
					__sleep(0);
				}
			}
		}

		__sleep(0);

		drawCCFontChar_new(&window, 0, 0, 0);

	}
	return 0;
}

