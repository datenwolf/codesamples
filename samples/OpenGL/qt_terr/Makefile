CC = gcc
CPP = g++
OBJS += quad.o terragen.o terrain.o vecmath.o main.o

qt_terr: $(OBJS)
	$(CPP) -o qt_terr $(OBJS) -lm -lGL -lGLU -lGLEW -lglut -lIL -lILU -lILUT

