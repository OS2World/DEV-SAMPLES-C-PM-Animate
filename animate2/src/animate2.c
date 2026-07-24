/*
 * animate2.c  --  PM animation using a single sprite-sheet bitmap
 *
 * Demonstrates an alternative to loading many individual bitmaps: all 16
 * animation frames are stored side-by-side in one wide bitmap (frames.bmp,
 * 800 x 50 pixels = 16 frames x 50 pixels each).  On each WM_TIMER tick the
 * program advances a frame counter and blits the next 50x50 slice to the
 * window at 2x size using GpiWCBitBlt.
 *
 * Key difference from animate1:
 *   animate1 uses 16 separate bitmaps loaded into a memory PS, then blits
 *   via GpiBitBlt (source = HPS).
 *   animate2 uses ONE wide bitmap loaded directly, then blits via
 *   GpiWCBitBlt (source = HBITMAP), walking across the bitmap by frame number.
 *
 * GPI functions demonstrated:
 *   GpiLoadBitmap, GpiWCBitBlt
 *
 * Version : 1.02
 * License : 3-Clause BSD License
 * Authors : Martin Iturbide (2023), Kelvin R. Lawrence (1996)
 */

/*
 * INCL_32  - enables 32-bit API prototypes (IBM Toolkit legacy define;
 *            harmless with modern kLIBC / GCC headers).
 * INCL_GPI - GpiXxx graphics functions.
 * INCL_WIN - WinXxx window, message, and timer functions.
 * All defines must precede <os2.h>.
 */
#define INCL_32
#define INCL_GPI
#define INCL_WIN
#include <os2.h>
#include <memory.h>   /* memset */

#include "animate2.h"

#define TITLEBARTEXT (PCSZ)"animate2 - Using bitmaps and timers"
#define CLASSNAME    (PCSZ)"animate2"

HAB  hab;
HWND hwndFrame;

/*
 * Animation constants:
 *   NUM_FRAMES   - number of frames in the sprite sheet (frames.bmp)
 *   FRAME_WIDTH  - width of each frame in pixels
 *   FRAME_HEIGHT - height of each frame in pixels
 *
 * The sprite sheet is NUM_FRAMES * FRAME_WIDTH pixels wide and
 * FRAME_HEIGHT pixels tall.  Frame N starts at x = N * FRAME_WIDTH.
 */
#define NUM_FRAMES   16
#define FRAME_WIDTH  50
#define FRAME_HEIGHT 50

HBITMAP hbmpFrames;           /* handle to the sprite-sheet bitmap */
LONG    lCurrentFrame = 1;    /* current frame index (0-based)     */

/* Forward declarations */
MRESULT EXPENTRY ClientWndProc(HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2);
HBITMAP LoadBitmap(HWND hwnd, LONG idBitmap);
VOID    DisplayNextFrame(HWND hwnd, LONG lFrameNumber, HBITMAP hbmpFrames);


int main(VOID)
{
    HMQ  hmq;
    HWND hwndClient;
    QMSG qmsg;

    ULONG flCreateFlags = FCF_BORDER     | FCF_SHELLPOSITION |
                          FCF_TASKLIST   | FCF_TITLEBAR      | FCF_SYSMENU |
                          FCF_SIZEBORDER | FCF_MINMAX        | FCF_MENU;

    /*
     * PM startup sequence (dialog-free variant):
     *   1. WinInitialize      - connect to PM
     *   2. WinCreateMsgQueue  - create message queue
     *   3. WinRegisterClass   - register client class
     *   4. WinCreateStdWindow - create frame + client
     *   5. WinShowWindow      - show the frame
     *   6. WinStartTimer      - start 10 ms animation timer
     *   7. LoadBitmap         - load the sprite-sheet into window PS
     *   8. Message loop       - pump until WM_QUIT
     *   9. Cleanup
     */
    hab = WinInitialize(0);
    hmq = WinCreateMsgQueue(hab, 0);

    WinRegisterClass(hab, CLASSNAME, (PFNWP)ClientWndProc,
                     (ULONG)CS_SIZEREDRAW, (USHORT)256);

    hwndFrame = WinCreateStdWindow(HWND_DESKTOP, 0UL, &flCreateFlags,
                                   CLASSNAME, TITLEBARTEXT, WS_VISIBLE,
                                   (HMODULE)0, ID_PROG, &hwndClient);

    WinShowWindow(hwndFrame, TRUE);

    WinStartTimer(hab, hwndClient, ID_TIMER, 10);

    /*
     * LoadBitmap loads the entire sprite-sheet into the window PS.
     * Unlike animate1, no memory DC is needed: GpiWCBitBlt takes the
     * HBITMAP handle directly as its source.
     */
    hbmpFrames = LoadBitmap(hwndClient, ID_FRAMES);

    while (WinGetMsg(hab, &qmsg, (HWND)0, 0, 0))
        WinDispatchMsg(hab, &qmsg);

    WinDestroyWindow(hwndFrame);
    WinDestroyMsgQueue(hmq);
    WinTerminate(hab);
    return 0;
}


MRESULT EXPENTRY ClientWndProc(HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2)
{
    switch (msg)
    {
        case WM_ACTIVATE:
            /* Force a full repaint when this window gains focus. */
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
            /* Clear to black; the timer drives actual animation drawing. */
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
             * Advance to the next frame and wrap around at NUM_FRAMES.
             * lCurrentFrame is the x-offset selector into the sprite sheet.
             */
            DisplayNextFrame(hwnd, lCurrentFrame, hbmpFrames);
            lCurrentFrame++;
            if (lCurrentFrame >= NUM_FRAMES)
                lCurrentFrame = 0;
            break;

        default:
            return WinDefWindowProc(hwnd, msg, mp1, mp2);
    }
    return 0;
}


/*
 * LoadBitmap() - load the sprite-sheet bitmap resource from this EXE.
 *
 * WinGetPS acquires a temporary PS for the window; GpiLoadBitmap loads
 * the bitmap identified by idBitmap from the EXE resources (module = 0).
 * The HBITMAP handle is returned for use in DisplayNextFrame.
 */
HBITMAP LoadBitmap(HWND hwnd, LONG idBitmap)
{
    HBITMAP hbmp;
    HPS     hps = WinGetPS(hwnd);

    hbmp = GpiLoadBitmap(hps,         /* PS context for loading        */
                         (HMODULE)0,  /* 0 = resource is in this EXE   */
                         idBitmap,    /* resource ID (ID_FRAMES = 800) */
                         0,           /* target width  (0 = original)  */
                         0);          /* target height (0 = original)  */

    WinReleasePS(hps);
    return hbmp;
}


/*
 * DisplayNextFrame() - blit one frame from the sprite sheet to the window.
 *
 * The sprite sheet is a single wide bitmap (NUM_FRAMES * FRAME_WIDTH wide).
 * Frame N occupies pixels [N*FRAME_WIDTH .. (N+1)*FRAME_WIDTH - 1] horizontally.
 *
 * GpiWCBitBlt (Window-Compatible BitBlt) differs from GpiBitBlt in that its
 * source is an HBITMAP handle rather than an HPS presentation space.  This
 * means no memory DC or memory PS is required — the bitmap is blitted
 * directly from the handle.
 *
 * The blit is a 4-point stretch: destination is 2x the source frame size.
 */
VOID DisplayNextFrame(HWND hwnd, LONG lFrameNumber, HBITMAP hbmpFrames)
{
    POINTL aptl[4];
    LONG   x1, y1, x2, y2;
    HPS    hps;

    /*
     * Calculate the source rectangle within the sprite sheet for this frame.
     * x1 = left edge of this frame, x2 = right edge (exclusive).
     * y1/y2 span the full bitmap height (FRAME_HEIGHT pixels, y-up).
     */
    x1 = lFrameNumber * FRAME_WIDTH;
    y1 = 0;
    x2 = x1 + FRAME_WIDTH  - 1;
    y2 = y1 + FRAME_HEIGHT - 1;

    /*
     * 4-point stretch blit with GpiWCBitBlt:
     *   aptl[0] - destination bottom-left  (screen, at origin)
     *   aptl[1] - destination top-right    (2x frame size)
     *   aptl[2] - source bottom-left       (this frame's x offset in sheet)
     *   aptl[3] - source top-right         (this frame's right/top edge)
     */
    aptl[0].x = 0;                    aptl[0].y = 0;
    aptl[1].x = 2 * FRAME_WIDTH  - 1; aptl[1].y = 2 * FRAME_HEIGHT - 1;
    aptl[2].x = x1;                   aptl[2].y = 0;
    aptl[3].x = x2;                   aptl[3].y = y2;

    /*
     * WinGetPS for timer-driven painting (does not restrict to damaged area).
     */
    hps = WinGetPS(hwnd);

    GpiWCBitBlt(hps,         /* target PS — the screen window              */
                hbmpFrames,  /* source HBITMAP — the full sprite sheet      */
                4,           /* 4 points = stretch blit                    */
                aptl,
                ROP_SRCCOPY, /* direct copy, no colour mixing              */
                0);

    WinReleasePS(hps);
}
