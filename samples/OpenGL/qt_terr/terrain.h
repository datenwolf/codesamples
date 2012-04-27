#ifndef TERRAIN_H
#define TERRAIN_H

#include "quad.h"

#include <vector>

class Terrain : public Quad
{
public:
	double * const z_mean() {
		
	};
	double z[2][2];

public:
	Terrain();

	virtual void split();
	virtual void track_down(double x, double y, double nz, int levels);

/*protected:
	virtual void set_range(double nx1, double nx2, double ny1, double ny2);*/
};

#endif/*TERRAIN_H*/
