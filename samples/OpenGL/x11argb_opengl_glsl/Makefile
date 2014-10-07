.PHONY: all

all: x11argb_opengl_glsl x11argb_opengl_glsl3

x11argb_opengl_glsl: x11argb_opengl_glsl.c Makefile
	cc -std=c99 -g3 -o x11argb_opengl_glsl x11argb_opengl_glsl.c -lX11 -lXrender -lGLEW -lm

x11argb_opengl_glsl3: x11argb_opengl_glsl.c Makefile
	cc -std=c99 -g3 -o x11argb_opengl_glsl3 -D USE_GLX_CREATE_CONTEXT_ATTRIB  x11argb_opengl_glsl.c -lX11 -lXrender -lGLEW -lm

