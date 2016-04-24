OBJS = main.o positiongen.o solid.o stats.o shaderloader.o debuggl.o
CFLAGS = -std=c99 -I../../../extra
LDLIBS = -lGL -lGLEW -lglut -lm

.PHONY: all clean

all: computeshader

clean:
	-rm -f $(OBJS) computeshader

computeshader: $(OBJS)
	$(CC) $(LDFLAGS) -o computeshader $(OBJS) $(LOADLIBES) $(LDLIBS)

shaderloader.o: ../../../extra/shaderloader/shaderloader.c
	$(CC) $(CPPFLAGS) $(CFLAGS) -c $<

debuggl.o: ../../../extra/debuggl/debuggl.c
	$(CC) $(CPPFLAGS) $(CFLAGS) -c $<
