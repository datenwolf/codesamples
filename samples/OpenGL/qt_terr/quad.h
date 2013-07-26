#ifndef QUAD_H
#define QUAD_H

#include <vector>

class Quad
{
public:
//	double x1,x2, x_mid;
//	double y1,y2, y_mid;
	
	unsigned int v1_offset; 
	unsigned int v2_offset;
	unsigned int v_mid_offset;

	double *x1() { return &(*V)[v1_offset + 0]; }
	double *y1() { return &(*V)[v1_offset + 1]; }
	double *z1() { return &(*V)[v1_offset + 2]; }

	double *x2() { return &(*V)[v2_offset + 0]; }
	double *y2() { return &(*V)[v2_offset + 1]; }
	double *z2() { return &(*V)[v2_offset + 2]; }
	
	double *x_mid() { return &(*V)[v_mid_offset + 0]; }
	double *y_mid() { return &(*V)[v_mid_offset + 1]; }
	double *z_mid() { return &(*V)[v_mid_offset + 2]; }

	Quad *quads[2][2];

public:
	Quad();
	Quad(Quad &q);
	~Quad();
	Quad& operator=(Quad &q);

	virtual void split();
	virtual bool is_split();

	virtual void track_down(double x, double y, int levels);
	virtual Quad *get_at(double x, double y, int max_level=0, int level=0);

	virtual void set_range(double nx1, double nx2, double ny1, double ny2);

protected:
	Quad(std::vector<double>*);
	std::vector<double> * const V;
};

#endif//QUAD_H
