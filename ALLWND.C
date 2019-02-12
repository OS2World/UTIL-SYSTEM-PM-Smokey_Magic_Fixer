/* allwnd.c:  Show all the fonts via their own look
 *  original code by Steve Firebaugh  4/20/90
 */

/* additions:
 *  1.  Be clever about placing the fonts on the screen
 *          based on the total number we are going to put up
 *  2.  Display only the fonts which match the screen resolution.
 *          optionally
 *  3.  Save the window with fonts as a big bitmap once.  Later
 *          just blt it back to the screen.  [ done 6/9/90 ]
 */



#define INCL_WIN
#define INCL_GPI
#include <os2.h>
#include <stdio.h>
#include "smf.h"

/* our allwnd is a frame, subclassed after the WM_CREATE goes through.
 *  we need a time to do initializations thus ...
 */
#define WM_USER_INIT    WM_USER+57

/* used in the font placement and mouse hitesting functions */
#define XINCR   150
#define YINCR    20
#define PERCOL   20


/* indent the allwnd window this much from its parent */
#define  BORDER  3

VOID QueryAllFonts (HPS);       //forward declaration.  HACK!


static  PFONTMETRICS pfm;

/* used to draw a bitmap into memory, then blit on the screen */
HPS hpsMemoryBitmap = NULL;

/* size of the allwnd window.  Also used to size memory bitmap */
RECTL  recWindow;

/* the allwnd window is a frame.  Subclass it and remember its windproc */
PFNWP   pfnOld = NULL;


VOID AllInit (HWND hwnd)
{

    /* first set up the window */
    {
    WinQueryWindowRect( hwnd, &recWindow);

    recWindow.xRight = (recWindow.xRight - recWindow.xLeft - 2*BORDER);
    recWindow.yTop   = (recWindow.yTop - recWindow.yBottom - 2*BORDER);

    hwndAll = WinCreateWindow (hwnd, WC_FRAME, "AllWnd", FS_BORDER ,
                BORDER, BORDER,
                (SHORT) recWindow.xRight,
                (SHORT) recWindow.yTop,
                hwnd, HWND_TOP, 999, NULL, NULL);

    pfnOld = WinSubclassWindow (hwndAll, allWndProc);
    WinPostMsg (hwndAll, WM_USER_INIT, 0L, 0L);
    }


    /* now fill in the array of all FONTMETRICS */
    {
    LONG  nFonts, lTemp = 0;
    HPS   hps;
    SEL   sel;

    hps = WinGetPS (hwndAll);

    /* Determine the number of fonts. */
    nFonts = GpiQueryFonts(hps, QF_PUBLIC, NULL, &lTemp,
         (LONG) sizeof(FONTMETRICS), NULL);

    /* Allocate space for the font metrics. */
    DosAllocSeg((USHORT) (sizeof(FONTMETRICS) * nFonts),
        &sel, SEG_NONSHARED);
    pfm = MAKEP(sel, 0);

    /* Retrieve the font metrics. */
    GpiQueryFonts(hps, QF_PUBLIC, NULL, &nFonts,
      (LONG) sizeof(FONTMETRICS), pfm);

    WinReleasePS(hps);


    /* beginning of controversial bitmap addition. */
    {
    HDC hdc;
    static SIZEL sizl = { 0, 0 };    /* use same page size as device */
    DEVOPENSTRUC dop;

    dop.pszLogAddress = NULL;
    dop.pszDriverName = (PSZ) "DISPLAY";
    dop.pdriv = NULL;
    dop.pszDataType = NULL;

    /* Create the memory device context. */

    hdc = DevOpenDC(hab, OD_MEMORY, "*", 4L, &dop, NULL);

    /* Create the presentation and associate the memory device context. */

    hpsMemoryBitmap = GpiCreatePS(hab, hdc, &sizl,
                              PU_PELS | GPIT_MICRO | GPIA_ASSOC);

    {
    BITMAPINFOHEADER bmp;
    HBITMAP hbm;

    bmp.cbFix = sizeof (BITMAPINFOHEADER);
    bmp.cx = (USHORT) recWindow.xRight - recWindow.xLeft;
    bmp.cy = (USHORT) recWindow.yTop - recWindow.yBottom;
    bmp.cPlanes = 1;
    bmp.cBitCount = 4;

    hbm = GpiCreateBitmap(hpsMemoryBitmap, &bmp, 0L, NULL, NULL);

    /* Set the bitmap and draw in it. */

    GpiSetBitmap(hpsMemoryBitmap, hbm);  /* sets bitmap in device context */
    // QueryAllFonts (hpsMemoryBitmap);  /* do the drawing */


    }
    }
    /* end of bitmap stuff */



    }
}





/* Need two (inverse) mappings.
 *
 *  lMatch to (x,y)
 *
 *        column index = lMatch div PERCOL
 *        row index    = lMatch mod PERCOL
 *
 *        x = column index * XINCR
 *        y = row index    * YINCR
 *
 *
 *  (x,y) to lMatch
 *
 *        lMatch = (column index * PERCOL) +        // x contribution
 *                 row index                        // y contribution
 */

LONG   fnlmatchtox (LONG lMatch, LONG nFonts)
{
    return (LONG) XINCR * (lMatch/PERCOL);
}

LONG   fnlmatchtoy (LONG lMatch, LONG nFonts)
{
    return (LONG) YINCR * (lMatch%PERCOL);
}


LONG   fnxytolmatch (SHORT x, SHORT y)
{
    LONG rowindex, colindex, lMatch;

    rowindex = y / (LONG) YINCR;
    colindex = x / (LONG) XINCR;

    lMatch = colindex * PERCOL + rowindex;
    return lMatch;
}



/********************** ClientWndProc **************************/
MRESULT EXPENTRY allWndProc (HWND hwnd, USHORT msg, MPARAM mp1, MPARAM mp2)
   {
   HPS    hps;
   RECTL  rcl;
   static POINTL aptl[4];
   static BOOL fGrabbedBitmap = FALSE;

   switch (msg) {

    case WM_USER_INIT:
        aptl[0].x = aptl[0].y = aptl[2].x = aptl[2].y = 0;
        aptl[1].x = aptl[3].x = recWindow.xRight;
        aptl[1].y = aptl[3].y = recWindow.yTop;
        break;


    case WM_BUTTON1DOWN: {
        SHORT x,y;
        LONG  lMatch;
        SHORT index;

        /* figure out which of the fonts was selected */
        x= SHORT1FROMMP (mp1);
        y= SHORT2FROMMP (mp1);
        lMatch = fnxytolmatch (x,y);
        index = (SHORT) lMatch -1;

        /* pass the font metric to the main window.  it passes to metric wnd */
        WinPostMsg (hwndMain, WM_ALL_MAIN_FONTSELECTED,
                    (MPARAM) &(pfm[index]), 0L);
        WinShowWindow (hwndAll, FALSE);
        } break;

    case WM_PAINT:
        hps = WinBeginPaint (hwnd, NULL, &rcl);

        if (!fGrabbedBitmap) {
            QueryAllFonts (hps);  /* draw to the visible window */

           GpiBitBlt(hpsMemoryBitmap,  /* slide the bits off into memory */
                hps,
                4L,
                aptl,
                ROP_SRCCOPY,
                BBO_IGNORE);

            fGrabbedBitmap = TRUE;
        }
        else {  /* we have the memory bitmap drawn, blit it out */

            GpiBitBlt(hps,        /* target presentation space      */
                hpsMemoryBitmap,              /* source presentation space      */
                4L,               /* four points needed to compress */
                aptl,             /* points to source and target    */
                ROP_SRCCOPY,      /* copy source replacing target   */
                BBO_IGNORE);      /* discard extra rows and columns */
        }
        WinEndPaint (hps);
        return 0;
     }
   if (pfnOld != NULL) return pfnOld (hwnd, msg, mp1, mp2);
   }









VOID QueryAllFonts (HPS hps)
{
    SHORT   i;
    LONG cFonts, lTemp = 0L;
    SEL sel;
    PFONTMETRICS pfm;
    CHAR    buffer [200];
    LONG    nFonts;

    GpiErase (hps);

    /* Determine the number of fonts. */
    nFonts = GpiQueryFonts(hps, QF_PUBLIC, NULL, &lTemp,
         (LONG) sizeof(FONTMETRICS), NULL);

    /* Allocate space for the font metrics. */
    DosAllocSeg((USHORT) (sizeof(FONTMETRICS) * nFonts),
        &sel, SEG_NONSHARED);
    pfm = MAKEP(sel, 0);

    /* Retrieve the font metrics. */
    cFonts = GpiQueryFonts(hps, QF_PUBLIC, NULL, &nFonts,
      (LONG) sizeof(FONTMETRICS), pfm);



    for (i = (SHORT) nFonts; i > 0; i--) {
        /******************************/ {
        USHORT ii;
        POINTL ptl ;
        FATTRS fat;
        ptl.x = fnlmatchtox((LONG)i, nFonts);
        ptl.y = fnlmatchtoy((LONG)i, nFonts);

        fat.usRecordLength = sizeof(FATTRS); /* sets size of structure      */
        fat.fsSelection = 0;            /* uses default selection           */
        fat.lMatch = (LONG) pfm[i-1].lMatch;
        fat.idRegistry = 0;             /* uses default registry            */
        fat.usCodePage = 850;           /* code-page 850                    */
        fat.lAveCharWidth =
        fat.lMaxBaselineExt = 0;
        fat.fsType = 0;                 /* uses default type                */
        fat.fsFontUse = NULL; /* does not mix with graphics  */

        for (ii=0; fat.szFacename[ii] = pfm[i-1].szFacename[ii]; ii++);

        GpiCreateLogFont(hps,           /* presentation space               */
                         NULL,              /* does not use logical font name   */
                         (LONG) i,       /* local identifier                 */
                         &fat);             /* structure with font attributes   */

        GpiSetCharSet(hps, (LONG) i);    /* sets font for presentation space */
        GpiCharStringAt(hps, &ptl, (LONG) ii, fat.szFacename);
        ii = sprintf (buffer, " %d ", fat.lMatch);
        GpiCharString (hps, (LONG) ii, buffer);


        if (pfm[i-1].fsDefn & FM_DEFN_OUTLINE)
             GpiCharString (hps, 3L," !V");

        } /******************************/

    }

}

















/* at some point, add the feature whereby the allwnd
 *  screen is drawn into memory first, and then just blted
 *  to the terminal.
 * The following may be useful code. ??

{


/* structures used in creating the hdc
SIZEL        sizell = {0,0};
DEVOPENSTRUC dopl   = { NULL, "DISPLAY",
                       NULL, NULL, NULL, NULL, NULL, NULL, NULL};


static HDC  hdcMemory;
static HPS  hpsMemory = NULL;
static HBITMAP     hbm;
static BITMAPINFOHEADER bmih = { sizeof (BITMAPINFOHEADER),
                            400, 300,
                            1, 1};
BITMAPINFO  bmi;
CHAR    buffer[12000];

    hdcMemory = DevOpenDC (hab, OD_MEMORY, "*", 4L, &dopl, NULL);
    hpsMemory = GpiCreatePS (hab, hdcMemory, &sizell, PU_PELS |
                                    GPIT_MICRO | GPIA_ASSOC);

    hbm = GpiCreateBitmap (hpsMemory, &bmih, CBM_INIT, buffer, &bmi);
    QueryAllFonts(hpsMemory);
}

/*
*/
