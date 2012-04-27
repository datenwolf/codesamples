CC = gcc
CPP = g++
OBJS += main.o quad.o terragen.o terrain.o vecmath.o

qt_terr: $(OBJS)
	$(CPP) -o qt_terr $(OBJS) -lm -lGL -lGLU -lGLEW -lglut -lIL -lILU -lILUT

