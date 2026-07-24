# DEV-SAMPLES-C-PM-Animate
Two OS/2 Presentation Manager animation samples written in C.
Both use a PM timer (WM_TIMER) to cycle through bitmap frames,
demonstrating two different techniques for bitmap animation.

![Animate Screenshot](/wiki/Animate_001.png)

## LICENSE
* The 3-Clause BSD License.

## SAMPLES

### animate1 — Multiple individual bitmaps
Loads 16 separate bitmap resources (frame1.bmp .. frame16.bmp) from the EXE
into an off-screen memory DC/PS at startup.  On each WM_TIMER tick the next
bitmap is selected into the memory PS and blitted to the window at 2x size
using `GpiBitBlt` (source = HPS).

Key techniques:
- `DevOpenDC(OD_MEMORY)` — memory DC compatible with the screen DC
- `GpiCreatePS` with `GPIT_MICRO | GPIA_ASSOC` — lightweight micro PS
- `GpiLoadBitmap` — loads each frame from EXE resources
- `GpiBitBlt` with 4 points — stretch blit (2x magnification)
- `WinStartTimer` / `WM_TIMER` — timer-driven frame advance

### animate2 — Single sprite-sheet bitmap
All 16 frames live in one wide bitmap (frames.bmp, 800 x 50 px = 16 × 50 px
frames).  On each WM_TIMER tick the next 50×50 slice is extracted and blitted
at 2x size using `GpiWCBitBlt` (source = HBITMAP — no memory DC needed).

Key techniques:
- `GpiLoadBitmap` — loads the full sprite sheet from EXE resources
- `GpiWCBitBlt` with 4 points — blits directly from an HBITMAP handle,
  walking across the sheet by frame number × FRAME_WIDTH
- Simpler setup than animate1: no memory DC/PS required

## PROJECT STRUCTURE
```
animate1/
  src/
    animate1.c      Window procedure, LoadBitmaps, DisplayNextBitmap
    animate1.h      Resource IDs (ID_BITMAP01..16, ID_TIMER, ID_PROG)
    animate1.def    Module definition (NAME ANIMATE1 WINDOWAPI, HEAPSIZE)
    animate1.rc     Resource script (menu + 16 BITMAP entries)
    animate1.ico    Application icon
    frame1.bmp      Animation frame 1
    ...
    frame16.bmp     Animation frame 16
  bin-ow/           OpenWatcom build output  (generated, not in repo)
  bin-gcc/          GCC build output         (generated, not in repo)
  compile-gcc.cmd
  compile-ow.cmd
  Makefile.gcc
  Makefile.ow

animate2/
  src/
    animate2.c      Window procedure, LoadBitmap, DisplayNextFrame
    animate2.h      Resource IDs (ID_FRAMES, ID_TIMER, ID_PROG)
    animate2.def    Module definition (NAME ANIMATE2 WINDOWAPI, STACKSIZE)
    animate2.rc     Resource script (menu + single BITMAP entry)
    animate2.ico    Application icon
    frames.bmp      Sprite-sheet bitmap (800 x 50 px, 16 frames)
  bin-ow/           OpenWatcom build output  (generated, not in repo)
  bin-gcc/          GCC build output         (generated, not in repo)
  compile-gcc.cmd
  compile-ow.cmd
  Makefile.gcc
  Makefile.ow
```

## COMPILE TOOLS

### GCC
* ArcaOS 5.x
* gcc (GCC) 9.2.0 20190812 (OS/2 RPM build 9.2.0-5.oc00)
* GNU make 3.81 k2 (2017-11-10)
* watcom-wlink-hll (provides wl.exe — the open-source wlink linker)
* watcom-wrc (provides wrc.exe — the open-source Watcom Resource Compiler)

### OpenWatcom
* OpenWatcom C/C++ 2.0
* wmake (included with OpenWatcom)

## REQUIREMENTS

### GCC build
```
yum install gcc libc-devel binutils watcom-wlink-hll watcom-wrc kbuild-make
```

### OpenWatcom build
Install OpenWatcom and set the `WATCOM` environment variable to its root
directory (e.g. `SET WATCOM=C:\WATCOM`).

## COMPILE INSTRUCTIONS

Run the scripts from inside the `animate1\` or `animate2\` directory.

### GCC — using compile script
```
compile-gcc.cmd
```
Output: `bin-gcc\animate1.exe` (or `animate2.exe`)
Log:    `bin-gcc\compile.log`

### GCC — using GNU make
```
make -f Makefile.gcc
```

### OpenWatcom — using compile script
```
compile-ow.cmd
```
Output: `bin-ow\animate1.exe` (or `animate2.exe`)
Log:    `bin-ow\compile.log`

### OpenWatcom — using wmake
```
wmake -f Makefile.ow
```

**Note on the GCC linker:**
The compile scripts set `EMXOMFLD_TYPE=WLINK` and `EMXOMFLD_LINKER=wl.exe`
so GCC's linker front-end calls wlink instead of ilink.  This links kLIBC
(C runtime and startup code) automatically and reads the `.def` file to mark
the executable as a PM (Presentation Manager) application.

## HISTORY

* 1.02 - 2026-07-23
  - Reorganized sources into `src/` subdirectory for each sample.
  - Added OpenWatcom build support (`compile-ow.cmd`, `Makefile.ow`).
  - Replaced single GCC makefile with `compile-gcc.cmd` and `Makefile.gcc`.
  - Improved source code comments (timer animation pattern, memory DC/PS
    setup, GpiBitBlt vs GpiWCBitBlt, sprite-sheet frame extraction).
  - Fixed animate2 ClientWndProc message parameter type (USHORT -> ULONG).
  - Added `.gitignore`.

* 1.01 - 2023-06-10
  - Updated to compile on GCC 9 / ArcaOS 5.0.7.

* 1.00
  - Original version by Kelvin R. Lawrence (IBM OS/2 Toolkit sample, 1996).

## AUTHORS
* Martin Iturbide (2023)
* Kelvin R. Lawrence (1996)

## LINKS
* https://github.com/OS2World/DEV-SAMPLES-C-PM-Animate
