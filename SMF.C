/*  Smokey Magic Fixer.  A program to examine the os/2-PM
 *   font system.  Intended to provide dynamic manipulation of
 *   the fattrs structure and the Charbundle, and examine their
 *   effect on the fonts which appear.
 *
 *  Steve Firebaugh.  May 1990.  Microsoft, PM Product Support
 *
 */
/*
    smf design framework.


    1.  All communication between windows is done via
       the main frame.

    2.  The main frame has access to all hwnds, and may use
       as it sees fit.

    3.  WM_ user message naming conventions
            the second string indicates which window sent from
            the third string indicates which window sent to
            the last string indicates the action

    4.  Windows with a similar appearance have the same window
       procedure.  e.g. metric windows 1 & 2, fattr & charbundle dialogs

    5.  Rectangles fly on screen to mirror message traffic
*/



#define INCL_WIN
#define INCL_GPI
#include <os2.h>
#include <stdio.h>
#include "smf.h"

/* local function prototypes */
VOID    FlyWinWin (HWND, HWND, SHORT);
VOID    GrowRec (HPS, PRECTL, PRECTL, SHORT);



int main (void)
    {
    static CHAR  szClientClass [] = "base";
    static ULONG flFrameFlags = FCF_TITLEBAR      | FCF_SYSMENU    |
                                FCF_BORDER        |
                                FCF_TASKLIST;

    HMQ    hmq;
    QMSG   qmsg;
    HPOINTER   hptrDefault, hptrWait;

    hab = WinInitialize (0);
    hmq = WinCreateMsgQueue (hab, 0);


    /* get the system pointer */
    hptrDefault = WinQuerySysPointer(HWND_DESKTOP, SPTR_ARROW, FALSE);
    hptrWait = WinQuerySysPointer(HWND_DESKTOP, SPTR_WAIT, FALSE);

    WinSetPointer(HWND_DESKTOP, hptrWait);





    WinRegisterClass (
                 hab,             // Anchor Block handle
                 szClientClass,   // Name of class being registered
                 ClientWndProc,   // Window procedure for class
                 CS_SIZEREDRAW,   // Class style
                 0);              // Extra bytes to reserve

    hwndFrame = WinCreateStdWindow (
                 HWND_DESKTOP,    // Parent of Window handle
                 NULL, // WS_VISIBLE,      // Style of frame window
                 &flFrameFlags,   // Pointer to control data
                 szClientClass,   // Client window class name
                 NULL,            // Title bar text
                 NULL, // Style of Client Window
                 NULL,            // Module of handle for resources
                 ID_RESOURCE,               // ID of resources
                 &hwndMain);    // Pointer to client window handle

    WinSetWindowText (hwndFrame, "smf: Logical font Examiner");
    WinSetWindowPos (hwndFrame, HWND_TOP, 0,0,0,0,
                    SWP_MAXIMIZE | SWP_SIZE | SWP_SHOW);

    FrameInit(hwndMain);  // works better to have this init after visible

    /* restore the system pointer */
    WinSetPointer(HWND_DESKTOP, hptrDefault);


    while (WinGetMsg (hab, &qmsg, NULL, 0, 0))
      WinDispatchMsg (hab, &qmsg);

    WinDestroyWindow (hwndFrame);
    WinDestroyMsgQueue (hmq);
    WinTerminate (hab);
    return 0;
}



/* frame init sets up the main window.
 *  This includes initializing the assorted children windows.
 */
VOID FrameInit( HWND hwndClient)
   {
        hwndSelect = WinCreateWindow (hwndClient, WC_BUTTON, "Select Font",
                        WS_VISIBLE,
                        10, 300, 180, 35,
                        hwndClient, HWND_TOP,
                        ID_SF_SELECT, NULL, NULL);

        hwndCreate = WinCreateWindow (hwndClient, WC_BUTTON, "Create Logical Font",
                        WS_VISIBLE,
                        10, 350, 180, 35,
                        hwndClient, HWND_TOP,
                        ID_SF_CREATE, NULL, NULL);

        hwndSetCHB = WinCreateWindow (hwndClient, WC_BUTTON, "Set Char Bundle",
                        WS_VISIBLE,
                        10, 400, 180, 35,
                        hwndClient, HWND_TOP,
                        ID_SF_SETCHB, NULL, NULL);

        /* create & initialize the other children windows */
        TestInit(hwndClient);
        AllInit (hwndClient);
        MetInit (hwndClient);

        /* get the fontmetrics from the test window to the metric window */
        {
        FONTMETRICS  *pfm;
        pfm = (PFONTMETRICS) WinSendMsg (hwndTest, WM_MAIN_TEST_GETFM, 0L, 0L);
        WinPostMsg (hMet1, WM_MAIN_MET_CHANGE, (MPARAM) pfm, 0L);
        }

        hwndFattrs = WinLoadDlg (hwndClient, hwndClient, FattrWndProc,
                                 NULL, 256, NULL);
        hwndCharbundle = WinLoadDlg (hwndClient, hwndClient, FattrWndProc,
                                 NULL, 257, NULL);

        FattrInit(hwndFattrs);
        FattrInit(hwndCharbundle);

        /* fill up the char bundle window with the values from test chb */
        {HPS  hps;
        hps = (HPS) WinSendMsg (hwndTest, WM_MAIN_TEST_GETPS, 0L, 0L);
        WinPostMsg (hwndCharbundle, WM_MAIN_CHB_FILLCHB, (MPARAM) hps, 0L);
        }

   }


/********************** ClientWndProc **************************/
MRESULT EXPENTRY ClientWndProc (HWND hwnd, USHORT msg, MPARAM mp1, MPARAM mp2)
   {
   HPS    hps;
   RECTL  rcl;
   static SHORT  color;

   switch (msg)
     {
     case WM_COMMAND:
        switch (COMMANDMSG(&msg)->cmd) {

            case ID_SF_SELECT: {

                WinSetWindowPos (hwndAll, HWND_TOP, 0,0,0,0,
                      SWP_ZORDER | SWP_SHOW);
                FlyWinWin (hwndSelect, hwndAll, FWW_LONG);

              //  WinShowWindow (hwndAll, TRUE);
              //  WinSetFocus (HWND_DESKTOP, hwndAll);
                } break;

            case ID_SF_CREATE: {
                HPS hps;

                /* Tell fattrs window to down load values and fill
                 * fattrs.  It is a global variable.
                 */
                FlyWinWin (hwndCreate, hwndFattrs, FWW_MEDIUM);
                if (WinSendMsg (hwndFattrs, WM_MAIN_FAT_READ,
                                (MPARAM) &fattrs, 0L)) {
                    /* we got the fattrs o.k. */
                    FlyWinWin (hwndFattrs, hwndTest, FWW_MEDIUM);

                    hps = WinSendMsg (hwndTest, WM_MAIN_TEST_GETPS, 0L, 0L);

                    /* create a logical font in the test window */
                    GpiSetCharSet (hps, 0L);
                    GpiDeleteSetId (hps, OUR_LCID);
                    GpiCreateLogFont(hps, NULL, OUR_LCID, &fattrs);
                    GpiSetCharSet( hps, OUR_LCID );
                    WinInvalidateRect (hwndTest, NULL, FALSE);  // repaint test

                    FlyWinWin (hwndTest, hMet1, FWW_SHORT);
                    /* reset the fontmetrics in the second metrics window */
                    {
                    FONTMETRICS  *pfm;
                    pfm = (PFONTMETRICS) WinSendMsg (hwndTest, WM_MAIN_TEST_GETFM, 0L, 0L);
                    WinPostMsg (hMet1, WM_MAIN_MET_CHANGE, (MPARAM) pfm, 0L);
                    }
                }
            } break;


            case ID_SF_SETCHB: {
                HPS hps;
                FlyWinWin (hwndSetCHB, hwndCharbundle, FWW_MEDIUM);
                if (WinSendMsg (hwndCharbundle, WM_MAIN_CHB_READ,
                                (MPARAM) &charbundle, 0L)) {
                    hps = WinSendMsg (hwndTest, WM_MAIN_TEST_GETPS, 0L, 0L);
                    GpiSetAttrs (hps, PRIM_CHAR, (LONG) (flAttrsMask & ~CBB_SET),
                               0L, &charbundle);

                    FlyWinWin (hwndCharbundle, hwndTest, FWW_MEDIUM);
                    WinInvalidateRect (hwndTest, NULL, FALSE);  // repaint test

                    FlyWinWin (hwndTest, hMet1, FWW_SHORT);
                    /* reset the fontmetrics in the second metrics window */
                    {
                    FONTMETRICS  *pfm;
                    pfm = (PFONTMETRICS) WinSendMsg (hwndTest, WM_MAIN_TEST_GETFM, 0L, 0L);
                    WinPostMsg (hMet1, WM_MAIN_MET_CHANGE, (MPARAM) pfm, 0L);
                    }
                }

            } break;

        } break;

    /* the user has selected the font.  mp1 contains PFONTMETRICS
     *  pass information along to the first metrics window
     */
    case WM_ALL_MAIN_FONTSELECTED: {
        HWND        hwndlMatchEF;

        WinSendMsg (hMet0, WM_MAIN_MET_CHANGE, mp1, 0L);
        FlyWinWin (hwndMain, hMet0, FWW_LONG);
        WinSendMsg (hwndFattrs, WM_MAIN_FAT_CHANGE, mp1, 0L);

        hwndlMatchEF = WinWindowFromID (hwndFattrs, F_lMatch);
        FlyWinWin (hMet0, hwndlMatchEF, FWW_MEDIUM);

    } break;

    case WM_PAINT:
        hps = WinBeginPaint (hwnd, NULL, &rcl);
        WinFillRect (hps, &rcl, (COLOR) (++color % 15));
        WinEndPaint (hps);
        return 0;
     }
   return WinDefWindowProc (hwnd, msg, mp1, mp2);
   }








/* structures used in creating the hdc */
SIZEL        sizel = {0,0};
DEVOPENSTRUC dop   = { NULL, "DISPLAY",
                       NULL, NULL, NULL, NULL, NULL, NULL, NULL};


/* handles & structures for the test window which recieves all the abuse. */
static HDC  hdcTest;
static HPS  hpsTest = NULL;
static FONTMETRICS  fmTest;

VOID    TestInit(HWND hwnd)
{

    hwndTest = WinCreateWindow (hwnd, WC_FRAME, NULL,
                    WS_VISIBLE | FS_BORDER,
                    445, 260, WIDTH, 190,
                    hwnd, HWND_TOP,
                    ID_SF_TEST, NULL, NULL);
    WinSubclassWindow (hwndTest, TestWndProc);
    // WinInvalidateRect (hwndTest, NULL, FALSE);

    // hdcTest = DevOpenDC (hab, OD_MEMORY, "*", 4L, &dop, NULL);
    hdcTest = WinOpenWindowDC (hwndTest);
    hpsTest = GpiCreatePS (hab, hdcTest, &sizel, PU_PELS |
                                    GPIT_MICRO | GPIA_ASSOC);

    //GpiSetCharMode(hpsTest, CM_MODE3);
}


MRESULT EXPENTRY TestWndProc (HWND hwnd, USHORT msg, MPARAM mp1, MPARAM mp2)
{
    switch (msg)
    {
    /*  HACK ?? get this right !!  memory bugs here ??
    case WM_DESTROY: {
        GpiAssociate (hpsTest, NULL);
        GpiDestroyPS (hpsTest);
        DevCloseDC (hdcTest);
        } break;
    */

    case WM_MAIN_TEST_GETPS:
        return (MRESULT) hpsTest;

    case WM_MAIN_TEST_GETFM:
        GpiQueryFontMetrics (hpsTest, (LONG) sizeof (FONTMETRICS), &fmTest);
        return (MRESULT) &fmTest;



    case WM_PAINT: {
        HPS  hps;
        RECTL r;
        POINTL  p;
        p.x = p.y = 40L;

        hps = WinBeginPaint (hwnd, hpsTest, &r);
        WinFillRect (hps, &r, CLR_PALEGRAY);
        GpiCharStringAt (hps, &p, 5L, "hello");
        WinEndPaint (hps);
    }break;

    }
    return WinDefWindowProc (hwnd, msg, mp1, mp2);
}














typedef struct {
    int     n;
    RECTL   rs[FWW_MODSIZE];
} RECLIST;









/*  implementation of flying rectangles. */


VOID FAR DrawRec (HPS hps, PRECTL pr)
{
    POINTL  p;

    p.x = pr->xRight;
    p.y = pr->yTop;

    GpiMove (hps,(PPOINTL) pr);
    GpiBox (hps, DRO_OUTLINE, &p, 0L, 0L);

    return;
}


VOID GrowRec (hps, pr1, pr2, steps)
HPS     hps;
PRECTL  pr1, pr2;
SHORT  steps;
{
    RECTL   r;
    LONG    delxLeft, delxRight, delyTop, delyBottom;
    ULONG  i;
    LONG   fraction1000;
    RECLIST eraser;
    USHORT  modsize;
    LONG    oldMix, oldColor;

    modsize = FWW_MODSIZE;
    oldMix = GpiQueryMix(hps); oldColor = GpiQueryColor (hps);


    delxLeft = (pr2->xLeft - pr1->xLeft);
    delxRight = (pr2->xRight - pr1->xRight);
    delyTop = (pr2->yTop - pr1->yTop);
    delyBottom = (pr2->yBottom - pr1->yBottom);


    r.xLeft = pr1->xLeft;
    r.xRight = pr1->xRight;
    r.yBottom = pr1->yBottom;
    r.yTop = pr1->yTop;


    GpiSetMix (hps, FM_INVERT);  GpiSetColor(hps, CLR_TRUE);

    for (i = 0; i < (steps+ modsize); i++) {

        if (i <= steps){
            DrawRec (hps, &r);
        }
        if (i>= (modsize-1)) {
            DrawRec (hps, &eraser.rs[ (i+1) % modsize]);
        }

        eraser.rs[ i% modsize] = r;

        fraction1000 = (1000 * i) / (LONG)  steps;
        r.xLeft = pr1->xLeft + (int)( fraction1000 * delxLeft/ 1000);
        r.xRight = pr1->xRight + (int)( fraction1000 * delxRight/ 1000);
        r.yTop = pr1->yTop + (int)( fraction1000 * delyTop/ 1000);
        r.yBottom = pr1->yBottom + (int)( fraction1000 *delyBottom/ 1000);
    }

    GpiSetMix (hps, oldMix); GpiSetColor (hps, oldColor);
}


VOID    FlyWinWin (HWND hwnd1, HWND hwnd2, SHORT nsteps)
{
    HPS    hps;
    RECTL  r1, r2;

    WinQueryWindowRect (hwnd1, &r1);
    WinMapWindowPoints (hwnd1, hwndFrame,(PPOINTL)&r1,2);
    WinQueryWindowRect (hwnd2, &r2);
    WinMapWindowPoints (hwnd2, hwndFrame,(PPOINTL)&r2,2);

    hps = WinGetPS (hwndFrame);

    GrowRec (hps, &r1, &r2, nsteps);

    WinReleasePS (hps);
}
