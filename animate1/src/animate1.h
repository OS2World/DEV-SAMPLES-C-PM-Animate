/*
 * animate1.h  --  Resource IDs and constants for the animate1 sample
 */

#define ID_PROG     1    /* program resource ID (window + menu)  */
#define IDM_EXIT  100    /* "Exit now" menu item                 */
#define IDM_RESUME 105   /* "Resume" menu item                   */

#define ID_TIMER   10    /* timer ID used with WinStartTimer     */

/*
 * Bitmap resource IDs for the 16 animation frames.
 * IDs run sequentially from 801 to 816 so that LoadBitmaps() can
 * iterate: for (i = ID_BITMAP01; i <= ID_BITMAP16; i++) ...
 * and index into abmph[] as abmph[i - ID_BITMAP01].
 */
#define ID_BITMAP01  801
#define ID_BITMAP02  802
#define ID_BITMAP03  803
#define ID_BITMAP04  804
#define ID_BITMAP05  805
#define ID_BITMAP06  806
#define ID_BITMAP07  807
#define ID_BITMAP08  808
#define ID_BITMAP09  809
#define ID_BITMAP10  810
#define ID_BITMAP11  811
#define ID_BITMAP12  812
#define ID_BITMAP13  813
#define ID_BITMAP14  814
#define ID_BITMAP15  815
#define ID_BITMAP16  816
