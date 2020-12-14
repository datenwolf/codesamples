#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include <X11/Xlib.h>

/*
If you get linking errors when using C++, you need
to add extern "C" here or in X11-xcb.h, unless
this bug is already fixed in your version:
http://bugs.freedesktop.org/show_bug.cgi?id=22252
*/
#include <X11/Xlib-xcb.h> /* for XGetXCBConnection, link with libX11-xcb */

#include <xcb/xcb.h>

#include <GL/glx.h>
#include <GL/gl.h>

static
void draw()
{
	glClearColor(0.2, 0.4, 0.9, 1.0);
	glClear(GL_COLOR_BUFFER_BIT);
}

static 
int main_loop(
	Display *display,
	xcb_connection_t *connection,
	xcb_window_t window,
	GLXDrawable drawable )
{
	int running = 1;
	while(running) {
		/* Wait for event */
		xcb_generic_event_t *event = xcb_wait_for_event(connection);
		if( !event ) {
			fprintf(stderr, "i/o error in xcb_wait_for_event");
			return -1;
		}

		switch(event->response_type & ~0x80) {
		case XCB_KEY_PRESS:
			/* Quit on key press */
			running = 0;
			break;

		case XCB_EXPOSE:
			/* Handle expose event, draw and swap buffers */
			draw();
			glXSwapBuffers(display, drawable);
			break;

		default:
			break;
		}

		free(event);
	}

	return 0;
}

static
int setup_and_run(
	Display* display,
	xcb_connection_t *connection,
	int default_screen,
	xcb_screen_t *screen )
{
	xcb_generic_error_t *error;

	/* Query framebuffer configurations */
	int num_fb_configs = 0;
	GLXFBConfig * const fb_configs = glXGetFBConfigs(display, default_screen, &num_fb_configs);
	if( !fb_configs || !num_fb_configs ){
		fprintf(stderr, "glXGetFBConfigs failed\n");
		return -1;
	}

	GLXFBConfig fb_config;
	int visualID = 0;
	for(int i = 0; !visualID && i < num_fb_configs; ++i) {
		/* Select first framebuffer config with a valid visual and query visualID */
		fb_config = fb_configs[i];
		XVisualInfo * const visual = glXGetVisualFromFBConfig(display, fb_config);
		if( !visual ) { continue; }
		visualID = visual->visualid;
	}
	if( !visualID ) {
		return -1;
	}

	/* Create OpenGL context */
	GLXContext const context = glXCreateNewContext(display, fb_config, GLX_RGBA_TYPE, 0, True);
	if( !context ) {
		fprintf(stderr, "glXCreateNewContext failed\n");
		return -1;
	}

	/* Create XID's for colormap and window */
	xcb_colormap_t colormap = xcb_generate_id(connection);
	xcb_window_t window = xcb_generate_id(connection);

	/* Create colormap */
	if((error = xcb_request_check(connection, xcb_create_colormap_checked(
			connection,
			XCB_COLORMAP_ALLOC_NONE,
			colormap,
			screen->root,
			visualID)
		)
	)){
		fprintf(stderr, "error creating colormap: %d\n", error->error_code);
		free(error);
		return -1;
	}

	/* Create window */
	uint32_t const eventmask = XCB_EVENT_MASK_EXPOSURE | XCB_EVENT_MASK_KEY_PRESS;
	uint32_t const valuelist[] = { eventmask, colormap };
	uint32_t const valuemask = XCB_CW_EVENT_MASK | XCB_CW_COLORMAP;

	if((error= xcb_request_check(connection, xcb_create_window_checked(
			connection,
			XCB_COPY_FROM_PARENT,
			window,
			screen->root,
			0, 0,
			150, 150,
			0,
			XCB_WINDOW_CLASS_INPUT_OUTPUT,
			visualID,
			valuemask,
			valuelist)
		)
	)){
		fprintf(stderr, "error creating window: %d\n", error->error_code);
		return -1;
	}

	// NOTE: window must be mapped before glXMakeContextCurrent
	if((error= xcb_request_check(connection, xcb_map_window(connection, window)) )) {
		fprintf(stderr, "error mapping window");
		return -1;
	}

	/* Create GLX Window */
	GLXDrawable drawable = 0;

	GLXWindow glxwindow = glXCreateWindow(display, fb_config, window, 0);
	if( !window ) {
		xcb_destroy_window(connection, window);
		glXDestroyContext(display, context);

		fprintf(stderr, "glXDestroyContext failed\n");
		return -1;
	}

	drawable = glxwindow;

	/* make OpenGL context current */
	if( !glXMakeContextCurrent(display, drawable, drawable, context) ) {
		xcb_destroy_window(connection, window);
		glXDestroyContext(display, context);

		fprintf(stderr, "glXMakeContextCurrent failed\n");
		return -1;
	}

	/* run main loop */
	int retval = main_loop(display, connection, window, drawable);

	/* Cleanup */
	glXDestroyWindow(display, glxwindow);
	xcb_destroy_window(connection, window);
	glXDestroyContext(display, context);

	return retval;
}

static
void screen_from_Xlib_Display(
	Display * const display,
	xcb_connection_t *connection,
	int * const out_screen_num,
	xcb_screen_t ** const out_screen)
{
	xcb_screen_iterator_t screen_iter = xcb_setup_roots_iterator(xcb_get_setup(connection));
	int screen_num = DefaultScreen(display);
	while( screen_iter.rem && screen_num > 0 ) {
		xcb_screen_next(&screen_iter);
		--screen_num;
	}
	*out_screen_num = screen_num;
	*out_screen = screen_iter.data;
}

int main(int argc, char* argv[])
{
	Display *display;

	/* Open Xlib Display */ 
	display = XOpenDisplay(0);
	if( !display ) {
		fprintf(stderr, "Can't open display\n");
		return -1;
	}


	/* Get the XCB connection from the display */
	xcb_connection_t *connection = XGetXCBConnection(display);
	if( !connection ) {
		XCloseDisplay(display);
		fprintf(stderr, "Can't get xcb connection from display\n");
		return -1;
	}

	/* Acquire event queue ownership */
	XSetEventQueueOwner(display, XCBOwnsEventQueue);

	/* Find XCB screen */
	int screen_num;
	xcb_screen_t *screen = 0;
	screen_from_Xlib_Display(display, connection, &screen_num, &screen);
	if( !screen ) {
		return -1;
	}

	/* Initialize window and OpenGL context, run main loop and deinitialize */  
	int retval = setup_and_run(display, connection, screen_num, screen);

	/* Cleanup */
	XCloseDisplay(display);

	return retval;
}
