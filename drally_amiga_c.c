#include "drally.h"
#include "drally_display.h"

#include <libraries/lowlevel.h>
#include <intuition/intuition.h>
#include <intuition/intuitionbase.h>
#include <graphics/videocontrol.h>
#include <workbench/startup.h>
#include <clib/alib_protos.h>
#include <proto/intuition.h>
#include <exec/execbase.h>
#include <proto/exec.h>
#include <proto/keymap.h>
#include <proto/lowlevel.h>
#include <proto/dos.h>
//#include <proto/timer.h>
#include <proto/graphics.h>
#include <proto/icon.h>

#include <cybergraphx/cybergraphics.h>
#include <proto/cybergraphics.h>

#include <SDI_compiler.h>


unsigned int INT8_FRAME_COUNTER = 0;
extern unsigned int ___60458h;
extern unsigned char * VGA13_ACTIVESCREEN;
extern unsigned char VGA13_ACTIVESCREEN_2[];
extern unsigned char VESA101_ACTIVESCREEN[];
extern int iStartX, iEndX, iStartY, iEndY;

extern int _argc;
extern char** _argv;

//unsigned int Ticks = 0;
//unsigned int VRetraceTicks = 0;
unsigned long __stack = (100*1024);
char __stdiowin[]="CON:////Death Rally/CLOSE/WAIT";

void IO_Loop(void);
void __VGA13_PRESENTSCREEN__(void);
void __VESA101_PRESENTSCREEN__(void);
void __PRESENTSCREEN__(void);
__BYTE__ dRally_Keyboard_popLastKey();

static struct GX {
	int 			ActiveMode;
	unsigned char *chunky;
	int width, height;
} GX = {0};
static struct Window *window = NULL;
static struct Screen *screen = NULL;
static unsigned char ppal[256 * 4];
static ULONG spal[1 + (256 * 3) + 1];
static int updatePalette = FALSE;
static int use_c2p = FALSE;
static int use_wcp = FALSE;
static int currentBitMap;
static struct ScreenBuffer *sbuf[2];
static struct RastPort temprp;
static struct BitMap *tempbm;
static struct MsgPort *dispport;
static struct MsgPort *safeport;
static int safetochange = FALSE;
static int safetowrite = FALSE;
static ULONG fsMonitorID = INVALID_ID;
//static ULONG fsModeID = INVALID_ID;
static int fsForce640 = FALSE;
static int rtg320x240 = FALSE;
struct Library *CyberGfxBase = NULL;
static char wndPubScreen[32] = {"Workbench"};
static UWORD *pointermem;
static unsigned char vgapal[768];

extern void ASM c2p1x1_8_c5_bm_040(REG(d0, WORD chunkyx), REG(d1, WORD chunkyy), REG(d2, WORD offsx), REG(d3, WORD offsy), REG(a0, APTR chunkyscreen), REG(a1, struct BitMap *bitmap));
#define c2p_write_bm c2p1x1_8_c5_bm_040

extern __DWORD__ ___60441h;
extern __DWORD__ ___6045ch;
extern __BYTE__ ___60446h;
extern void (*___6044ch)(void);
void __VRETRACE_WAIT_IF_INACTIVE(void);

static void IRQ0_TimerISR(void){

	//if((___6045ch != 0)&&(___60441h == 0)) return;
	INT8_FRAME_COUNTER++;
	if(___60446h == 1) ___6044ch();
}
static APTR timerIntHandle;
static unsigned int currentTicks = 0;
static void updateTimer(void)
{
	if (currentTicks != ___60458h)
	{
		if (currentTicks != 0)
		{
			StopTimerInt(timerIntHandle);
		}
		ULONG timerDelay = (1000 * 1000) / ___60458h;
		StartTimerInt(timerIntHandle, timerDelay, TRUE);
		currentTicks = ___60458h;
	}
	//		___60458h = A; // ticks
	//		___6045ch = B;  // ints
}

int skip;
unsigned int __GET_FRAME_COUNTER(void){

	updateTimer();
	//IO_Loop();
	int counter = INT8_FRAME_COUNTER;
	if (skip == counter)
	{
		if (currentTicks == 60)
		{
			//Delay(1);
			Forbid();
			SysBase->SysFlags |= 1<<15; // scheduling attention required
			Permit();
		}
	}
	else
	{
		IO_Loop();
		skip = counter;
	}
	//printf("%d %d %d %d %d\n", ___60458h, ___6045ch, ___60441h, ___60446h, INT8_FRAME_COUNTER);
	return INT8_FRAME_COUNTER;
}

// preview
void __VRETRACE_WAIT_FOR_START(void){
	
	WaitTOF();
}

// cinematics
void __VRETRACE_WAIT_IF_INACTIVE(void){

	WaitTOF();
}

void __TIMER_SET_TIMER(void){

	//Ticks = SDL_GetTicks();
}

void __WAIT_5(void){

	unsigned int tmp = 5;

	tmp *= ___60458h;
	tmp += __GET_FRAME_COUNTER();

	while((__GET_FRAME_COUNTER() < tmp) && !dRally_Keyboard_popLastKey());
}




void __VESA101_SETMODE();
void __DISPLAY_SET_PALETTE_COLOR(int b, int g, int r, int n);



void __PRESENTSCREEN__(void){
//printf("ActiveMode %d screen %p use_c2p %d window %p\n", GX.ActiveMode, screen, use_c2p, window);
	if(GX.ActiveMode){
		if (screen) {
			int forbidden = FALSE;
			if (GX.ActiveMode == VESA101) {
				// align to 32-pixel boundaries
				iStartX = (iStartX / 32) * 32;
				if (iEndX > 0)
					iEndX = (((iEndX + 32 - 1) / 32) * 32) - 1;
				else
					iEndX = 32 - 1;

				// clip
				if (iStartX < 0)
					iStartX = 0;
				if (iEndX >= GX.width)
					iEndX = GX.width-1;
				if (iEndY < 0)
					iEndY = 0;
				if (iEndY >= GX.height)
					iEndY = GX.height-1;

				if (!(iStartX > iEndX || iStartY > iEndY))
				{
					unsigned char *chunky = GX.chunky + iStartY * GX.width + iStartX;

					if (CyberGfxBase) {
						int width = iEndX - iStartX + 1;
						int height = iEndY - iStartY + 1;
						WritePixelArray(chunky, 0, 0, GX.width, &screen->RastPort, iStartX, iStartY, width, height, RECTFMT_LUT8);
					} else if (use_wcp) {
						WriteChunkyPixels(&screen->RastPort, iStartX, iStartY, iEndX, iEndY, chunky, GX.width);
					} else {
						int width = iEndX - iStartX + 1;
						int height = iEndY - iStartY + 1;
						unsigned char *p_read = chunky;
						for (int j = 0; j < height; j++)
						{
							static unsigned char wplbuf[640];
							memcpy(wplbuf, p_read, width);
							WritePixelLine8(&screen->RastPort, iStartX, iStartY + j, width, wplbuf, &temprp);
							p_read += GX.width;
						}
					}
					/*
					{
						struct RastPort *rp = window->RPort;
						ULONG pen = rand() % 255;
						SetAPen(rp, pen);
						Move(rp, iStartX, iStartY);
						Draw(rp, iEndX, iStartY);
						Draw(rp, iEndX, iEndY);
						Draw(rp, iStartX, iEndY);
						Draw(rp, iStartX, iStartY);
					}
					*/
				}

				iEndX = iEndY = 0;
				iStartX = GX.width;
				iStartY = GX.height;
				//return;
			} else {
				currentBitMap ^= 1;
				/*if (safeport && !safetowrite) {
					while (!GetMsg (safeport)) WaitPort(safeport);
					safetowrite = TRUE;
				}*/
				if (use_c2p) {
					c2p_write_bm(GX.width, GX.height, 0, 0, GX.chunky, sbuf[currentBitMap]->sb_BitMap);
				} else if (CyberGfxBase) {
					temprp.BitMap = sbuf[currentBitMap]->sb_BitMap;
					if (screen->Width >= 640 && screen->Height >= 400) {
						ScalePixelArray(GX.chunky, GX.width, GX.height, GX.width, &temprp, 0, 0, GX.width*2, GX.height*2, RECTFMT_LUT8);
					} else {
						WritePixelArray(GX.chunky, 0, 0, GX.width, &temprp, 0, 0, GX.width, GX.height, RECTFMT_LUT8);
					}
				} /*else if (use_wcp) {
					WriteChunkyPixels(window->RPort, 0, 0, GX.width - 1, GX.height - 1, GX.chunky, GX.width);
				} else {
					WritePixelArray8(window->RPort, 0, 0, GX.width - 1, GX.height - 1, GX.chunky, &temprp);
				}*/
				if (dispport) {
					if (!safetochange) {
						while (!GetMsg(dispport)) WaitPort(dispport);
					}
				} else {
					WaitTOF();
					//WaitBOVP(&screen->ViewPort);
				}
				if (ChangeScreenBuffer(screen, sbuf[currentBitMap])) {
					safetochange = FALSE;
					safetowrite = FALSE;
				}
				safetochange = FALSE;
				safetowrite = FALSE;
			}
			if (updatePalette) {
				//spal[0] = 256 << 16;
				LoadRGB32(&screen->ViewPort, spal);
				updatePalette = FALSE;
			}
		} else if (window) {
			WriteLUTPixelArray(GX.chunky, 0, 0, GX.width, window->RPort, ppal, window->BorderLeft, window->BorderTop, GX.width, GX.height, CTABFMT_XRGB8);
		}
	}
}

void __VGA13_PRESENTSCREEN__(void){

    __PRESENTSCREEN__();
}

void __VESA101_PRESENTSCREEN__(void){

	__PRESENTSCREEN__();
}

void __DISPLAY_SET_PALETTE_COLOR(int b, int g, int r, int n){

	vgapal[3*n] = r;
	vgapal[3*n+1] = g;
	vgapal[3*n+2] = b;
	if(GX.ActiveMode)
	{
		if (screen) {
			spal[n*3+1] = r<<26;
			spal[n*3+2] = g<<26;
			spal[n*3+3] = b<<26;
			updatePalette = TRUE;
		} else {
			ppal[n*4+1] = r<<2;
			ppal[n*4+2] = g<<2;
			ppal[n*4+3] = b<<2;
		}
	}

}

void DISPLAY_CLEAR_PALETTE(void){

	int 	n;

	n = -1;
	while(++n < 0x100) __DISPLAY_SET_PALETTE_COLOR(0, 0, 0, n);
}

void dRally_Display_init(int mode){

	char *exename;
	struct DiskObject *appicon;

	if (_argc == 0) {
		struct WBStartup *startup = (struct WBStartup *)_argv;
		exename = (char *)startup->sm_ArgList->wa_Name;
	} else {
		exename = _argv[0];
	}

	if ((appicon = GetDiskObject((STRPTR)exename))) {
		char *value;

		if ((value = (char *)FindToolType((CONST_STRPTR *)appicon->do_ToolTypes, (CONST_STRPTR)"FORCEMODE"))) {
			if (!strcmp(value, "NTSC"))
				fsMonitorID = NTSC_MONITOR_ID;
			else if (!strcmp(value, "PAL"))
				fsMonitorID = PAL_MONITOR_ID;
			else if (!strcmp(value, "MULTISCAN"))
				fsMonitorID = VGA_MONITOR_ID;
			else if (!strcmp(value, "EURO72"))
				fsMonitorID = EURO72_MONITOR_ID;
			else if (!strcmp(value, "EURO36"))
				fsMonitorID = EURO36_MONITOR_ID;
			else if (!strcmp(value, "SUPER72"))
				fsMonitorID = SUPER72_MONITOR_ID;
			else if (!strcmp(value, "DBLNTSC"))
				fsMonitorID = DBLNTSC_MONITOR_ID;
			else if (!strcmp(value, "DBLPAL"))
				fsMonitorID = DBLPAL_MONITOR_ID;
		}
		if (FindToolType((CONST_STRPTR *)appicon->do_ToolTypes, (CONST_STRPTR)"FORCE640") != NULL) {
			fsForce640 = TRUE;
		}
		if (FindToolType((CONST STRPTR *)appicon->do_ToolTypes, (CONST STRPTR)"RTG320X240") != NULL) {
			rtg320x240 = TRUE;
		}

		FreeDiskObject(appicon);
	}

	pointermem = (UWORD *)AllocVec(2 * 6, MEMF_CHIP | MEMF_CLEAR);
	CyberGfxBase = OpenLibrary((STRPTR)"cybergraphics.library", 41);
	timerIntHandle = AddTimerInt((APTR)IRQ0_TimerISR, NULL);

	if (fsMonitorID == (ULONG)INVALID_ID) {
		ULONG modeID = (ULONG)INVALID_ID;
		// try to find a suitable monitor ID for 640x480 ahead of time
		if (CyberGfxBase) {
			modeID = BestCModeIDTags(
				CYBRBIDTG_Depth, 8,
				CYBRBIDTG_NominalWidth, 640,
				CYBRBIDTG_NominalHeight, 480,
				TAG_DONE);
		}
		if (modeID == (ULONG)INVALID_ID) {
			modeID = BestModeID(
				BIDTAG_NominalWidth, 640,
				BIDTAG_NominalHeight, 480,
				BIDTAG_Depth, 8,
				TAG_DONE);
			if (modeID != (ULONG)INVALID_ID) {
				fsMonitorID = modeID & MONITOR_ID_MASK;
			}
		}
	}

	SetMode(Input(), 1);
}

static void shutdownvideo(void)
{
	if (temprp.BitMap) {
		temprp.BitMap = NULL;
	}
	if (tempbm) {
		FreeBitMap(tempbm);
		tempbm = NULL;
	}
	if (window) {
		CloseWindow(window);
		window = NULL;
	}
	if (sbuf[0]) {
		FreeScreenBuffer(screen, sbuf[0]);
		sbuf[0] = NULL;
	}
	if (sbuf[1]) {
		FreeScreenBuffer(screen, sbuf[1]);
		sbuf[1] = NULL;
	}
	if (dispport) {
		/*
		if (!safetochange) {
			while (GetMsg(dispport));
			safetochange = TRUE;
		}
		*/
		DeleteMsgPort(dispport);
		dispport = NULL;
	}
	if (safeport) {
		/*
		if (!safetowrite) {
			while (GetMsg (safeport));
			safetowrite = TRUE;
		}
		*/
		DeleteMsgPort(safeport);
		safeport = NULL;
	}
	if (screen) {
		CloseScreen(screen);
		screen = NULL;
	}
	use_c2p = 0;
}

void dRally_Display_clean(void){

	shutdownvideo();

	if (pointermem) {
		FreeVec(pointermem);
		pointermem = NULL;
	}
	if (CyberGfxBase) {
		CloseLibrary(CyberGfxBase);
		CyberGfxBase = NULL;
	}
	if (timerIntHandle)
	{
		StopTimerInt(timerIntHandle);
		RemTimerInt(timerIntHandle);
		timerIntHandle = NULL;
	}

	SetMode(Input(), 0);
}

static int setvideomode(int x, int y, int c, int fs)
{
	ULONG flags, idcmp;

	if (fs) {
		ULONG modeID = INVALID_ID;

		if (fsMonitorID != (ULONG)INVALID_ID) {
			//printf("Using forced monitor: %08x\n", fsMonitorID);
		} else if (CyberGfxBase) {
			if (!(rtg320x240 && x == 320 && y == 200)) {
				modeID = BestCModeIDTags(
					CYBRBIDTG_Depth, 8,
					CYBRBIDTG_NominalWidth, x,
					CYBRBIDTG_NominalHeight, y,
					TAG_DONE);
			}

			if (modeID == (ULONG)INVALID_ID && x == 320 && y == 200) {
				// some cards like the Voodoo 3 lack a 320x200 mode
				modeID = BestCModeIDTags(
					CYBRBIDTG_Depth, 8,
					CYBRBIDTG_NominalWidth, 320,
					CYBRBIDTG_NominalHeight, 240,
					TAG_DONE);
			}
		}

		int ntscHack = 0;
		if (modeID == (ULONG)INVALID_ID) {
			modeID = BestModeID(
				BIDTAG_NominalWidth, x,
				BIDTAG_NominalHeight, y,
				BIDTAG_Depth, 8,
				//BIDTAG_DIPFMustNotHave, SPECIAL_FLAGS|DIPF_IS_LACE,
				(fsMonitorID == (ULONG)INVALID_ID) ? TAG_IGNORE : BIDTAG_MonitorID, fsMonitorID,
				TAG_DONE);

			ULONG monitorID = modeID & MONITOR_ID_MASK;
			if (GX.ActiveMode == VESA101 && (monitorID == NTSC_MONITOR_ID || monitorID == EURO36_MONITOR_ID || monitorID == EURO72_MONITOR_ID || monitorID == DBLNTSC_MONITOR_ID))
			{
				struct Rectangle rect;
				if (QueryOverscan(modeID, &rect, OSCAN_MAX))
				{
					//printf("overscan MinX %d MinY %d MaxX %d MaxY %d\n", rect.MinX, rect.MinY, rect.MaxX, rect.MaxY);
					ntscHack = rect.MinY;
				}
			}
		}

		struct TagItem vctl[] =
		{
			//{VTAG_BORDERBLANK_SET, TRUE},
			{VC_IntermediateCLUpdate, FALSE},
			{VTAG_END_CM, 0}
		};

		screen = OpenScreenTags(0,
			modeID != (ULONG)INVALID_ID ? SA_DisplayID : TAG_IGNORE, modeID,
			ntscHack != 0 ? SA_Top : TAG_IGNORE, ntscHack,
			SA_Width, x,
			SA_Height, y,
			SA_Depth, 8,
			SA_ShowTitle, FALSE,
			SA_Quiet, TRUE,
			SA_Draggable, FALSE,
			SA_Type, CUSTOMSCREEN,
			SA_VideoControl, (Tag)vctl,
			//SA_DetailPen, blackcol,
			//SA_BlockPen, whitecol,
			//SA_Overscan, OSCAN_MAX,
			TAG_DONE);

		spal[0] = 256 << 16;
		SetRast(&screen->RastPort, 0);
		for (int i = 0; i < 256; i++)
			SetRGB32(&screen->ViewPort, i, 0, 0, 0);

		currentBitMap = 0;
		use_c2p = use_wcp = 0;

		if (GX.ActiveMode == VESA101) {
			if ((GetBitMapAttr(screen->RastPort.BitMap, BMA_FLAGS) & BMF_STANDARD)) {
				if (((struct Library *)GfxBase)->lib_Version >= 40) {
					use_wcp = TRUE;
				} else {
					// allocate a temp rastport for WritePixelArray8
					tempbm = AllocBitMap(screen->Width, 1, 8, BMF_CLEAR|BMF_STANDARD, NULL);
					if (tempbm) {
						CopyMem(window->RPort, &temprp, sizeof(struct RastPort));
						temprp.Layer = NULL;
						temprp.BitMap = tempbm;
					}
				}
			}
		} else {
			if (!(sbuf[0] = AllocScreenBuffer(screen, 0, SB_SCREEN_BITMAP)) || !(sbuf[1] = AllocScreenBuffer(screen, 0, SB_COPY_BITMAP))) {
				shutdownvideo();
				puts("Could not allocate the screen buffers");
				return -1;
			}
			InitRastPort(&temprp);

			if ((GetBitMapAttr(screen->RastPort.BitMap, BMA_FLAGS) & BMF_STANDARD) != 0 && (x % 32) == 0) {
				use_c2p = TRUE;
			}
			safetochange = TRUE;
			dispport = CreateMsgPort();
			sbuf[0]->sb_DBufInfo->dbi_DispMessage.mn_ReplyPort = dispport;
			sbuf[1]->sb_DBufInfo->dbi_DispMessage.mn_ReplyPort = dispport;
			/*
			safetowrite = TRUE;
			safeport = CreateMsgPort();
			sbuf[0]->sb_DBufInfo->dbi_SafeMessage.mn_ReplyPort = safeport;
			sbuf[1]->sb_DBufInfo->dbi_SafeMessage.mn_ReplyPort = safeport;
			*/
		}
	}

	flags = WFLG_ACTIVATE | WFLG_RMBTRAP;
	idcmp = /*IDCMP_CLOSEWINDOW | IDCMP_ACTIVEWINDOW | IDCMP_INACTIVEWINDOW |*/ IDCMP_RAWKEY;
	if (screen) {
		flags |= WFLG_BACKDROP | WFLG_BORDERLESS;
	} else {
		flags |=  WFLG_DRAGBAR | WFLG_DEPTHGADGET | WFLG_CLOSEGADGET;
	}

	window = OpenWindowTags(0,
		WA_InnerWidth, x,
		WA_InnerHeight, y,
		screen ? TAG_IGNORE : WA_Title, (Tag)"dRally / Open Source Engine / Death Rally [1996]",
		WA_Flags, flags,
		screen ? WA_CustomScreen : TAG_IGNORE, (Tag)screen,
		!screen ? WA_PubScreenName : TAG_IGNORE, (Tag)wndPubScreen,
		WA_IDCMP, idcmp,
		TAG_DONE);

	if (window == NULL) {
		shutdownvideo();
		puts("Could not open the window");
		return -1;
	}
	
	if (pointermem && window->Pointer != pointermem) {
		SetPointer(window, pointermem, 1, 1, 0, 0);
	}

	return 0;
}

void __VGA3_SETMODE(void){

	GX.ActiveMode = VGA3;
}

void __VGA13_SETMODE(void){

	if(GX.ActiveMode != VGA13){

		VGA13_ACTIVESCREEN = VGA13_ACTIVESCREEN_2+20*320;
		GX.ActiveMode = VGA13;
		GX.chunky = VGA13_ACTIVESCREEN;
		GX.width = 320;
		GX.height = 200;
		shutdownvideo();
		if (fsForce640) {
			setvideomode(640, 400, 8, 1);
		} else {
			setvideomode(320, 200, 8, 1);
		}
		/*
		if (setvideomode(320, 200, 8, 1))
		{
			return;
		}
		*/

		iEndX = iEndY = 0;
		iStartX = GX.width;
		iStartY = GX.height;
	}
}

void __VESA101_SETMODE(void){

	if(GX.ActiveMode != VESA101){

		GX.ActiveMode = VESA101;
		GX.chunky = VESA101_ACTIVESCREEN;
		GX.width = 640;
		GX.height = 480;
		shutdownvideo();
		if (setvideomode(640, 480, 8, 1))
		{
			// TODO error handling
			return;
		}

		/*
		iEndX = iEndY = 0;
		iStartX = GX.width;
		iStartY = GX.height;
		*/
		iStartX = iStartY = 0;
		iEndX = GX.width-1;
		iEndY = GX.height-1;
	}
}

void DISPLAY_GET_PALETTE(unsigned char * dst){

	int n;

	if(GX.ActiveMode){

		n = -1;
		while(++n < 0x100*3){
			dst[n] = vgapal[n];
		}
	}
}

void __DISPLAY_GET_PALETTE_COLOR(unsigned char * dst, unsigned char n){

	if(GX.ActiveMode){

		dst[0] = vgapal[3*n];
		dst[1] = vgapal[3*n+1];
		dst[2] = vgapal[3*n+2];
	}
}

void switch_b(__POINTER__ b1, __POINTER__ b2){

	__BYTE__ 	b_tmp;

	b_tmp = B(b1);
	B(b1) = B(b2);
	B(b2) = b_tmp;
}


#if 0
	extern __BYTE__ ___243ca4h[];
	extern __BYTE__ SUPERGLOBAL___243898h[];
	extern __BYTE__ SUPERGLOBAL___243894h[];
	extern __BYTE__ ___243ca8h[];
	extern __BYTE__ ___243874h[];
	extern __BYTE__ ___2438d0h[];

void incCounter(int n){

	if(n == 1) D(___243ca4h)++;
	if(n == 2) D(SUPERGLOBAL___243898h)++;
	if(n == 3) D(SUPERGLOBAL___243894h)++;
	if(n == 4) D(___243ca8h)++;
	if(n == 5) D(___243874h)++;
	if(n == 6) D(___2438d0h)++;
}

void decCounter(int n){

	if(n == 1) D(___243ca4h)--;
	if(n == 2) D(SUPERGLOBAL___243898h)--;
	if(n == 3) D(SUPERGLOBAL___243894h)--;
	if(n == 4) D(___243ca8h)--;
	if(n == 5) D(___243874h)--;
	if(n == 6) D(___2438d0h)--;
}

__DWORD__ getCounter(int n){

	if(n == 1) return D(___243ca4h);
	if(n == 2) return D(SUPERGLOBAL___243898h);
	if(n == 3) return D(SUPERGLOBAL___243894h);
	if(n == 4) return D(___243ca8h);
	if(n == 5) return D(___243874h);
	if(n == 6) return D(___2438d0h);

	return 0;
}

void resetCounter(int n){

	if(n == 1) D(___243ca4h) = 0;
	if(n == 2) D(SUPERGLOBAL___243898h) = 0;
	if(n == 3) D(SUPERGLOBAL___243894h) = 0;
	if(n == 4) D(___243ca8h) = 0;
	if(n == 5) D(___243874h) = 0;
	if(n == 6) D(___2438d0h) = 0;
}

void setCounter(int n, __DWORD__ val){

	if(n == 1) D(___243ca4h) = val;
	if(n == 2) D(SUPERGLOBAL___243898h) = val;
	if(n == 3) D(SUPERGLOBAL___243894h) = val;
	if(n == 4) D(___243ca8h) = val;
	if(n == 5) D(___243874h) = val;
	if(n == 6) D(___2438d0h) = val;
}
#endif

void dRally_Keyboard_make(SDL_Scancode);
void dRally_Keyboard_break(SDL_Scancode);

#include "drally_structs_free.h"
#include "drally_keyboard.h"

	extern __BYTE__ LAST_KEY;
	extern __BYTE__ kmap[];
	extern kb_control_t ___1a1140h;

ULONG oldportstate;

#define CheckJoyEvent(key, mkey, mask) \
	do { \
		if ((portstate & mask) != (oldportstate & mask)) { \
			kmap[key] = !!(portstate & mask); \
			if (mkey != -1) LAST_KEY = (portstate & mask) ? mkey : 0; \
		} \
	} while(0)

static void ReadJoystick(ULONG port)
{
	ULONG portstate;

	portstate = ReadJoyPort(port);

	if (((portstate & JP_TYPE_MASK) == JP_TYPE_GAMECTLR) || ((portstate & JP_TYPE_MASK) == JP_TYPE_JOYSTK))
	{
		CheckJoyEvent(___1a1140h.accelerate, DR_SCAN_UP, JPF_JOY_UP);
		CheckJoyEvent(___1a1140h.brake, DR_SCAN_DOWN, JPF_JOY_DOWN);
		CheckJoyEvent(___1a1140h.steer_left, DR_SCAN_LEFT, JPF_JOY_LEFT);
		CheckJoyEvent(___1a1140h.steer_right, DR_SCAN_RIGHT, JPF_JOY_RIGHT);

		CheckJoyEvent(___1a1140h.machine_gun, DR_SCAN_ENTER, JPF_BUTTON_RED); // joystick button 1
		CheckJoyEvent(___1a1140h.turbo_boost, DR_SCAN_ESCAPE, JPF_BUTTON_BLUE);// joystick button 2
		CheckJoyEvent(___1a1140h.drop_mine, -1, JPF_BUTTON_YELLOW);
		CheckJoyEvent(___1a1140h.drop_mine, -1, JPF_BUTTON_PLAY); // joystick button 3

		// in-game dialog boxes
		CheckJoyEvent(DR_SCAN_Y, -1, JPF_BUTTON_RED);
		CheckJoyEvent(DR_SCAN_N, -1, JPF_BUTTON_BLUE);

		oldportstate = portstate;
	}
}

void IO_Loop(void){

	struct IntuiMessage *imsg;

	if (!window) {
		return;
	}

	// re-activate the window if needed
	if (screen && screen == IntuitionBase->FirstScreen && !(window->Flags & WFLG_WINDOWACTIVE)) { // IntuitionBase->ActiveWindow = window
		ActivateWindow(window);
	}

	while ((imsg = (struct IntuiMessage *)GetMsg(window->UserPort)))
	{
		switch (imsg->Class)
		{
		case IDCMP_RAWKEY:
			{
				int press = (imsg->Code & IECODE_UP_PREFIX) == 0;
				int code = imsg->Code & ~IECODE_UP_PREFIX;

				if (press)
					dRally_Keyboard_make(code);
				else
					dRally_Keyboard_break(code);
			}
			break;
			/*
		case IDCMP_ACTIVEWINDOW:
			appactive = 1;
			break;
		case IDCMP_INACTIVEWINDOW:
			appactive = 0;
			break;
		case IDCMP_CLOSEWINDOW:
			quitevent = 1;
			break;
			*/
		default:
			//printf("IDCMP %x\n",imsg->Class);
			break;
		}

		ReplyMsg((struct Message *)imsg);
	}

	//updatejoystick();
	ReadJoystick(1);

	// input/network callback
	//if(___60446h == 1) ___6044ch();
}

#include <stdarg.h>
void BE_ST_DebugText(int x, int y, const char *fmt, ...)
{
	char buffer[256];
	va_list ap;

	va_start(ap, fmt); 
	vsnprintf(buffer, sizeof(buffer), fmt, (char *)ap);
	va_end(ap);

	Move(&screen->RastPort, x, y + screen->RastPort.Font->tf_Baseline);
	Text(&screen->RastPort, (STRPTR)buffer, strlen(buffer));
	/*
	Move(window->RPort, x, y + window->RPort->Font->tf_Baseline);
	Text(window->RPort, (STRPTR)buffer, strlen(buffer));
	*/
}
