#include <stdlib.h>
#include <stdio.h>

#include <sys/ipc.h>
#include <sys/shm.h>

#include <xcb/xcb.h>
#include <xcb/shm.h>
#include <xcb/xcb_image.h>

#if __ORDER_LITTLE_ENDIAN == __BYTE_ORDER__
# define NATIVE_XCB_IMAGE_ORDER    XCB_IMAGE_ORDER_LSB_FIRST
#else
# define NATIVE_XCB_IMAGE_ORDER    XCB_IMAGE_ORDER_MSB_FIRST
#endif

enum  {
	IMAGE_WIDTH  = 512,
	IMAGE_HEIGHT = 512,
	IMAGE_DEPTH  = 24,
};

static xcb_format_t const *query_xcb_format_for_depth(
	xcb_connection_t *const connection,
	unsigned depth )
{
	xcb_setup_t const *const setup = xcb_get_setup(connection);
	xcb_format_iterator_t it;
	for( it = xcb_setup_pixmap_formats_iterator(setup)
	   ; it.rem
	   ; xcb_format_next(&it)
	){
		xcb_format_t const *const format = it.data;
		if( format->depth == depth ){
			return format;
		}
	}
	return NULL;
}

typedef struct shm_xcb_image_t {
	xcb_connection_t *connection;
	xcb_image_t  *image;
	xcb_shm_seg_t shm_seg;
	int           shm_id;
} shm_xcb_image_t;

static shm_xcb_image_t *shm_xcb_image_create(
	xcb_connection_t *const connection,
	unsigned const width,
	unsigned const height,
	unsigned const depth )
{
	xcb_generic_error_t *error = NULL;

	shm_xcb_image_t *shmimg = calloc(1, sizeof(*shmimg));
	if( !shmimg ){ goto fail; }
	shmimg->connection = connection;

	xcb_format_t const *const format = query_xcb_format_for_depth(connection, depth);
	shmimg->image = xcb_image_create(
		width, height,
		XCB_IMAGE_FORMAT_Z_PIXMAP,
		format->scanline_pad,
		format->depth, format->bits_per_pixel, 0,
		NATIVE_XCB_IMAGE_ORDER,
		XCB_IMAGE_ORDER_MSB_FIRST,
		NULL, ~0, 0);
	if( !shmimg->image ){
		fprintf(stderr, "could not create X11 image structure\n");
	}
	size_t const image_segment_size = shmimg->image->stride * shmimg->image->height;

	shmimg->shm_id = shmget(IPC_PRIVATE, image_segment_size, IPC_CREAT | 0600);
	if( 0 > shmimg->shm_id ){ goto fail; }

	shmimg->image->data = shmat(shmimg->shm_id, 0, 0);
	if( (void*)-1 == (void*)(shmimg->image->data) ){ goto fail; }

	shmimg->shm_seg = xcb_generate_id(connection),
	error = xcb_request_check(connection,
		xcb_shm_attach_checked(
			connection,
			shmimg->shm_seg, shmimg->shm_id, 0) );
fail:
	if( shmimg ){
		if( shmimg->image ){
			if( shmimg->image->data && error ){
				shmdt(shmimg->image->data);
				shmimg->image->data = NULL;
			}
			if( !shmimg->image->data ){
				shmctl(shmimg->shm_id, IPC_RMID, 0);
				shmimg->shm_id = -1;
			}
		}
		if( 0 > shmimg->shm_id ){
			xcb_image_destroy(shmimg->image);
			shmimg->image = NULL;
		}
		if( !shmimg->image ){
			free(shmimg);
			shmimg = NULL;
		}
	}
	free(error);

	return shmimg;
}

static void shm_xcb_image_destroy(shm_xcb_image_t *shmimg)
{
	xcb_shm_detach(shmimg->connection, shmimg->shm_seg);
	shmdt(shmimg->image->data);
	shmctl(shmimg->shm_id, IPC_RMID, 0);
	xcb_image_destroy(shmimg->image);
	free(shmimg);
}

static void generate_image(
	shm_xcb_image_t *shmimg,
	unsigned t )
{
	for( unsigned j = 0; j < shmimg->image->height; ++j ){
		uint8_t *const line = shmimg->image->data + j * shmimg->image->stride;
		for( unsigned i = 0; i < shmimg->image->width; ++i ){
			unsigned const bytes_per_pixel = shmimg->image->bpp/8;
			uint8_t *pixel = line + i * bytes_per_pixel;

			unsigned const a = (i + t);
			unsigned const b = (j + (i >> 8) & 0xFF);
			unsigned const c = (j >> 8) & 0xFF;

			switch( bytes_per_pixel ){
			case 4: pixel[3] = 0xFF; /* fallthrough */
			case 3: pixel[2] = a & 0xFF; /* fallthrough */
			case 2: pixel[1] = b & 0xFF; /* fallthrough */
			case 1: pixel[0] = c & 0xFF; /* fallthrough */
			default: break;
			}
		}
	}
}

int main(int argc, char *argv[])
{
	/* Open the connection to the X server */
	xcb_connection_t *connection = xcb_connect(NULL, NULL);

	/* Check that X MIT-SHM is available (should be). */
	const xcb_query_extension_reply_t *shm_extension = xcb_get_extension_data(connection, &xcb_shm_id);
	if( !shm_extension || !shm_extension->present ){
		fprintf(stderr, "Query for X MIT-SHM extension failed.\n");
		return 1;
	}

	shm_xcb_image_t *shmimg = shm_xcb_image_create(connection, IMAGE_WIDTH, IMAGE_HEIGHT, IMAGE_DEPTH);
	if( !shmimg ){
		fprintf(stderr, "Creating shared memory image failed");
	}

	/* Get the first screen */
	xcb_screen_t *const screen = xcb_setup_roots_iterator(xcb_get_setup(connection)).data;

	/* Create a window */
	uint32_t const window_mask = XCB_CW_EVENT_MASK;
	uint32_t const window_values[] = { XCB_EVENT_MASK_EXPOSURE};
	xcb_drawable_t const window = xcb_generate_id(connection);
	xcb_create_window(connection,
		XCB_COPY_FROM_PARENT,          /* depth */
		window,                        /* window Id */
		screen->root,                  /* parent window */
		0, 0,                          /* x, y */
		IMAGE_WIDTH, IMAGE_HEIGHT,     /* width, height */
		0,                             /* border_width */
		XCB_WINDOW_CLASS_INPUT_OUTPUT, /* class */
		screen->root_visual,           /* visual */
		window_mask, window_values     /* masks */
	);

	/* Create black (foreground) graphic context */
	xcb_gcontext_t gc = xcb_generate_id( connection );
	uint32_t const gc_mask = XCB_GC_FOREGROUND | XCB_GC_GRAPHICS_EXPOSURES;
	uint32_t const gc_values[] = {screen->black_pixel, 0};
	xcb_create_gc(connection, gc, window, gc_mask, gc_values);

	/* Map the window on the screen and flush*/
	xcb_map_window(connection, window);
	xcb_flush(connection);

	/* Event loop */
	unsigned counter = 0;
	for( xcb_generic_event_t *event
	   ; (event = xcb_wait_for_event(connection))
	   ; free(event)
	){
		switch( event->response_type & ~0x80 ){
		case XCB_EXPOSE:
			generate_image(shmimg, counter++);
			xcb_shm_put_image(connection, window, gc,
				shmimg->image->width, shmimg->image->height, 0, 0,
				shmimg->image->width, shmimg->image->height, 0, 0,
				shmimg->image->depth, shmimg->image->format, 0, 
				shmimg->shm_seg, 0);

			/* flush the request */
			xcb_flush(connection);
			break;
		default:
			/* Unknown event type, ignore it */
			break;
		}

	}

	shm_xcb_image_destroy(shmimg);

	return 0;
}
