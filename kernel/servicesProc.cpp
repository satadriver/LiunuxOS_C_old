#include "servicesProc.h"
#include "task.h"

DWORD __kServicesProc(DWORD no, DWORD * params) {
	switch (no)
	{
	case KBD_OUTPUT:
	{

		break;
	}
	case KBD_INPUT:
	{
		break;
	}
	case MOUSE_OUTPUT:
	{
		break;
	}
	case GRAPH_CHAR_OUTPUT:
	{
		break;
	}
	case RANDOM:
	{
		break;
	}
	case SLEEP:
	{
		DWORD times = params[0] / 10;
		DWORD mod = params[0] % 10;
		if (mod == 0 && times == 0)
		{
			times++;
		}
		else if (mod == 0 && times != 0)
		{

		}
		else if (mod != 0 && times == 0)
		{
			times++;
		}
		else if (mod != 0 && times != 0)
		{
			times++;
		}

		if (times > 0)
		{
			LPPROCESS_INFO tss = (LPPROCESS_INFO)CURRENT_TASK_TSS_BASE;
			tss->sleep = times - 1;

		}
		__asm {
			hlt;
		}

// 		for (int i = 0; i < times; i++)
// 		{
// 			__asm {
// 				hlt
// 			}
// 		}

		break;
	}
	case TURNON_SCREEN:
	{
		break;
	}
	case TURNOFF_SCREEN:
	{
		break;
	}
	case CPU_MANUFACTORY:
	{
		break;
	}
	case TIMESTAMP:
	{
		break;
	}
	case SWITCH_SCREEN:
	{
		break;
	}
	case CPUINFO:
	{
		break;
	}
	case DRAW_MOUSE:
	{
		break;
	}
	case RESTORE_MOUSE:
	{
		break;
	}
	case SET_VIDEOMODE:
	{
		break;
	}

	default: {
		break;
	}
	}

	return 0;
}