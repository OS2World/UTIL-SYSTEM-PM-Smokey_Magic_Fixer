/* code for the fattr dialog box,
 *  and the char bundle dialog box.
 *
 * Both an Init procedure, and a window procedure.
 */



#define INCL_WIN
#define INCL_GPI
#include <os2.h>
#include <stdio.h>
#include <stdlib.h>

#include "smf.h"

VOID    FattrInit(HWND hwnd)
{
     /* code to enumerate the entry fields.  (FATTRS window)
      * for each one, set the presparms for a small font.
      */
    HENUM  heList;
    HWND   hwndi;
    CHAR    buffer[100];

    heList = WinBeginEnumWindows (hwnd);

    while (hwndi = WinGetNextWindow (heList) ) {
        WinQueryClassName (hwndi, 100, buffer);
        if ( (SHORT) atoi (&buffer[1]) == (SHORT) WC_ENTRYFIELD )
            WinSetPresParam (hwndi, PP_FONTNAMESIZE,
                (ULONG)sizeof (szCourier), szCourier);
    }

    WinEndEnumWindows (heList);
}


MRESULT EXPENTRY FattrWndProc (HWND hwnd, USHORT msg, MPARAM mp1, MPARAM mp2)
{
    switch (msg)
    {

    /* fontmetrics changed, selectively update fattrs window */
    case WM_MAIN_FAT_CHANGE: {
        FONTMETRICS *pfm;

        pfm = (PFONTMETRICS) mp1;
        WinSetDlgItemShort (hwnd, F_lMatch, (USHORT) pfm->lMatch, FALSE);
        WinSetDlgItemText (hwnd, F_szFacename, (PSZ) pfm->szFacename);

    } break;

    case WM_MAIN_CHB_FILLCHB:
        if (hwnd == hwndCharbundle) {
            HPS  hpsTest;

            hpsTest = (HPS) mp1;

            GpiQueryAttrs (hpsTest, PRIM_CHAR, flAttrsMask, &charbundle);

            WinSetDlgItemShort (hwnd, CB_lColor, (USHORT) charbundle.lColor, FALSE);
            WinSetDlgItemShort (hwnd, CB_lBackColor, (USHORT) charbundle.lBackColor, FALSE);
            WinSetDlgItemShort (hwnd, CB_usMixMode, (USHORT) charbundle.usMixMode, FALSE);
            WinSetDlgItemShort (hwnd, CB_usBackMixMode, (USHORT) charbundle.usBackMixMode, FALSE);
            WinSetDlgItemShort (hwnd, CB_usSet, (USHORT) charbundle.usSet, FALSE);
            WinSetDlgItemShort (hwnd, CB_usPrecision, (USHORT) charbundle.usPrecision, FALSE);

            WinSetDlgItemShort (hwnd, CB_sizfxCell_X,
                                (USHORT) FIXEDINT(charbundle.sizfxCell.cx), FALSE);
            WinSetDlgItemShort (hwnd, CB_sizfxCell_Y,
                                (USHORT) FIXEDINT(charbundle.sizfxCell.cy), FALSE);

            WinSetDlgItemShort (hwnd, CB_ptlAngle_X,
                                (USHORT) charbundle.ptlAngle.x, FALSE);
            WinSetDlgItemShort (hwnd, CB_ptlAngle_Y,
                                (USHORT) charbundle.ptlAngle.y, FALSE);

            WinSetDlgItemShort (hwnd, CB_ptlShear_X,
                                (USHORT) charbundle.ptlShear.x, FALSE);
            WinSetDlgItemShort (hwnd, CB_ptlShear_Y,
                                (USHORT) charbundle.ptlShear.y, FALSE);

            WinSetDlgItemShort (hwnd, CB_usDirection, (USHORT) charbundle.usDirection, FALSE);
        }
    break;

    case WM_MAIN_CHB_READ: {
        USHORT      usTemp;
        CHARBUNDLE  *pchb;
        pchb = (CHARBUNDLE*) mp1;

        /* first bring the window to top & set focus for user */
        WinSetWindowPos (hwnd, HWND_TOP, 0,0,0,0,
              SWP_ZORDER | SWP_SHOW);
        WinSetFocus (HWND_DESKTOP, hwnd);

        /* now pull each of the values off screen & put in structure */
        if (!WinQueryDlgItemShort (hwnd, CB_lColor, &usTemp, FALSE)) {
            WinMessageBox (hwndMain, hwndMain, "lColor must be a number",
                "CharBundle warning", 0, MB_ICONEXCLAMATION);
            return (MRESULT) FALSE;
        } else pchb->lColor = (LONG) usTemp;

        if (!WinQueryDlgItemShort (hwnd, CB_lBackColor, &usTemp, FALSE)) {
            WinMessageBox (hwndMain, hwndMain, "lBAckColor must be a number",
                "CharBundle warning", 0, MB_ICONEXCLAMATION);
            return (MRESULT) FALSE;
        } else pchb->lBackColor = (LONG) usTemp;

        if (!WinQueryDlgItemShort (hwnd, CB_usMixMode, &usTemp, FALSE)) {
            WinMessageBox (hwndMain, hwndMain, "usMixMode must be a number",
                "CharBundle warning", 0, MB_ICONEXCLAMATION);
            return (MRESULT) FALSE;
        } else pchb->usMixMode = usTemp;

        if (!WinQueryDlgItemShort (hwnd, CB_usBackMixMode, &usTemp, FALSE)) {
            WinMessageBox (hwndMain, hwndMain, "usBackMixMode must be a number",
                "CharBundle warning", 0, MB_ICONEXCLAMATION);
            return (MRESULT) FALSE;
        } else pchb->usBackMixMode = usTemp;

        if (!WinQueryDlgItemShort (hwnd, CB_usSet, &usTemp, FALSE)) {
            WinMessageBox (hwndMain, hwndMain, "usSet must be a number",
                "CharBundle warning", 0, MB_ICONEXCLAMATION);
            return (MRESULT) FALSE;
        } else pchb->usSet = usTemp;

        if (!WinQueryDlgItemShort (hwnd, CB_usPrecision, &usTemp, FALSE)) {
            WinMessageBox (hwndMain, hwndMain, "usPrecision must be a number",
                "CharBundle warning", 0, MB_ICONEXCLAMATION);
            return (MRESULT) FALSE;
        } else pchb->usPrecision = usTemp;

        if (!WinQueryDlgItemShort (hwnd, CB_sizfxCell_X, &usTemp, TRUE)) {
            WinMessageBox (hwndMain, hwndMain, "sizfxCell.x must be a number",
                "CharBundle warning", 0, MB_ICONEXCLAMATION);
            return (MRESULT) FALSE;
        } else pchb->sizfxCell.cx = MAKEFIXED (usTemp, 0);

        if (!WinQueryDlgItemShort (hwnd, CB_sizfxCell_Y, &usTemp, TRUE)) {
            WinMessageBox (hwndMain, hwndMain, "sizfxCell.y must be a number",
                "CharBundle warning", 0, MB_ICONEXCLAMATION);
            return (MRESULT) FALSE;
        } else pchb->sizfxCell.cy = MAKEFIXED (usTemp, 0);

        if (!WinQueryDlgItemShort (hwnd, CB_ptlAngle_X, &usTemp, TRUE)) {
            WinMessageBox (hwndMain, hwndMain, "ptlAngle.x must be a number",
                "CharBundle warning", 0, MB_ICONEXCLAMATION);
            return (MRESULT) FALSE;
        } else pchb->ptlAngle.x = (LONG)usTemp;

        if (!WinQueryDlgItemShort (hwnd, CB_ptlAngle_Y, &usTemp, TRUE)) {
            WinMessageBox (hwndMain, hwndMain, "ptlAngle.y must be a number",
                "CharBundle warning", 0, MB_ICONEXCLAMATION);
            return (MRESULT) FALSE;
        } else pchb->ptlAngle.y = (LONG)usTemp;

        if (!WinQueryDlgItemShort (hwnd, CB_ptlShear_X, &usTemp, TRUE)) {
            WinMessageBox (hwndMain, hwndMain, "ptlShear.x must be a number",
                "CharBundle warning", 0, MB_ICONEXCLAMATION);
            return (MRESULT) FALSE;
        } else pchb->ptlShear.x = (LONG)usTemp;

        if (!WinQueryDlgItemShort (hwnd, CB_ptlShear_Y, &usTemp, TRUE)) {
            WinMessageBox (hwndMain, hwndMain, "ptlShear.y must be a number",
                "CharBundle warning", 0, MB_ICONEXCLAMATION);
            return (MRESULT) FALSE;
        } else pchb->ptlShear.y = (LONG)usTemp;

        if (!WinQueryDlgItemShort (hwnd, CB_usDirection, &usTemp, TRUE)) {
            WinMessageBox (hwndMain, hwndMain, "usDirection must be a number",
                "CharBundle warning", 0, MB_ICONEXCLAMATION);
            return (MRESULT) FALSE;
        } else pchb->usDirection = usTemp;

        return TRUE;
    } break;


    /* pull out the values and fill up a fattr structure.
     *  mp1 will be a pointer to the structure to be filled
     */
    case WM_MAIN_FAT_READ: {
        SHORT       sTemp;
        FATTRS      *pFattrs;

        /* first bring the fattr window to top & set focus for user */
        WinSetWindowPos (hwndFattrs, HWND_TOP, 0,0,0,0,
              SWP_ZORDER | SWP_SHOW);
        WinSetFocus (HWND_DESKTOP, hwndFattrs);


        pFattrs = (PFATTRS) mp1;

        pFattrs->usRecordLength = sizeof (FATTRS);
        WinSetDlgItemShort (hwndFattrs, F_usRecordLength,
                (USHORT) (sizeof (FATTRS)), FALSE);

        if (!WinQueryDlgItemShort (hwndFattrs, F_fsSelection,
                                   &(pFattrs->fsSelection), FALSE)) {
            WinMessageBox (hwndMain, hwndMain, "fsSelection must be a number",
                "FATTRS warning", 0, MB_ICONEXCLAMATION);
            return (MRESULT) FALSE;
        }

        if (!WinQueryDlgItemShort (hwndFattrs, F_lMatch,
                                   &sTemp, FALSE)) {
            WinMessageBox (hwndMain, hwndMain, "lMatch must be a number",
                "FATTRS warning", 0, MB_ICONEXCLAMATION);
            return (MRESULT) FALSE;
        } else
            pFattrs->lMatch = (LONG) sTemp;

        if (!WinQueryDlgItemShort (hwndFattrs, F_idRegistry,
                                   &(pFattrs->idRegistry), FALSE)) {
            WinMessageBox (hwndMain, hwndMain, "idRegistry must be a number",
                "FATTRS warning", 0, MB_ICONEXCLAMATION);
            return (MRESULT) FALSE;
        }

        if (!WinQueryDlgItemShort (hwndFattrs, F_usCodePage,
                                   &(pFattrs->usCodePage), FALSE)) {
            WinMessageBox (hwndMain, hwndMain, "usCodePage must be a number",
                "FATTRS warning", 0, MB_ICONEXCLAMATION);
            return (MRESULT) FALSE;
        }

        if (!WinQueryDlgItemText (hwndFattrs, F_szFacename,
                                   FACESIZE, (pFattrs->szFacename))) {
            WinMessageBox (hwndMain, hwndMain, "unable to read face name",
                "FATTRS warning", 0, MB_ICONEXCLAMATION);
            return (MRESULT) FALSE;
        }

        if (!WinQueryDlgItemShort (hwndFattrs, F_lMaxBaselineExt,
                                   &sTemp, FALSE)) {
            WinMessageBox (hwndMain, hwndMain, "lMaxBaselineExt must be a number",
                "FATTRS warning", 0, MB_ICONEXCLAMATION);
            return (MRESULT) FALSE;
        } else
            pFattrs->lMaxBaselineExt = (LONG) sTemp;

        if (!WinQueryDlgItemShort (hwndFattrs, F_lAveCharWidth,
                                   &sTemp, FALSE)) {
            WinMessageBox (hwndMain, hwndMain, "lAveCharWidth must be a number",
                "FATTRS warning", 0, MB_ICONEXCLAMATION);
            return (MRESULT) FALSE;
        } else
            pFattrs->lAveCharWidth = (LONG) sTemp;

        if (!WinQueryDlgItemShort (hwndFattrs, F_fsType,
                                   &(pFattrs->fsType), FALSE)) {
            WinMessageBox (hwndMain, hwndMain, "fsType must be a number",
                "FATTRS warning", 0, MB_ICONEXCLAMATION);
            return (MRESULT) FALSE;
        }

        if (!WinQueryDlgItemShort (hwndFattrs, F_fsFontUse,
                                   &(pFattrs->fsFontUse), FALSE)) {
            WinMessageBox (hwndMain, hwndMain, "fsFontUse must be a number",
                "FATTRS warning", 0, MB_ICONEXCLAMATION);
            return (MRESULT) FALSE;
        }


        return (MRESULT) TRUE;
    } break;




    }
    return WinDefDlgProc (hwnd, msg, mp1, mp2);
}
