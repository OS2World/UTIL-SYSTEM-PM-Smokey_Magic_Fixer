/* smf.h.  Header file for smokey magic fixer
 *  contains defines, user messages, function prototypes, & global variables
 */

#define ID_RESOURCE  1

/* sizes and locations of the metric windows */
#define CXMET   180
#define CYMET   265
#define XMET0    5
#define YMET0    5
#define XMET1   455
#define YMET1    5

/* general width for many windows */
#define WIDTH   180

/* number of steps for FlyWinWin() */
#define FWW_SHORT   10
#define FWW_MEDIUM  30
#define FWW_LONG    50

#define FWW_MODSIZE  5


/* window ids of the buttons on the main window */
#define ID_SF_SELECT        0x0300
#define ID_SF_CREATE        0x0301
#define ID_SF_SETCHB        0x0302

#define ID_SF_TEST          0x0500


/* font identifier for gpicreatelogfont.  just some unique number [1,255] */
#define OUR_LCID        27L

/* the following are messages passed to or from the main window */

#define WM_ALL_MAIN_FONTSELECTED  WM_USER+100

#define WM_MAIN_MET_CHANGE        WM_USER+200
                                       /* mp1: &fm
                                        */
#define WM_MAIN_FAT_CHANGE   WM_USER+201
                                       /* mp1: &fm
                                        */
#define WM_MAIN_CHB_FILLCHB  WM_USER+300
                                       /* mp1: hps
                                        */
#define WM_MAIN_CHB_READ     WM_USER+301
                                       /* mp1: &charbundle,
                                        * return: TRUE iff successful
                                        */
#define WM_MAIN_FAT_READ     WM_USER+302
                                       /* mp1: &fattrs
                                        * return: TRUE iff successful
                                        */

#define WM_MAIN_TEST_GETPS  WM_USER+400
                                        /* return: hps */
#define WM_MAIN_TEST_GETFM  WM_USER+401
                                        /* return: &fm */


/* entry field i.d.s for the FATTRS dialog box */
#define F_usRecordLength    0x1001
#define F_fsSelection       0x1002
#define F_lMatch            0x1003
#define F_szFacename        0x1004
#define F_idRegistry        0x1005
#define F_usCodePage        0x1006
#define F_lMaxBaselineExt   0x1007
#define F_lAveCharWidth     0x1008
#define F_fsType            0x1009
#define F_fsFontUse         0x100A

/* entry field i.d.s for the Charbundle dialog box */
#define CB_lColor           0x2001
#define CB_lBackColor       0x2002
#define CB_usMixMode        0x2003
#define CB_usBackMixMode    0x2004
#define CB_usSet            0x2005
#define CB_usPrecision      0x2006
#define CB_sizfxCell_X      0x2007
#define CB_sizfxCell_Y      0x2008
#define CB_ptlAngle_X       0x2009
#define CB_ptlAngle_Y       0x200A
#define CB_ptlShear_X       0x200B
#define CB_ptlShear_Y       0x200C
#define CB_usDirection      0x200D



/* function prototypes and window handles */

VOID    FrameInit(HWND);
MRESULT EXPENTRY ClientWndProc (HWND, USHORT, MPARAM, MPARAM);
HWND    hwndFrame, hwndMain;  /* hwndMain is the main client area */
HWND    hwndCreate, hwndSelect, hwndSetCHB;

VOID    AllInit (HWND);
MRESULT EXPENTRY allWndProc (HWND, USHORT, MPARAM, MPARAM);
HWND    hwndAll;

int cdecl MetInit (HWND);
MRESULT EXPENTRY MetWndProc (HWND, USHORT, MPARAM, MPARAM );
HWND    hMet0, hMet1;

VOID    FattrInit(HWND);
MRESULT EXPENTRY FattrWndProc (HWND, USHORT, MPARAM, MPARAM);
HWND    hwndFattrs, hwndCharbundle;

VOID    TestInit(HWND);
MRESULT EXPENTRY TestWndProc (HWND, USHORT, MPARAM, MPARAM);
HWND    hwndTest;


/* metric windows contain an index in their extra bytes.
 *  It indexes an array of the following structures.
 *  The array is global, but only referenced by the metwndproc
 */
typedef struct  _metExtraBytes {
    HWND   hwndVscroll ;
    SHORT  sVscrollMax;
    SHORT  sVscrollPos;
    SHORT  cxChar;
    SHORT  cyChar;
    SHORT  cyDesc;
    SHORT  cxClient;
    SHORT  cyClient;
    HPS    hFontPS;
    HDC    hFontDC;
    LONG   theFontSem;
    FONTMETRICS theFont;
} metExtraBytes;



/* a small font.  used to setting pres params */
static CHAR  szCourier[] = "8.Tms Rmn";

/* see GpiSetAttrs/ GpiQueryAttrs */
static LONG  flAttrsMask =
     CBB_COLOR | CBB_BACK_COLOR | CBB_MIX_MODE |
     CBB_BACK_MIX_MODE | CBB_SET | CBB_MODE | CBB_BOX |
     CBB_ANGLE | CBB_SHEAR | CBB_DIRECTION;




/* global variables. */

HAB     hab;

CHARBUNDLE  charbundle;  /* mirrors the charbundle window */
FATTRS      fattrs;      /* mirrors the fattrs window */

#define NCHAR 100
CHAR   buffer[NCHAR];  /* storage for the face name pulled out of window */

FONTMETRICS  fm;    /* holds dynamic f.m. after create log font */
