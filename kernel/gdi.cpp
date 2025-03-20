
#include "gdi.h"
#include "Utils.h"
#include "math.h"
#include "file.h"
#include "malloc.h"
#include "cmosPeriodTimer.h"
#include "video.h"
#include "cmosExactTimer.h"
#include "hardware.h"

float f(float x, float y, float z) {
	return x*x + y*y + z*z - 1;
}

float h(float x, float z) {
	//��ȡ����(�ⷽ�̵Ĺ���)
	//�������˵�� Y�� ��ֱֽ������ ��������ϵ
	for (float y = 1.0f; y >= 0.0f; y -= 0.001f) {
		if (f(x, y, z) <= 0.0f)
			return y;
	}
	return 0.0f;
}


void sphere(int x,int y,int raduis,DWORD color,unsigned char * backup) {

// 	__asm {
// 		clts
// 		fwait
// 		fninit
// 	}
	unsigned int pos = __getpos(x - raduis, y - raduis);
	unsigned char * showpos = pos + (unsigned char *)gGraphBase;
	unsigned char * keepy = showpos;


	DWORD r = 0;

	int width = raduis*2, height = raduis*2;
	for (int sy = 0; sy < height; sy++) {
		float z = 1.5f - sy * 3.0f / height;//����������Ļ����ת��ģ�ʹ�С�ĳ߶�
		for (int sx = 0; sx < width; sx++) {
			float x = sx *3.0f / width - 1.5f;//����������Ļ����ת��ģ�ʹ�С�ĳ߶�

			if (f(x, 0.0f, z) <= 0.0f) {//��������
										//���淨������ʽd=[x,y,z]
				float y = h(x, z);
				float cosA = (-x + y + z) / __sqrt(x*x + y*y + z*z) / __sqrt(3);//(����������[1,-1,-1])
				float d = cosA*0.5f + 0.5f;//wrapped diffuse����
											//float d = cosA+0.2f>1.0f?1.0f:cosA+0.2f;//����ȫ�ֹ⴦��

				r = (DWORD)(d*255.0f);

			}
			else {
				r = color;
			}

			char * data = (char*)&r;
			for (int i = 0; i < gBytesPerPixel; i++)
			{
				*backup = *showpos;
				backup++;

				*showpos = data[i];
				showpos++;
			}
		}
		keepy += gBytesPerLine;
		showpos = keepy;
	}
}


void sphere7(int x, int y, int raduis,DWORD color,unsigned char *backup) {
 	__asm {
 		//clts
 		//fwait
 		//fninit
 	}
	unsigned int pos = __getpos(x-raduis, y-raduis);
	unsigned char * showpos = pos + (unsigned char *)gGraphBase;
	unsigned char * keepy = showpos;
	int width = raduis*2, height = raduis*2;

	DWORD c = 0;

	for (int sy = 0; sy < height; sy++) {
		float z = 1.5f - sy * 3.0f / height;//����������Ļ����ת��ģ�ʹ�С�ĳ߶�
		for (int sx = 0; sx < width; sx++) {
			float x = sx *3.0f / width - 1.5f;//����������Ļ����ת��ģ�ʹ�С�ĳ߶�

			if (f(x, 0.0f, z) <= 0.0f) {//��������
										//���淨������ʽd=[x,y,z]
				float y = h(x, z);
				float cosA = (-x + y + z) / __sqrt(x*x + y*y + z*z) / __sqrt(3);//(����������[-1,1,1])

																			  //�����䣬wrapped diffuse����
				float diffuse = cosA*0.5f + 0.5f;
				//float d = cosA+0.2f>1.0f?1.0f:cosA+0.2f;//����ȫ�ֹ⴦��

				//�߹��
				float specular = __pow(cosA, 64.0);

				int r = (int)(diffuse * 255.0f + specular * 200.0f);
				int g = (int)(diffuse * 255.0f + specular * 200.0f);
				int b = (int)(diffuse * 255.0f + specular * 200.0f);
// 				int g = (int)(specular * 200.0f);
// 				int b = (int)(specular * 200.0f);
				r = r > 255 ? 255 : r;
				g = g > 255 ? 255 : g;
				b = b > 255 ? 255 : b;

				c = (r << 16) + (g << 8) + b;
			}
			else {
				c = color;
			}
			char * data = (char*)&c;
			for (int i = 0; i < gBytesPerPixel; i++)
			{
				*backup = *showpos;
				backup++;

				*showpos = data[i];
				showpos++;
			}
		}
		keepy += gBytesPerLine;
		showpos = keepy;
	}

}



DWORD showIcon(int x,int y, LPBITMAPINFOHEADER lpbmpinfo) {

	int width = lpbmpinfo->biWidth;

	int height = lpbmpinfo->biHeight;

	int bitspix = lpbmpinfo->biBitCount>>3;

	unsigned char * data = (unsigned char*)lpbmpinfo + sizeof(BITMAPINFOHEADER);

	unsigned char * ptr = (unsigned char*)__getpos(x, y) + gGraphBase;

	unsigned char * keep = ptr;

	for (int i = 0; i < height; i++)
	{
		for (int j = 0; j < width; j++)
		{
			unsigned char * p = ptr;
			unsigned char * d = data;
			for (int k = 0; k < gBytesPerPixel; k++)
			{
				*p = *d;
				p++;
				d++;
			}

			ptr += gBytesPerPixel;
			data += bitspix;
		}

		keep += gBytesPerLine;
		ptr = keep;
	}

	return (DWORD)ptr- gGraphBase;
}


int showBmpBits(int x, int y, BITMAPINFOHEADER* info, unsigned char * data) {

	char szout[1024];
	if (info->biBitCount < 8)
	{
		__printf(szout, ( char*)"bmp file bit error\n");
		return FALSE;
	}

	int bytesperline = info->biWidth * (info->biBitCount >> 3);
	int mod = (info->biWidth * (info->biBitCount >> 3)) % 4;
	if (mod)
	{
		bytesperline = bytesperline + 4 - mod;
	}

	unsigned char * colordata = 0;
	unsigned char * color = 0;
	unsigned int deltaline = 0;

	int width = info->biWidth;
	int height = info->biHeight;
	if (width < 0 && height < 0)
	{
		colordata = data;
		color = colordata;
		deltaline = bytesperline;
	}
	else {
		colordata = data + bytesperline*height - bytesperline;
		color = colordata;
		deltaline = -bytesperline;
	}

	RGBQUAD * rgb = (RGBQUAD*)(data);;
	if (info->biBitCount == 8) {
		data += sizeof(RGBQUAD) * 256;
	}
	unsigned char * screenpos = (unsigned char*)gGraphBase + __getpos(x, y);
	unsigned char * keepy = screenpos;

	for (int i = 0; i < height; i++)
	{
		for (int j = 0; j < width; j++)
		{
			int ic = 0;
			unsigned char * cc = (unsigned char*)&ic;
			if (info->biBitCount == 8)
			{
				cc[0] = rgb[*color].rgbBlue;
				cc[1] = rgb[*color].rgbGreen;
				cc[2] = rgb[*color].rgbRed;
				color++;
			}
			else if (info->biBitCount == 16)
			{
				//xxxx x xxx xxx x xxxx
				int tmpc = *(unsigned short*)color;
				cc[2] = (tmpc & 0xf800) >> 8;
				cc[1] = (tmpc & 0x7e0) >> 3;
				cc[0] = tmpc & 0x1f << 3;
				color += 2;
			}
			else if (info->biBitCount == 15)
			{
				//xxxx xx xx xxx x xxxx
				int tmpc = *(unsigned short*)color;
				cc[2] = (tmpc & 0x7c00) >> 8;
				cc[1] = (tmpc & 0x3e0) >> 3;
				cc[0] = tmpc & 0x1f << 3;
				color += 2;
			}
			else if (info->biBitCount == 24)
			{
				cc[0] = color[0];
				cc[1] = color[1];
				cc[2] = color[2];
				color += 3;
			}
			else if (info->biBitCount == 32)
			{
				cc[0] = color[0];
				cc[1] = color[1];
				cc[2] = color[2];
				cc[3] = color[3];
				color += 4;
			}
			else {
				return FALSE;
			}

			for (int k = 0; k < gBytesPerPixel; k++)
			{
				*screenpos = *cc;
				cc++;
				screenpos++;
			}
		}

		keepy += gBytesPerLine;
		screenpos = keepy;

		colordata = colordata + deltaline;
		color = colordata;
	}

	return info->biSizeImage;
}






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
"����ū������ڻ��š� \r\n"
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
" ���Ρ�����\r\n"
"ŭ����ڣ�ƾ������������Ъ��\r\n"
"̧���ۡ����쳤Х��׳�����ҡ���ʮ��������������ǧ��·�ƺ��¡�\r\n"
"Ī���С���������ͷ���ձ��С�\r\n"
"�����ܣ���δѩ�����Ӻޣ���ʱ��\r\n"
"�ݳ�����̤�ƺ���ɽȱ��׳־���ͺ�²�⣬Ц̸������ūѪ��\r\n"
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
"Ԫ ������Զ\r\n"
"����������ѻ��С����ˮ�˼ң��ŵ���������Ϧ�����£��ϳ��������ġ�\r\n\0";

unsigned char* poem_ljx = (unsigned char*)
"���ٽ��ɡ�\r\n"
"��������\r\n"
"������������ˮ���˻��Ծ�Ӣ�ۡ��Ƿǳɰ�תͷ�ա���ɽ�����ڣ�����Ϧ���졣\r\n"
"�׷����Խ���ϣ��߿����´��硣һ���Ǿ�ϲ��ꡣ�Ž�����£�����Ц̸�С�\r\n\0";

unsigned char* poem_ltsl = (unsigned char*)
"������ɳ�\r\n"
"���  ����\r\n"
"������������������ɺ��������������������ﲻ֪���ǿͣ�һ��̰����\r\n"
"����Īƾ�������޽�ɽ����ʱ���׼�ʱ�ѡ���ˮ�仨��ȥҲ�������˼䡣\0";


unsigned char* poem_cjhyy = (unsigned char*)
"����������ҹ��\r\n"
"�� �� ������\r\n"
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
"ë�󶫡� ���ִ�\r\n"
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
"�Ƴ� �׾���\r\n"
"��ɽ�±���ͤ����ˮ���ƽ�ƽŵ͡�\r\n"
"������ݺ��ů����˭�������Ĵ��ࡣ\r\n"
"�һ����������ۣ�ǳ�ݲ���û���㡣\r\n"
"������в��㣬���������ɳ�̡�\r\n\0";


unsigned char* poem_dkh = (unsigned char*)
"���ǿƺ�\r\n"
"�Ͻ�\r\n"
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




#define CHINESE_POEM_DELAY 60


unsigned char* g_cc_peoms[] = { poem_jjj,poem_xln, poem_nnjcbhg ,poem_gjtyqy ,poem_gldy ,poem_mjhxh,poem_dgx ,poem_gch ,
poem_qyccs,poem_tjsqs,poem_ljx ,poem_ltsl,poem_cjhyy,poem_qycx ,poem_cyj ,poem_dkh ,poem_qthcx ,poem_yqelsg ,poem_ltsbdh };

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





int displayCCPoem() {

	int result = 0;
	//global variable do not need to initialize

	result = initCCFont();
	if (result)
	{
		g_poem_num = 0;
		g_cc_idx = 0;
		g_poem_pos_x = 0;
		g_poem_pos_y = gWindowHeight - TASKBAR_HEIGHT;

		__drawCCS((unsigned char*)"��ӭ�������ֵ����磡\r\n", 0xff0000);

		g_cc_timer = __kAddExactTimer((DWORD)drawCCFontChar, CHINESE_POEM_DELAY, 
			(DWORD)&poem_ljx, (DWORD)&g_cc_color, (DWORD)&g_cc_idx, (DWORD)&g_cc_timer);
	}

	return g_cc_timer;
}




void SetPalette(char* palette) {

	for (int i = 0; i < 256; i++)
	{
		outportb(0x3c8, i);
		outportb(0x3c9, palette[i*4]);
		outportb(0x3c9, palette[i*4 + 1]);
		outportb(0x3c9, palette[i*4 + 2]);
	}
}