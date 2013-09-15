/*------------------------------------------------------------------------
 * What happens if you stuff the X11 server with large and large amounts
 * of atoms? When does it run out of memory? How is its performance
 * impaired by this? This is a little program to experimenting with
 * torturing the X11 server by overfeeding it with atoms.
 *
 * (c) 2013 datenwolf
 *
 * License agreement: This source code is provided "as is". You
 * can use this source code however you want for your own personal
 * use. If you give this source code to anybody else then you must
 * leave this message in it.
------------------------------------------------------------------------*/

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdint.h>

#include <time.h>
#include <unistd.h>

#include <X11/Xatom.h>
#include <X11/Xutil.h>

static int Xscreen;
static Display *Xdisplay;

char const doitkey[] = "wastemyX11server";

int main(int argc, char *argv[])
{
	uint64_t i;
	unsigned int r = getpid() ^ time(NULL);

	if( argc < 2 || strcmp(argv[1], doitkey) ) {
		fprintf(stderr,
"***************** WARNING! *****************\n"
"\n"
"This program wastes serious resources of the\n"
"X11 server it is started on. Do not execute\n"
"this program on a production X11 session as\n"
"the allocated resources can not be reclaimed\n"
"without resetting or quiting the X11 server.\n"
"\n"
"To actually perform this, do the following:\n"
"\n"
"%s %s\n"
"\n"
"***************** WARNING! *****************\n",
			argv[0], doitkey);
		return 0;
	}

	Xdisplay = XOpenDisplay(NULL);
	if (!Xdisplay) {
		fprintf(stderr, "Couldn't connect to X server\n");
		return -1;
	}
	Xscreen = DefaultScreen(Xdisplay);

	for(i=0; i < 0xffffffff; i++) {
		char atomstr[33];
		snprintf(atomstr,32, "_wasted_0x%08x_0x%08x", r, (unsigned int)i);
		XInternAtom(Xdisplay, atomstr, False);
		if( !(i % 0x00010000 ) ) {
			fprintf(stderr, "%s\n", atomstr);
		}
	}

	return 0;
}

