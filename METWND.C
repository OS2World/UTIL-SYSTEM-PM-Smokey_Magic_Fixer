/* MetWnd.C:
 *  original code by Robert Hess for FONTWORK, 1988
 *  modified by Steve Firebaugh  4/1/90
 *
 *  This is code to support the two FONTMETRICS windows.
 *  A large amount of it is simply concerned with handling the
 *   painting correctly and dealing with the scroll bars.  Note
 *   that we are *not* using a list box here.
 */

#define INCL_WIN
#define INCL_GPI
#include <os2.h>
#include <stdlib.h>
#include <stdio.h>
#include "smf.h"

#define NUMLINES 49

  /* hack.  encode a 0,1 window identifier in window id
   *  i.e. one window i.d will be SF_UNIQUEID, the other
   *  will be SF_UNIQUEID + 1
   */
#define SF_UNIQUEID     57

/* formatting strings for the various lines displayed */
char szPrint[] = "%s = %s";
char  iPrint[] = "%s = %d";
char  aPrint[] = "%s = %ld (%d)";
char  xPrint[] = "%s = 0x%4.4X";
char  szTmsRmn[] = "8.Helv";
int   length;
char szText[80];

FONTMETRICS localFont;
PSZ dcData[9] = {NULL, (PSZ)"DISPLAY"};


metExtraBytes  meb[2];
HWND hMetWnd;
HWND hwndClient0, hwndClient1;


int cdecl MetInit (HWND hwnd)
{
    LONG ctlData;

    if (!WinRegisterClass (hab,
			   "FontMet",
			   MetWndProc,
			   CS_SYNCPAINT | CS_SIZEREDRAW,
                           0))
	return FALSE;

    ctlData = FCF_VERTSCROLL | FCF_BORDER ;

    hMet0 = WinCreateStdWindow (hwnd ,
                            NULL,
			    &ctlData,
			    "FontMet",
                            "Metrics a",
                            0L,
                            NULL,
                            SF_UNIQUEID + 0,
                            &hwndClient0);
    WinSetWindowPos (hMet0, HWND_TOP, XMET0, YMET0, CXMET, CYMET,
                     SWP_MOVE | SWP_SIZE | SWP_SHOW);

    hMet1 = WinCreateStdWindow (hwnd ,
                            NULL,
			    &ctlData,
			    "FontMet",
                            "Metrics b",
                            0L,
                            NULL,
                            SF_UNIQUEID + 1,
                            &hwndClient1);

    WinSetWindowPos (hMet1, HWND_TOP, XMET1, YMET1, CXMET, CYMET,
                     SWP_MOVE | SWP_SIZE | SWP_SHOW);


    return TRUE;
}


MRESULT EXPENTRY MetWndProc (HWND hwnd, USHORT msg, MPARAM mp1, MPARAM mp2)
{
     // CHAR          szBuffer [80] ;
     HPS           hPS;
     POINTL	   ptl, box[TXTBOX_COUNT], temp;
     LONG          actual;
     SHORT         sLine, sPaintBeg, sPaintEnd, sVscrollInc ;
     RECTL	   rclInvalid, rctl;
     BOOL	   update, forceScroll;
     metExtraBytes *pmeb;
     USHORT         id;

    /* the frame i.d. selects between 0 & 1 in the meb array  */
    id = WinQueryWindowUShort (WinQueryWindow (hwnd, QW_PARENT, FALSE),
                               QWS_ID) - SF_UNIQUEID;
    if (id != id % 2)  DosBeep (1000,1000);  /* just to be sure in [0,1] */
    pmeb = &(meb[id]);


    switch (msg) {

        case WM_CREATE: {
               SIZEL         sizel;

               pmeb->cxChar = 8;
               pmeb->cyChar = 14;
               pmeb->cyDesc = 4;
               pmeb->theFontSem = NULL;  /* no fontmetrics yet */

               pmeb->hwndVscroll = WinWindowFromID (
                                   WinQueryWindow (hwnd, QW_PARENT, FALSE),
				   FID_VERTSCROLL) ;

               pmeb->hFontDC = DevOpenDC (hab, OD_INFO,
                                          (PSZ)"*", 2L, dcData, NULL);
	       sizel.cx = sizel.cy = 0L;
               pmeb->hFontPS = GpiCreatePS (hab, pmeb->hFontDC, &sizel,
                         PU_PELS | GPIT_MICRO | GPIA_ASSOC);


               WinSetPresParam (hwnd, PP_FONTNAMESIZE,
                    (ULONG) sizeof (szTmsRmn), szTmsRmn);

               }
	       return 0 ;

	case WM_DESTROY:
            GpiDestroyPS (pmeb->hFontPS);
	    break;

	case WM_SIZE:
               pmeb->hwndVscroll = WinWindowFromID (
                                   WinQueryWindow (hwnd, QW_PARENT, FALSE),
				   FID_VERTSCROLL) ;

               pmeb->cxClient = SHORT1FROMMP (mp2) - 1;
               pmeb->cyClient = SHORT2FROMMP (mp2) - 1;


               pmeb->sVscrollMax = max (0, NUMLINES - pmeb->cyClient /
                                        pmeb->cyChar) ;
               pmeb->sVscrollPos = min (pmeb->sVscrollPos, pmeb->sVscrollMax) ;

               WinSendMsg (pmeb->hwndVscroll, SBM_SETSCROLLBAR,
                                        MPFROM2SHORT (pmeb->sVscrollPos, 0),
                                        MPFROM2SHORT (0, pmeb->sVscrollMax)) ;

               WinEnableWindow (pmeb->hwndVscroll,
                                        pmeb->sVscrollMax ? TRUE : FALSE) ;
               return 0 ;

          case WM_CALCVALIDRECTS:
               return MRFROMSHORT (CVR_ALIGNLEFT | CVR_ALIGNTOP) ;

	  case WM_VSCROLL:
		update = TRUE;
		forceScroll = FALSE;
		switch (SHORT2FROMMP (mp2))
                    {
                    case SB_LINEUP:
                         sVscrollInc = -1 ;
                         break ;

                    case SB_LINEDOWN:
                         sVscrollInc = 1 ;
                         break ;

                    case SB_PAGEUP:
                         sVscrollInc = min (-1, -pmeb->cyClient / pmeb->cyChar) ;
                         break ;

                    case SB_PAGEDOWN:
                         sVscrollInc = max (1, pmeb->cyClient / pmeb->cyChar) ;
                         break ;

		    case SB_SLIDERTRACK:
			 update = FALSE;
                         sVscrollInc = SHORT1FROMMP (mp2) - pmeb->sVscrollPos;
			 break ;

		    case SB_SLIDERPOSITION:
			forceScroll = TRUE;
			sVscrollInc = 0;
			break;

                    default:
                         sVscrollInc = 0 ;
                         break ;
                    }

               sVscrollInc = max (- pmeb->sVscrollPos, min (sVscrollInc,
                                    pmeb->sVscrollMax-pmeb->sVscrollPos));
	       if (sVscrollInc || forceScroll) {
                    pmeb->sVscrollPos += sVscrollInc ;
                    WinScrollWindow (hwnd, 0, pmeb->cyChar * sVscrollInc,
                                   NULL, NULL, NULL, NULL, SW_INVALIDATERGN) ;

                    if (update) WinSendMsg (pmeb->hwndVscroll, SBM_SETPOS,
                                        MPFROM2SHORT (pmeb->sVscrollPos, 0), NULL) ;
                    WinUpdateWindow (hwnd) ;
                    }
               return 0 ;

          case WM_MAIN_MET_CHANGE:
            pmeb->theFont = ( *(FONTMETRICS *)mp1);
            pmeb->theFontSem = TRUE;


            /* fall through to WM_PAINT */
            WinInvalidateRect (hwnd, NULL, FALSE);

          case WM_PAINT:

              hPS = WinBeginPaint (hwnd, NULL, &rclInvalid) ;
              GpiErase (hPS) ;
              if ( pmeb->theFontSem == NULL) {
                WinEndPaint(hPS);
                return 0L;
              }
              /* else we do have a fontmetrics structure ... */

              sPaintBeg = max (0, pmeb->sVscrollPos +
                             (pmeb->cyClient - (SHORT) rclInvalid.yTop) / pmeb->cyChar) ;
              sPaintEnd = min (NUMLINES, pmeb->sVscrollPos +
                             (pmeb->cyClient - (SHORT) rclInvalid.yBottom)
                                  / pmeb->cyChar + 1) ;

	       for (sLine = sPaintBeg ; sLine < sPaintEnd ; sLine++) {
                    ptl.x = pmeb->cxChar/2 ;
                    ptl.y = pmeb->cyClient - pmeb->cyChar *
                                      (sLine + 1 - pmeb->sVscrollPos) + pmeb->cyDesc ;

                    localFont = pmeb->theFont;

		    switch (sLine) {

case  0: length = sprintf (szText,szPrint, "szFamilyname", localFont.szFamilyname); break;
case  1: length = sprintf (szText,szPrint, "szFacename", localFont.szFacename); break;
case  2: length = sprintf (szText, iPrint, "idRegistry", localFont.idRegistry); break;
case  3: length = sprintf (szText, iPrint, "usCodePage", localFont.usCodePage); break;

case  4:

    GpiQueryTextBox (pmeb->hFontPS, 1L, (PSZ)"M ", TXTBOX_COUNT, box);
    actual = box[2].y - box[4].y;

    GpiResetBoundaryData (pmeb->hFontPS);
    temp.x = temp.y = 0;
    GpiMove (pmeb->hFontPS, &temp);
    GpiCharString (pmeb->hFontPS, 1L, (PSZ)"M");
    GpiQueryBoundaryData(pmeb->hFontPS, &rctl);
    actual = rctl.yTop - rctl.yBottom;

    GpiSetColor (hPS, CLR_GREEN);
    length = sprintf (szText, aPrint, "lEmHeight", localFont.lEmHeight, (int)actual);
    break;

case  5:

    GpiQueryTextBox (pmeb->hFontPS, 1L, (PSZ)"x ", TXTBOX_COUNT, box);
    actual = box[2].y - box[4].y;

    GpiResetBoundaryData (pmeb->hFontPS);
    temp.x = temp.y = 0;
    GpiMove (pmeb->hFontPS, &temp);
    GpiCharString (pmeb->hFontPS, 1L, (PSZ)"x");
    GpiQueryBoundaryData(pmeb->hFontPS, &rctl);
    actual = rctl.yTop - rctl.yBottom;

    GpiSetColor (hPS, CLR_BLUE);
    length = sprintf (szText, aPrint, "lXHeight", localFont.lXHeight, (int)actual);
    break;

case  6:

    GpiQueryTextBox (pmeb->hFontPS, 1L, (PSZ)"\177 ", TXTBOX_COUNT, box);
    actual = box[2].y - box[4].y;

    GpiResetBoundaryData (pmeb->hFontPS);
    temp.x = temp.y = 0;
    GpiMove (pmeb->hFontPS, &temp);
    GpiCharString (pmeb->hFontPS, 1L, (PSZ)"\177");
    GpiQueryBoundaryData(pmeb->hFontPS, &rctl);
    actual = rctl.yTop - rctl.yBottom;

    GpiSetColor (hPS, CLR_DARKGREEN);
    length = sprintf (szText, aPrint, "lMaxAscender", localFont.lMaxAscender, (int)actual);
    break;

case  7:

    GpiQueryTextBox (pmeb->hFontPS, 1L, (PSZ)"\177 ", TXTBOX_COUNT, box);
    actual = box[4].y - box[3].y;

    GpiResetBoundaryData (pmeb->hFontPS);
    temp.x = temp.y = 0;
    GpiMove (pmeb->hFontPS, &temp);
    GpiCharString (pmeb->hFontPS, 1L, (PSZ)"\177");
    GpiQueryBoundaryData(pmeb->hFontPS, &rctl);
    actual = rctl.yTop - rctl.yBottom;

    GpiSetColor (hPS, CLR_DARKGREEN);
    length = sprintf (szText, aPrint, "lMaxDescender", localFont.lMaxDescender, (int)actual);
    break;

case  8:

    GpiQueryTextBox (pmeb->hFontPS, 1L, (PSZ)"d ", TXTBOX_COUNT, box);
    actual = box[2].y - box[4].y;

    GpiResetBoundaryData (pmeb->hFontPS);
    temp.x = temp.y = 0;
    GpiMove (pmeb->hFontPS, &temp);
    GpiCharString (pmeb->hFontPS, 1L, (PSZ)"\177");
    GpiQueryBoundaryData(pmeb->hFontPS, &rctl);
    actual = rctl.yTop - rctl.yBottom;

    GpiSetColor (hPS, CLR_RED);
    length = sprintf (szText, aPrint, "lLowerCaseAscent", localFont.lLowerCaseAscent, (int)actual);
    break;

case  9:

    GpiQueryTextBox (pmeb->hFontPS, 1L, (PSZ)"g ", TXTBOX_COUNT, box);
    actual = box[4].y - box[3].y;

    GpiResetBoundaryData (pmeb->hFontPS);
    temp.x = temp.y = 0;
    GpiMove (pmeb->hFontPS, &temp);
    GpiCharString (pmeb->hFontPS, 1L, (PSZ)"g");
    GpiQueryBoundaryData(pmeb->hFontPS, &rctl);
    actual = rctl.yTop - rctl.yBottom;

    GpiSetColor (hPS, CLR_RED);
    length = sprintf (szText, aPrint, "lLowerCaseDescent", localFont.lLowerCaseDescent, (int)actual);
    break;

case 10: length = sprintf (szText, iPrint, "lInternalLeading", localFont.lInternalLeading); break;
case 11: length = sprintf (szText, iPrint, "lExternalLeading", localFont.lExternalLeading); break;
case 12: length = sprintf (szText, iPrint, "lAveCharWidth", localFont.lAveCharWidth); break;
case 13: length = sprintf (szText, iPrint, "lMaxCharInc", localFont.lMaxCharInc); break;
case 14: length = sprintf (szText, iPrint, "lEmInc", localFont.lEmInc); break;
case 15: length = sprintf (szText, iPrint, "lMaxBaselineExt", localFont.lMaxBaselineExt); break;
case 16: length = sprintf (szText, iPrint, "sCharSlope", localFont.sCharSlope); break;
case 17: length = sprintf (szText, iPrint, "sInLineDir", localFont.sInlineDir); break;
case 18: length = sprintf (szText, iPrint, "sCharRot", localFont.sCharRot); break;
case 19: length = sprintf (szText, iPrint, "usWeightClass", localFont.usWeightClass); break;
case 20: length = sprintf (szText, iPrint, "usWidthClass", localFont.usWidthClass); break;
case 21: length = sprintf (szText, iPrint, "sXDeviceRes", localFont.sXDeviceRes); break;
case 22: length = sprintf (szText, iPrint, "sYDeviceRes", localFont.sYDeviceRes); break;
case 23: length = sprintf (szText, iPrint, "sFirstChar", localFont.sFirstChar); break;
case 24: length = sprintf (szText, iPrint, "sLastChar", localFont.sLastChar); break;
case 25: length = sprintf (szText, iPrint, "sDefaultChar", localFont.sDefaultChar); break;
case 26: length = sprintf (szText, iPrint, "sBreakChar", localFont.sBreakChar); break;
case 27: length = sprintf (szText, iPrint, "sNominalPointSize", localFont.sNominalPointSize); break;
case 28: length = sprintf (szText, iPrint, "sMinimumPointSize", localFont.sMinimumPointSize); break;
case 29: length = sprintf (szText, iPrint, "sMaximumPointSize", localFont.sMaximumPointSize); break;
case 30: length = sprintf (szText, xPrint, "fsType", localFont.fsType); break;
case 31: length = sprintf (szText, xPrint, "fsDefn", localFont.fsDefn); break;
case 32: length = sprintf (szText, xPrint, "fsSelection", localFont.fsSelection); break;
case 33: length = sprintf (szText, xPrint, "fsCapabilities", localFont.fsCapabilities); break;
case 34: length = sprintf (szText, iPrint, "lSubscriptXSize", localFont.lSubscriptXSize); break;
case 35: length = sprintf (szText, iPrint, "lSubscriptYSize", localFont.lSubscriptYSize); break;
case 36: length = sprintf (szText, iPrint, "lSubscriptXOffset", localFont.lSubscriptXOffset); break;
case 37: length = sprintf (szText, iPrint, "lSubscriptYOffset", localFont.lSubscriptYOffset); break;
case 38: length = sprintf (szText, iPrint, "lSupersciptXSize", localFont.lSuperscriptXSize); break;
case 39: length = sprintf (szText, iPrint, "lSuperscriptYSize", localFont.lSuperscriptYSize); break;
case 40: length = sprintf (szText, iPrint, "lSuperscriptXOffset", localFont.lSuperscriptXOffset); break;
case 41: length = sprintf (szText, iPrint, "lSupscriptYOffset", localFont.lSuperscriptYOffset); break;
case 42: length = sprintf (szText, iPrint, "lUnderscoreSize", localFont.lUnderscoreSize); break;
case 43: length = sprintf (szText, iPrint, "lUnderscorePosition", localFont.lUnderscorePosition); break;
case 44: length = sprintf (szText, iPrint, "lStrikeoutSize", localFont.lStrikeoutSize); break;
case 45: length = sprintf (szText, iPrint, "lStrikeoutPosition", localFont.lStrikeoutPosition); break;
case 46: length = sprintf (szText, iPrint, "sKerningPairs", localFont.sKerningPairs); break;
case 47: length = sprintf (szText, iPrint, "sFamilyClass", localFont.sFamilyClass); break;
case 48: length = sprintf (szText, iPrint, "lMatch", localFont.lMatch);

		    }

                    GpiCharStringAt (hPS, &ptl, (LONG)length, szText);
		    GpiSetColor (hPS, CLR_BLACK);
	       }

	       WinEndPaint (hPS) ;
               return 0 ;
          }
     return WinDefWindowProc (hwnd, msg, mp1, mp2) ;
     }
