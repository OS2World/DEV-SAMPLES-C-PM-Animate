/*
 * animate1.c  --  PM animation using 16 individual bitmap frames
 *
 * Demonstrates how a series of similar bitmaps, combined with a PM timer,
 * create the illusion of animation.  Each WM_TIMER tick advances a frame
 * counter and blits the next bitmap to the screen, producing smooth motion.
 *
 * Technique summary:
 *   - 16 bitmap resources (frame1.bmp .. frame16.bmp) are loaded from the
 *     EXE at startup into a memory PS (an off-screen drawing surface).
 *   - A PM timer fires every 10 ms.  On each WM_TIMER, the next bitmap is
 *     selected into the memory PS and blitted to the window at 2x size.
 *   - The memory DC is created compatible with the screen DC so colour and
 *     resolution characteristics match exactly.
 *
 * GPI functions demonstrated:
 *   GpiLoadBitmap, GpiSetBitmap, GpiBitBlt, GpiQueryBitmapInfoHeader,
 *   GpiDeleteBitmap, GpiCreatePS, GpiDestroyPS, DevOpenDC, DevCloseDC
 *
 * Version : 1.02
 * License : 3-Clause BSD License
 * Authors : Martin Iturbide (2023), Kelvin R. Lawrence (1996)
 */

/*
 * INCL_32  - enables 32-bit API prototypes (IBM Toolkit legacy define;
 *            harmless with modern kLIBC / GCC headers).
 * INCL_GPI - GpiXxx / DevXxx graphics and device-context functions.
 * INCL_WIN - WinXxx window, message, and timer functions.
 * All defines must precede <os2.h>.
 */
#define INCL_32
#define INCL_GPI
#define INCL_WIN
#include <os2.h>
#include <memory.h>   /* memset */

#include "animate1.h"

#define TITLEBARTEXT (PCSZ)"animate1 - Using bitmaps and timers"
#define CLASSNAME    (PCSZ)"animate1"

/*
 * Global handles shared across main(), LoadBitmaps(), and DisplayNextBitmap().
 * Making them global avoids threading context through every call — the
 * conventional approach for simple single-window PM applications.
 */
HAB   hab;
HWND  hwndFrame;
HWND  hwndMenu;
HPS   hpsMemory;   /* off-screen memory presentation space */
HDC   hdcMemory;   /* memory device context                */
HDC   hdcScreen;   /* screen DC for our client window      */

#define NUM_BITMAPS 16

HBITMAP abmph[NUM_BITMAPS];   /* one handle per animation frame */
LONG    idBitmap = 0;         /* index of the frame to show next */

/* Forward declarations */
MRESULT EXPENTRY ClientWndProc(HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2);
VOID LoadBitmaps(HWND hwnd);
VOID DisplayNextBitmap(HWND hwnd, LONG idBitmap);


int main(VOID)
{
    HMQ  hmq;
    HWND hwndClient;
    QMSG qmsg;

    /*
     * FCF_* flags select the standard frame controls:
     *   FCF_BORDER / FCF_SIZEBORDER - resizable window border
     *   FCF_SHELLPOSITION           - let PM choose initial size and position
     *   FCF_TASKLIST                - appear in the Window List (Alt+Esc)
     *   FCF_TITLEBAR / FCF_SYSMENU  - title bar and system menu icon
     *   FCF_MINMAX                  - minimize / maximize buttons
     *   FCF_MENU                    - load menu from resource ID_PROG
     */
    ULONG flCreateFlags = FCF_BORDER     | FCF_SHELLPOSITION |
                          FCF_TASKLIST   | FCF_TITLEBAR      | FCF_SYSMENU |
                          FCF_SIZEBORDER | FCF_MINMAX        | FCF_MENU;

    /*
     * PM startup sequence:
     *   1. WinInitialize      - connect this thread to PM (returns the HAB)
     *   2. WinCreateMsgQueue  - create the message queue for this thread
     *   3. WinRegisterClass   - register our client window class
     *   4. WinCreateStdWindow - create the frame + client window pair
     *   5. WinShowWindow      - make the frame visible
     *   6. WinStartTimer      - start the animation timer (10 ms)
     *   7. LoadBitmaps        - create memory DC/PS and load all 16 frames
     *   8. Message loop       - pump messages until WM_QUIT
     *   9. Cleanup            - release GPI resources, destroy window/queue
     */
    hab = WinInitialize(0);
    hmq = WinCreateMsgQueue(hab, 0);

    /*
     * CS_SIZEREDRAW causes the client to receive WM_PAINT on every resize.
     * The extra-bytes value (256) reserves window-word space in the window
     * structure; 0 would also suffice for this application.
     */
    WinRegisterClass(hab, CLASSNAME, (PFNWP)ClientWndProc,
                     (ULONG)CS_SIZEREDRAW, (USHORT)256);

    /*
     * WinCreateStdWindow creates a frame window and a client window together.
     * Return value  = frame window handle (for WinDestroyWindow).
     * Last argument = receives the client window handle (our WndProc target).
     */
    hwndFrame = WinCreateStdWindow(HWND_DESKTOP, 0UL, &flCreateFlags,
                                   CLASSNAME, TITLEBARTEXT, WS_VISIBLE,
                                   (HMODULE)0, ID_PROG, &hwndClient);

    hwndMenu = WinWindowFromID(hwndFrame, FID_MENU);

    WinShowWindow(hwndFrame, TRUE);

    /*
     * WinStartTimer asks PM to post WM_TIMER to hwndClient every 10 ms.
     * The timer starts before LoadBitmaps so the message loop can begin
     * immediately; a WM_TIMER that arrives before bitmaps are loaded is
     * harmless because abmph[] is zero-initialised (globals).
     */
    WinStartTimer(hab, hwndClient, ID_TIMER, 10);

    LoadBitmaps(hwndClient);

    while (WinGetMsg(hab, &qmsg, (HWND)0, 0, 0))
        WinDispatchMsg(hab, &qmsg);

    /*
     * Cleanup in reverse creation order:
     *   deselect bitmap → delete all bitmaps → destroy PS → close DC.
     */
    if (hpsMemory) {
        LONG i;
        GpiSetBitmap(hpsMemory, 0);
        for (i = 0; i < NUM_BITMAPS; i++)
            GpiDeleteBitmap(abmph[i]);
        GpiDestroyPS(hpsMemory);
    }
    if (hdcMemory)
        DevCloseDC(hdcMemory);
    if (hwndFrame) {
        WinDestroyWindow(hwndFrame);
        WinDestroyMsgQueue(hmq);
        WinTerminate(hab);
    }
    return 0;
}


MRESULT EXPENTRY ClientWndProc(HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2)
{
    switch (msg)
    {
        case WM_ACTIVATE:
            /*
             * Force a full client repaint when this window gains focus.
             * SHORT1FROMMP(mp1) is TRUE on activation, FALSE on deactivation.
             */
            if (SHORT1FROMMP(mp1))
                WinInvalidateRect(hwnd, 0L, FALSE);
            break;

        case WM_COMMAND:
            switch (SHORT1FROMMP(mp1))
            {
                case IDM_EXIT:
                    WinPostMsg(hwnd, WM_CLOSE, (MPARAM)0L, (MPARAM)0L);
                    break;
            }
            break;

        case WM_PAINT:
        {
            /*
             * Clear the client area to black on every WM_PAINT.
             * Animation frames are drawn on WM_TIMER, not here — WM_PAINT
             * only handles expose / resize repaints.
             */
            HPS   hpsPaint;
            RECTL rectl;

            hpsPaint = WinBeginPaint(hwnd, (HPS)0, &rectl);
            WinQueryWindowRect(hwnd, &rectl);
            WinFillRect(hpsPaint, &rectl, CLR_BLACK);
            WinEndPaint(hpsPaint);
            break;
        }

        case WM_TIMER:
            /*
             * WM_TIMER fires every 10 ms (started in main with WinStartTimer).
             * Display the current frame, then advance and wrap the counter.
             */
            DisplayNextBitmap(hwnd, idBitmap);
            idBitmap++;
            if (idBitmap >= NUM_BITMAPS)
                idBitmap = 0;
            break;

        default:
            return WinDefWindowProc(hwnd, msg, mp1, mp2);
    }
    return 0;
}


/*
 * LoadBitmaps() - create the memory DC/PS and load all 16 frame bitmaps.
 *
 * The memory DC is opened compatible with the window's screen DC so that
 * colour depth and display format match, ensuring correct blit results.
 */
VOID LoadBitmaps(HWND hwnd)
{
    SIZEL        sizl;
    HAB          hab;    /* local HAB — retrieved from the window's thread */
    LONG         i;
    DEVOPENSTRUC dev;

    /*
     * WinQueryAnchorBlock returns the HAB for the thread that owns hwnd.
     * This local declaration intentionally shadows the global `hab`;
     * using the window's own anchor block is safer if this function were
     * ever called from a secondary thread.
     */
    hab = WinQueryAnchorBlock(hwnd);

    /*
     * Obtain the screen DC for this window.
     * WinQueryWindowDC returns a cached DC if one already exists;
     * WinOpenWindowDC creates and caches one if not.
     */
    if (!(hdcScreen = WinQueryWindowDC(hwnd)))
        hdcScreen = WinOpenWindowDC(hwnd);

    /*
     * Create a memory DC that is compatible with the screen DC.
     *   pszDriverName = "DISPLAY" - use the display device driver.
     *   2L = number of DEVOPENSTRUC fields provided to DevOpenDC.
     *   hdcScreen as the last argument makes the memory DC inherit the
     *   colour format and resolution of the physical display, ensuring
     *   that blits between them produce correct colours.
     *   OD_MEMORY = off-screen surface, not tied to any physical device.
     */
    memset(&dev, 0, sizeof(DEVOPENSTRUC));
    dev.pszDriverName = (PSZ)"DISPLAY";

    hdcMemory = DevOpenDC(hab, OD_MEMORY, (PCSZ)"*", 2L,
                          (PDEVOPENDATA)&dev, hdcScreen);

    /*
     * Create a micro PS associated with the memory DC.
     * sizl = {0,0} defers the PS size to the bitmap selected into it later.
     * PU_PELS    - coordinate units are device pixels
     * GPIA_ASSOC - associate this PS with hdcMemory
     * GPIT_MICRO - lightweight PS, sufficient for bitmap operations
     */
    sizl.cx = sizl.cy = 0;
    hpsMemory = GpiCreatePS(hab, hdcMemory, &sizl,
                            PU_PELS | GPIA_ASSOC | GPIT_MICRO);

    /*
     * Load each animation frame from the EXE resources into the memory PS.
     * Resource IDs run from ID_BITMAP01 (801) to ID_BITMAP16 (816).
     * module = 0 means the resource is in this EXE (not a DLL).
     * Width = Height = 0 loads the bitmap at its original size.
     */
    for (i = ID_BITMAP01; i <= ID_BITMAP16; i++) {
        abmph[i - ID_BITMAP01] = GpiLoadBitmap(
                hpsMemory,    /* PS context for loading          */
                (HMODULE)0,   /* 0 = load from this EXE          */
                i,            /* bitmap resource ID              */
                0,            /* target width  (0 = as stored)   */
                0);           /* target height (0 = as stored)   */
    }
}


/*
 * DisplayNextBitmap() - select a frame into the memory PS and blit it 2x.
 *
 * Queries the bitmap dimensions, then performs a 4-point stretch blit
 * (GpiBitBlt with lCount=4) that doubles the frame size on screen.
 */
VOID DisplayNextBitmap(HWND hwnd, LONG idBitmap)
{
    POINTL            aptl[4];
    BITMAPINFOHEADER2 bmp2;
    LONG              lWidth, lHeight;
    HPS               hpsWindow;

    /*
     * GpiQueryBitmapInfoHeader fills in the bitmap dimensions and format.
     * cbFix must be set to the struct size before calling.
     */
    memset(&bmp2, 0, sizeof(BITMAPINFOHEADER2));
    bmp2.cbFix = sizeof(BITMAPINFOHEADER2);
    GpiQueryBitmapInfoHeader(abmph[idBitmap], &bmp2);

    lWidth  = bmp2.cx;
    lHeight = bmp2.cy;

    /* Select the bitmap into the memory PS as the blit source. */
    GpiSetBitmap(hpsMemory, abmph[idBitmap]);

    /*
     * 4-point stretch blit: GpiBitBlt with lCount=4 interprets aptl as:
     *   aptl[0] - destination bottom-left
     *   aptl[1] - destination top-right   (2x source size = 2x magnification)
     *   aptl[2] - source bottom-left
     *   aptl[3] - source top-right        (full bitmap)
     */
    aptl[0].x = 0;           aptl[0].y = 0;
    aptl[1].x = lWidth * 2;  aptl[1].y = lHeight * 2;
    aptl[2].x = 0;           aptl[2].y = 0;
    aptl[3].x = lWidth;      aptl[3].y = lHeight;

    /*
     * WinGetPS acquires the window's cached PS without restricting drawing
     * to a damaged region (unlike WinBeginPaint).  Correct here because
     * we are painting from a timer, not in response to a WM_PAINT request.
     */
    hpsWindow = WinGetPS(hwnd);

    GpiBitBlt(hpsWindow,    /* target PS — the screen window  */
              hpsMemory,    /* source PS — the memory surface */
              4,            /* 4 points = stretch blit        */
              aptl,
              ROP_SRCCOPY,  /* direct copy, no colour mixing  */
              0);

    WinReleasePS(hpsWindow);
}
