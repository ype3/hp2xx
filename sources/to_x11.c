/*
   Copyright (c) 1991 - 1994 Michael Schoene & Heinz W. Werntges.
   Parts Copyright (c) 1999  Martin Kroeker.
   
   All rights reserved. Distributed by Free Software Foundation, Inc.

This file is part of HP2xx.

HP2xx is distributed in the hope that it will be useful, but
WITHOUT ANY WARRANTY.  No author or distributor accepts responsibility
to anyone for the consequences of using it or for whether it serves any
particular purpose or works at all, unless he says so in writing.  Refer
to the GNU General Public License, Version 2 or later, for full details.

Everyone is granted permission to copy, modify and redistribute
HP2xx, but only under the conditions described in the GNU General Public
License.  A copy of this license is supposed to have been
given to you along with HP2xx so you can know your rights and
responsibilities.  It should be in a file named COPYING.  Among other
things, the copyright notice and this notice must be preserved on all
copies.

- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
*/

/** to_x11.c: X11 preview part of project "hp2xx" (UNIX only)
 **
 ** 92/01/15  V 1.00  HWW  Derived from to_vga.c (V1.01b)
 **			  X11 essentials due to M. Schoene
 ** 92/01/28  V 1.01  HWW  Window offset user-defined via -o -O
 ** 92/02/03  V 1.02  HWW  bug fixes, error handling
 ** 92/05/19  V 1.02b HWW  Abort if color mode
 ** 92/05/25  V 1.10  HWW  8 Colors supported
 ** 93/01/06  V 1.10b HWW  Improved selection of foreground color
 ** 94/02/14  V 1.20a HWW  Adapted to changes in hp2xx.h
 **
 **	      NOTE: Color assignment leaves something to be desired
 **		    with some X11 servers.
 **/


#include <stdio.h>
#include <stdlib.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>

#include "bresnham.h"
#include "hp2xx.h"
#include "x11.h"




#define WIN_NAME	"x11"		/* Window name 		*/
#define PROG_NAME	"hp2xx"		/* Program name			*/



/**
 ** Global variables for X11
 **/

static Display*		XDisplay = NULL;	/* Workstation id	*/
static int		XScreen;
static Window		XRoot;			/* Number of root window	*/
static Visual*		XVisual = NULL;

static GC		XGcWin;
static Window		XWin;			/* Window id		*/

static	unsigned long	col_table[CMS_SIZE];
static	XColor		Xcol;
static	Colormap	def_cmap;

/**
 ** Screen sizes
 **/

static int	scr_width;
static int	scr_height;


/**
 ** Window sizes
 **/

static int	width;
static int	bytes;
static int	height;



/**
 ** Initialize X11 and open window
 **/

static int
win_open(const GEN_PAR *pg, int x, int y, int w, int h )
{
	char*			DisplayName = NULL;
	char**			argv;
	XSizeHints		Hints;
	unsigned long		ValueMask;
	XSetWindowAttributes	WinAttr;
	XEvent			Event;
	char 			colorname[13];
        int i;

	/**
	 ** Simulate command line arguments
	 **/

	argv = (char**) malloc( 2 * sizeof( char* ) );
	argv[0] = PROG_NAME;
	argv[1] = NULL;

	/**
	 ** X11 server reachable ?
	 **/

    if ((XDisplay = (Display *) XOpenDisplay( DisplayName )) == NULL)
	{
		Eprintf("No X11 server found !\n" );
		return NO_SERVER;
	}

	XScreen		= DefaultScreen( XDisplay );
	XRoot		= RootWindow( XDisplay, XScreen );
	XVisual		= DefaultVisual( XDisplay, XScreen );
	XGcWin		= DefaultGC( XDisplay, XScreen );

	scr_width	= WidthOfScreen( ScreenOfDisplay( XDisplay, XScreen ) );
	scr_height	= HeightOfScreen(ScreenOfDisplay( XDisplay, XScreen ) );
	if (x+w > scr_width || y+h > scr_height)
	{
		Eprintf ("Window exceeds screen limits !\n" );
		return SIZE;
	}

	/**
	 ** Set window attributes
	 **/

	WinAttr.background_pixel= WhitePixel( XDisplay, XScreen );
	WinAttr.border_pixel	= WhitePixel( XDisplay, XScreen );
	WinAttr.backing_store	= Always;
	ValueMask = CWBackPixel | CWBorderPixel | CWBackingStore;

	/**
	 ** Create Window
	 **/

	XWin = XCreateWindow( XDisplay, XRoot,
						  x, y, w, h,
						  1, 0,
						  CopyFromParent,
						  CopyFromParent,
						  ValueMask, &WinAttr );

	/**
	 ** Define window properties
	 **/

	Hints.flags = PSize | PMinSize | PMaxSize | USPosition;
	Hints.x		= x;
	Hints.y		= y;
	Hints.width	= Hints.min_width  = Hints.max_width  = w;
	Hints.height= Hints.min_height = Hints.max_height = h;

	XSetStandardProperties( XDisplay, XWin,
					WIN_NAME, WIN_NAME, 0,
					argv, 1, &Hints );

/**
 ** Define color table (compatible to SunView and Turbo-C usage)
 **/

  def_cmap    = DefaultColormap( XDisplay, XScreen );
  if (DefaultDepth( XDisplay, XScreen ) < 4)
  {
	col_table[BLACK]	= WhitePixel( XDisplay, XScreen );
	col_table[WHITE]	= BlackPixel( XDisplay, XScreen );
	col_table[GRAY]		= col_table[WHITE];
	col_table[RED]		= col_table[WHITE];
	col_table[GREEN]	= col_table[WHITE];
	col_table[BLUE]		= col_table[WHITE];
	col_table[CYAN]		= col_table[WHITE];
	col_table[MAGENTA]	= col_table[WHITE];
	col_table[YELLOW]	= col_table[WHITE];
	col_table[LIGHTGRAY]	= col_table[WHITE];
	col_table[LIGHTRED]	= col_table[WHITE];
	col_table[LIGHTGREEN]	= col_table[WHITE];
	col_table[LIGHTBLUE]	= col_table[WHITE];
	col_table[LIGHTCYAN]	= col_table[WHITE];
	col_table[LIGHTMAGENTA]	= col_table[WHITE];
  }
  else
  {
	XParseColor( XDisplay, def_cmap, "gray10",&Xcol );
	XAllocColor( XDisplay, def_cmap, &Xcol );
	col_table[GRAY] = Xcol.pixel;

for (i=1;i<=pg->maxpens;i++){

sprintf (colorname,"#%2.2X%2.2X%2.2X", pg->Clut[i][0],pg->Clut[i][1],pg->Clut[i][2]);
/*fprintf (stderr,"#%2.2X%2.2X%2.2X", pg->Clut[i][0],pg->Clut[i][1],pg->Clut[i][2]);*/
	XParseColor( XDisplay, def_cmap, colorname, &Xcol );
	XAllocColor( XDisplay, def_cmap, &Xcol );
	col_table[i] = Xcol.pixel;
}
  }

	/**
	 **  Set foreground and background colors
	 **/

	XSetState ( 	XDisplay, XGcWin,
			col_table[BLACK], col_table[WHITE],
			GXcopy, AllPlanes );

	/**
	 ** Define permitted events for this window
	 **/

	XSelectInput( XDisplay, XWin, ExposureMask | VisibilityChangeMask);

	/**
	 ** Display window
	 **/

	XMapWindow( XDisplay, XWin );
	do {
		XNextEvent( XDisplay, &Event);
	} while (Event.type != Expose && Event.type != VisibilityNotify);

	width	= w;
	height	= h;
	bytes	= (w + 7) / 8;

	free( (char*) argv );
	return( 0 );
}



static void
win_close()
{
	XDestroyWindow( XDisplay, XWin );
	XCloseDisplay( XDisplay );
}




#define	setXcolor(col) XSetForeground (XDisplay, XGcWin, col_table[col])




int
PicBuf_to_X11 (const GEN_PAR *pg, const OUT_PAR *po)
/**
 ** Interface to higher-level routines,
 **   similar in structure to other previewers
 **/
{
int		row_c, x, y;
const RowBuf	*row;
const PicBuf	*pb;

  if (pg == NULL || po == NULL)
	return ERROR;
  pb = po->picbuf;
  if (pb == NULL)
	return ERROR;

  if (!pg->quiet)
  {
	Eprintf ("\nX11 preview follows.\n");
	Eprintf ("Press <return> to end graphics mode\n");
  }

  if (win_open(pg, (int)(po->xoff * po->dpi_x / 25.4),
		(int)(po->yoff * po->dpi_y / 25.4),
		pb->nb << 3, pb->nr ) )
	return ERROR;

  /* Backward since highest index is lowest line on screen! */
  for (row_c=0, y=pb->nr-1; row_c < pb->nr; row_c++, y--)
  {
	row = get_RowBuf (pb, row_c);
	if (row == NULL)
		return 0;
	for (x=0; x < pb->nc; x++)
	{
		switch (index_from_RowBuf(row, x, pb))
		{

		case xxBackground:			continue;
		case xxForeground:setXcolor (WHITE);	break;
/*
		case xxRed:	setXcolor (RED);	break;
		case xxGreen:	setXcolor (GREEN);	break;
		case xxBlue:	setXcolor (BLUE);	break;
		case xxCyan:	setXcolor (CYAN);	break;
		case xxMagenta:	setXcolor (MAGENTA);	break;
		case xxYellow:	setXcolor (YELLOW);	break;
*/
		default:	setXcolor(index_from_RowBuf(row,x,pb)); break;
		}
		XDrawPoint (XDisplay, XWin, XGcWin, x, y);
	}
  }

  XFlush( XDisplay );
  SilentWait();
  win_close();
  return 0;
}
