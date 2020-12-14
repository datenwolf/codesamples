.PHONY: all clean

all: xcb_shm_image_example

clean:
	- rm xcb_shm_image_example xcb_shm_image_example.o

xcb_shm_image_example: LDFLAGS += -lxcb -lxcb-image -lxcb-shm
xcb_shm_image_example: xcb_shm_image_example.o
