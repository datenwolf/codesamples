#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "terragen.h"

// Some custom types and helper macros that
// allow for a portable file to number variable
// read procedure

typedef uint8_t DUET[2];
typedef uint8_t QUARTET[4];
typedef uint8_t OCTET[8];

#define DUET_TO_NUMBER(duet)		((duet[0]|duet[1]<<8))
#define QUARTET_TO_NUMBER(quartet)	((quartet[0])|(quartet[1]<<8)|(quartet[2]<<16)|(quartet[3]<<24))

int read_terrain(char const *filename, double **pbuffer, int *width, int *height, double *scale)
// [in] filename: path to the file to be read in
// [out] pbuffer: address of new allocated data buffer
// [out] width: width of the terrain
// [out] heigt: height of the terrain
// [out] return value: 0 if read in successfully 1 otherwise
{
	FILE *file=fopen(filename, "rb");
	if(!file)
		return -1;

	OCTET octet;
	QUARTET quartet;
	QUARTET quartet_marker;
	DUET duet;

	fread(octet, 8, 1, file);
	if(memcmp(octet, "TERRAGEN", 8))
		return -1;

	fread(octet, 8, 1, file);
	if(memcmp(octet, "TERRAIN ", 8))
		return -1;

	fread(quartet, 4, 1, file);
	if(memcmp(quartet, "SIZE", 4))
		return -1;
	fread(duet, 2, 1, file);
	fseek(file, 2, SEEK_CUR);
	int size=DUET_TO_NUMBER(duet);

	int xpts=0, ypts=0;
	do
	{
		fread(quartet_marker, 4, 1, file);

		if(memcmp(quartet_marker, "XPTS", 4)==0)
		{
			fread(duet, 2, 1, file);
			fseek(file, 2, SEEK_CUR);
			xpts=DUET_TO_NUMBER(duet);
			continue;
		}

		if(memcmp(quartet_marker, "YPTS", 4)==0)
		{
			fread(duet, 2, 1, file);
			fseek(file, 2, SEEK_CUR);
			ypts=DUET_TO_NUMBER(duet);
			continue;
		}

		// I'm going to ignore the other segments so long
		// so we can leave the quartet marker test to the
		// while condition
		//if(strcmp(quartet_marker, "SIZE")) // Ignore SIZE
		//	fseek(file, 4, SEEK_CUR);

		if(memcmp(quartet_marker, "SCAL", 4)==0&&scale) // Ignore SCAL
		{
			float fscale[3];
			fread(&(fscale[0]), 4, 1, file);
			fread(&(fscale[1]), 4, 1, file);
			fread(&(fscale[2]), 4, 1, file);
			scale[0]=fscale[0];
			scale[1]=fscale[1];
			scale[2]=fscale[2];
		}

		//if(strcmp(quartet_marker, "CRAD")) // Ignore CRAD
		//	fseek(file, 4, SEEK_CUR);

		//if(strcmp(quartet_marker, "CRVM")) // Ignore CRVM
		//	fseek(file, 4, SEEK_CUR);

	}while(memcmp(quartet_marker, "ALTW", 4));

	int16_t height_scale, base_height;
	fread(duet, 2, 1, file);
	height_scale = DUET_TO_NUMBER(duet);
	fread(duet, 2, 1, file);
	base_height = DUET_TO_NUMBER(duet);

	if(xpts==0&&ypts==0)
	{
		xpts=size+1;
		ypts=size+1;
	}

	*width=xpts;
	*height=ypts;

	// The caller of this function is responsible
	// to free the memory consumed by buffer
	DUET *buffer=new DUET[xpts*ypts];
	fread(buffer, 2, xpts*ypts, file);

	*pbuffer=new double[xpts*ypts];
	for(int y=0; y<ypts; y++)
	{
		for(int x=0; x<xpts; x++)
		{
			int16_t elev = (int16_t)DUET_TO_NUMBER( buffer[y*xpts+x] );
			//memcpy(&elev, (buffer+(y*xpts+x)), 2);
			(*pbuffer)[y*xpts+x]=
				static_cast<double>(base_height+elev*height_scale/65536);
		}
	}
	delete[] buffer;

	// a last test, the next we read should be "EOF ", since currently the Terragen
	// file format says, that "ALTW" must be the last segment of a file
	// however it's no problem for _us_ if we've no "EOF " here, because it doesn't matter
	fread(quartet, 4, 1, file);
	if(memcmp(quartet, "EOF ", 4))
		printf("read terrain file with \"EOF \" at the end\n");

	fclose(file);
	return 0;
}
