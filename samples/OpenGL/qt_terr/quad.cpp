#include <stdlib.h>
#include "quad.h"

Quad::Quad() :
	V(new std::vector<double>)
{
	quads[0][0] =
	quads[0][1] =
	quads[1][0] =
	quads[1][1] = 0;

	unsigned int V_old_size = V->size();
	unsigned int V_new_size = V_old_size + 9;
	V->resize(V_new_size, 0.);

	v1_offset = V_new_size - 3*3;
	v2_offset = V_new_size - 2*3;
	v_mid_offset = V_new_size - 1*3;

	set_range(0., 1., 0., 1.);
}

Quad::Quad( std::vector<double> * const V_) :
	V(V_)
{
	quads[0][0] =
	quads[0][1] =
	quads[1][0] =
	quads[1][1] = 0;

	unsigned int V_old_size = V->size();
	unsigned int V_new_size = V_old_size + 9;
	V->resize(V_new_size, 0.);

	v1_offset = V_new_size - 3*3;
	v2_offset = V_new_size - 2*3;
	v_mid_offset = V_new_size - 1*3;

	set_range(0., 1., 0., 1.);
}

Quad::Quad(Quad &q) :
	V(new std::vector<double>)
{
	operator= (q);
}

Quad::~Quad()
{
	if(is_split())
	{
		delete quads[0][0];
		delete quads[0][1];
		delete quads[1][0];
		delete quads[1][1];
	}
}

Quad& Quad::operator=(Quad &q)
{
	*x1() = *q.x1();
	*x2() = *q.x2();
	*y1() = *q.y1();
	*y2() = *q.y2();

	if( q.is_split() )
	{
		split();		
		quads[0][0] = q.quads[0][0];
		quads[0][1] = q.quads[0][1];
		quads[1][0] = q.quads[1][0];
		quads[1][1] = q.quads[1][1];
	}

	return *this;
}

void Quad::split()
{
	if(is_split())
		return;

	quads[0][0] = new Quad(V);
	quads[0][1] = new Quad(V);
	quads[1][0] = new Quad(V);
	quads[1][1] = new Quad(V);
	
	quads[0][0]->set_range( *x1(), *x_mid(), *y1(), *y_mid() );
	quads[0][1]->set_range( *x_mid(), *x2(), *y1(), *y_mid() );
	quads[1][0]->set_range( *x1(), *x_mid(), *y_mid(), *y2() );
	quads[1][1]->set_range( *x_mid(), *x2(), *y_mid(), *y2() );
}

bool Quad::is_split()
{
	return	quads[0][0] &&
	        quads[0][1] &&
	        quads[1][0] &&
	        quads[1][1];
}

void Quad::set_range(double nx1, double nx2, double ny1, double ny2)
{
	*x1() = nx1;
	*x2() = nx2;
	*y1() = ny1;
	*y2() = ny2;
	*x_mid() = ( *x1() + *x2() )*0.5f;
	*y_mid() = ( *y1() + *y2() )*0.5f;

}

void Quad::track_down(double x, double y, int levels)
{
	if(levels>0)
	{
		int a = ( x < *x_mid() ) ? 0 : 1;
		int b = ( y < *y_mid() ) ? 0 : 1;

		if( !is_split() )
			split();

		quads[b][a]->track_down(x, y, levels-1);
	}
}

Quad *Quad::get_at(double x, double y, int max_level, int level)
{
	if(!is_split())
		return this;

	/*if(max_level>-1 && level>=max_level)
		return this;*/

	int a = ( x < *x_mid() ) ? 0 : 1;
	int b = ( y < *y_mid() ) ? 0 : 1;

	return quads[b][a]->get_at(x, y, max_level, level+1);
}

