/*------------------------------------------------------------------------
 * A demonstration of a clobberable X11 window, i.e. a window that will
 * assume the contents of whatever was visible above it, without clearing
 * or tidying up itself.
 *
 * (c) 2013 by Wolfgang 'datenwolf' Draxinger
  
 * License agreement: This source code is provided "as is". You
 * can use this source code however you want for your own personal
 * use. If you give this source code to anybody else then you must
 * leave this message in it.
------------------------------------------------------------------------*/
#define _GNU_SOURCE

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>

#include <sys/types.h>
#include <time.h>

#include <X11/Xatom.h>
#include <X11/Xutil.h>

static void fatalError(const char *why)
{
	fprintf(stderr, "%s", why);
	exit(0x666);
}

static int Xscreen;
static Atom del_atom;
static Colormap cmap;
static Display *Xdisplay;
static Visual *Xvisual;
static Window Xroot, window_handle;
static int width, height;

static Bool WaitForMapNotify(Display *d, XEvent *e, char *arg)
{
	return d && e && arg && (e->type == MapNotify) && (e->xmap.window == *(Window*)arg);
}

static void createTheWindow()
{
	XEvent event;
	int x,y, attr_mask;
	XSizeHints hints;
	XWMHints *startup_state;
	XTextProperty textprop;
	XSetWindowAttributes attr = {0,};
	static char *title = "FTB's little OpenGL example - ARGB extension by WXD";

	Xdisplay = XOpenDisplay(NULL);
	if (!Xdisplay) {
		fatalError("Couldn't connect to X server\n");
	}
	Xscreen = DefaultScreen(Xdisplay);
	Xroot = RootWindow(Xdisplay, Xscreen);
	Xvisual = DefaultVisual(Xdisplay, Xscreen);

	/* Create a colormap - only needed on some X clients, eg. IRIX */
	cmap = XCreateColormap(Xdisplay, Xroot, Xvisual, AllocNone);

	attr.colormap = cmap;
	attr.background_pixmap = None;
	attr.border_pixmap = None;
	attr.border_pixel = 0;
	attr.event_mask =
		StructureNotifyMask |
		EnterWindowMask |
		LeaveWindowMask |
		ExposureMask |
		ButtonPressMask |
		ButtonReleaseMask |
		OwnerGrabButtonMask |
		KeyPressMask |
		KeyReleaseMask;

	attr_mask = 0
	//	| CWBackPixmap
		| CWColormap
	//	| CWBorderPixel
		| CWEventMask ;

	width  = DisplayWidth(Xdisplay, DefaultScreen(Xdisplay))/2;
	height = DisplayHeight(Xdisplay, DefaultScreen(Xdisplay))/2;
	x=width/2, y=height/2;

	window_handle = XCreateWindow(	Xdisplay,
					Xroot,
					x, y, width, height,
					0,
					CopyFromParent,
					InputOutput,
					Xvisual,
					attr_mask, &attr);

	if( !window_handle ) {
		fatalError("Couldn't create the window\n");
	}

	textprop.value = (unsigned char*)title;
	textprop.encoding = XA_STRING;
	textprop.format = 8;
	textprop.nitems = strlen(title);

	hints.x = x;
	hints.y = y;
	hints.width = width;
	hints.height = height;
	hints.flags = USPosition|USSize;

	startup_state = XAllocWMHints();
	startup_state->initial_state = NormalState;
	startup_state->flags = StateHint;

	XSetWMProperties(Xdisplay, window_handle,&textprop, &textprop,
			NULL, 0,
			&hints,
			startup_state,
			NULL);

	XFree(startup_state);

	XMapWindow(Xdisplay, window_handle);
	XIfEvent(Xdisplay, &event, WaitForMapNotify, (char*)&window_handle);

	if ((del_atom = XInternAtom(Xdisplay, "WM_DELETE_WINDOW", 0)) != None) {
		XSetWMProtocols(Xdisplay, window_handle, &del_atom, 1);
	}
}

static int updateTheMessageQueue()
{
	XEvent event;
	XConfigureEvent *xc;

	while (XPending(Xdisplay))
	{
		XNextEvent(Xdisplay, &event);
		switch (event.type)
		{
		case ClientMessage:
			if (event.xclient.data.l[0] == del_atom)
			{
				return 0;
			}
		break;

		case ConfigureNotify:
			xc = &(event.xconfigure);
			width = xc->width;
			height = xc->height;
			break;
		}
	}
	return 1;
}

int main(int argc, char *argv[])
{
	createTheWindow();

	while (updateTheMessageQueue()) {
	}

	return 0;
}

