/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 4; tab-width: 4 -*- */
/*
 * mitie
 * Copyright (C) Jason Spencer 2009 <mitie@jasonspencer.org>
 * 
 * mitie is free software: you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 * 
 * mitie is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License along
 * with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef _JSPLIBPERFWALLTIMEVENT_H_
#define _JSPLIBPERFWALLTIMEVENT_H_

#include <sys/time.h>

namespace jsplib { namespace perf {

class WallTimeEvent : public EventBase {

private:

struct timeval m_start;
double m_total_duration;
bool m_enabled;
bool m_triggered;

// WallTimeEvent( const WallTimeEvent & );	// non-copyable

public:

WallTimeEvent(): m_total_duration(0.0), m_enabled(false), m_triggered(false) { }

void start() {
	assert(!m_enabled);
	m_triggered = true;
	m_enabled = true;
	gettimeofday(&m_start,NULL);
}

void stop() {
	struct timeval end, diff;
	gettimeofday ( &end, NULL );
	assert(m_enabled);
	assert(m_triggered);
	timersub ( &end, &m_start, &diff );
	m_total_duration += (double)diff.tv_sec+((double)diff.tv_usec/1000000.0);
	m_enabled = false;
}

void reset() {
	assert(!m_enabled);
//	memset( &m_start, 0, sizeof(struct timeval) );
	m_total_duration = 0.0;
}

double getDuration() const {
	assert(!m_enabled);
	assert(m_triggered);
	return m_total_duration;
}

~WallTimeEvent() {
	assert(!m_enabled);
	}

WallTimeEvent(WallTimeEvent const &) = delete;
WallTimeEvent& operator= ( WallTimeEvent const & ) = delete;


};

} } // namespace jsplib::perf

#endif	// _JSPLIBPERFWALLTIMEVENT_H_
