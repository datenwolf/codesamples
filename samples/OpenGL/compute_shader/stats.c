#include "stats.h"

void stats_running_reset(struct stats_running *s)
{
	if( s ) {
		s->n = 0;
		s->S = NAN;
		s->m = NAN;
	}
}

void stats_running_push(struct stats_running *s, double value)
{
	if( s && isfinite(value) ) {
		double   const m_prev = 0 < s->n ? s->m : 0.;
		double   const S_prev = 1 < s->n ? s->S : 0.;
		unsigned const n      = (s->n += 1);

		s->m = m_prev + (value - m_prev) / n;
		if( 1 < n ) {
			/* variance is defined only for n > 1 */
			s->S = S_prev + (value - s->m) * (value - m_prev);
		}
	}
}
