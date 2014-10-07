#ifndef TERRAIN_H
#define TERRAIN_H

#include "quad.h"

#include <vector>
#include <math.h>

class Terrain : public Quad
{
public:
	double z_mean_;
	double z_mean() {
		if( isnan(z_mean_) ) {
			return z_mean_ = 
				( z[0][0] + z[0][1] +
				  z[1][0] + z[1][1] ) / 4.;
		}
		return z_mean_;
	};
	double z[2][2];

public:
	Terrain(std::vector<double>* V_) : Quad(V_) { z_mean_ = NAN; }

	virtual void split();
	virtual void track_down(double x, double y, double nz, int levels);

/*protected:
	virtual void set_range(double nx1, double nx2, double ny1, double ny2);*/
};

#endif/*TERRAIN_H*/
