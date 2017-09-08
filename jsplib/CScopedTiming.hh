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

#ifndef _JSPLIBCSCOPEDTIMING_H_
#define _JSPLIBCSCOPEDTIMING_H_ 1

#ifdef _WIN32
#include <windows.h>
#else
#include <sys/time.h>
#endif

#include <iostream>
#include <string>
#include <cstring>

#define MAXTIMERNAMELEN 64

namespace jsplib {
/*!
\class CScopedTiming
\brief A simple class to do scoped timing

Upon creation the timer will start, on destruction the double specified in the constructor will be updated with the time in seconds that the object existed.
If compiled under win32 then this is only accurate to 10ms or 16ms, under linux accuracy is dependant on the HZ kernel parameter
\code
double dur;
{
jsplib::CScopedTiming timer(dur);
some_operations_to_time();
}
std::cout << "some_operations_to_time() took " << dur << " seconds to complete." << std::endl;
\endcode
*/
class CScopedTiming {

private:
double * m_out;
char m_name [MAXTIMERNAMELEN+1];
std::ostream * m_os;
#ifdef _WIN32
DWORD m_start;
#else
struct timeval m_start;
#endif

// prevent dynamic instantion so this class so it can only created on the stack
static void *operator new(size_t size);
static void operator delete(void *ptr);
static void *operator new[](size_t size);
static void operator delete[](void *ptr);
CScopedTiming( const CScopedTiming & );	// non-copyable
// TODO maybe make this copyable to be able to take split times? but then we need a new way to return values since it can't go into m_out

public:
/// Creates a scoped timing object
/** The timer is started on construction
 * \param out The variable to which to add the elapsed time upon destruction of this object
 */
CScopedTiming( double & out ):m_out(&out) {
	m_name[0] = '\0';
#ifdef _WIN32
	m_start = GetTickCount();
#else
	gettimeofday(&m_start,NULL);
#endif
	}

CScopedTiming( const char * name ):m_out(0),m_os(0) {
	strncpy( m_name, name, MAXTIMERNAMELEN );
#ifdef _WIN32
	m_start = GetTickCount();
#else
	gettimeofday(&m_start,NULL);
#endif
	}

CScopedTiming( const char * name, std::ostream & os):m_out(0),m_os(&os)  {
	strncpy( m_name, name, MAXTIMERNAMELEN );
	m_name[MAXTIMERNAMELEN-1] = '\0';
#ifdef _WIN32
	m_start = GetTickCount();
#else
	gettimeofday(&m_start,NULL);
#endif
	}

/// The destructor - the timer is stopped and the time elapsed is stored in the parameter specified in the constructor or printed with the tag specified in the constructor
~CScopedTiming() {
	double duration = 0;
#ifdef _WIN32
	duration = ((double)(GetTickCount() - m_start))/1000.0;
#else
	struct timeval end, diff;
	gettimeofday ( &end, NULL );
	timersub ( &end, &m_start, &diff );
	duration = (double)diff.tv_sec+((double)diff.tv_usec/1000000.0);
#endif
	if(m_out) { *m_out += duration; }
	else { (m_os?*m_os:std::cout) << m_name << ": " << duration << " seconds.\n"; }
	}
};

} // namespace jsplib

#endif	// _JSPLIBCSCOPEDTIMING_H_
