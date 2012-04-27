#include "terrain.h"

Terrain::Terrain()
{
	z_mean=0.0;
}

void Terrain::split()
{
	if(is_split())
		return;

	quads[0][0] = new Terrain(V);
	quads[0][1] = new Terrain(V);
	quads[1][0] = new Terrain(V);
	quads[1][1] = new Terrain(V);

	quads[0][0]->set_range(x1, x_mid, y1, y_mid);
	quads[0][1]->set_range(x_mid, x2, y1, y_mid);
	quads[1][0]->set_range(x1, x_mid, y_mid, y2);
	quads[1][1]->set_range(x_mid, x2, y_mid, y2);
}

void Terrain::track_down(double x, double y, double nz, int levels)
{
	if(levels>0)
	{
		int a=(x<x_mid)?0:1;
		int b=(y<y_mid)?0:1;

		if(!is_split())
			split();

		quads[b][a]->track_down(x, y, nz, levels-1);
	}
	else
	{
		z[0][0]=
		z[0][1]=
		z[1][1]=
		z[1][0]=z_mean=nz;
	}
}
