#pragma once
#ifndef STATS_H
#define STATS_H

#include <math.h>

struct stats_running {
	unsigned n;
	double   S;
	double   m;
};
#define STATS_RUNNING_INIT {0, NAN, NAN}

void stats_running_reset(struct stats_running *s);
void stats_running_push(struct stats_running *s, double value);

static inline
unsigned stats_running_N(struct stats_running const *s)
{
	return s ? s->n : 0;
}

static inline
double stats_running_mean(struct stats_running const *s)
{
	return s && (0 < s->n) ? s->m : NAN;
}

static inline
double stats_running_variance(struct stats_running const *s)
{
	return s && (1 < s->n) ? s->S/s->n : NAN;
}

#endif/*STATS_H*/
