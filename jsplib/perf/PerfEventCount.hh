#ifndef _JSPLIBPERFSCOPEDPERFCOUNTER_H_
#define _JSPLIBPERFSCOPEDPERFCOUNTER_H_

#include <iostream>
#include <stdexcept>
#include <initializer_list>
#include <vector>
#include <iterator>
#include <algorithm> // copy_if
#include <memory>
#include <utility>	// make_pair

#include <array>	//std::array

#include <cstdlib>
#include <cstring>	// memset, strerror

#include <cassert>

#include "EventBase.hh"	// EventBase

#include "../pp/Stringify.hh"

extern "C" {
#include <unistd.h>
#include <sys/ioctl.h>
#include <linux/perf_event.h>
#include <asm/unistd.h>
#include <linux/types.h>

static long perf_event_open(struct perf_event_attr *hw_event, pid_t pid, int cpu, int group_fd, unsigned long flags) {
   return  syscall(__NR_perf_event_open, hw_event, pid, cpu, group_fd, flags);
}

}

// TODO reimplement groups - if users want many events they can have multiple PerfEventcount objects
// http://sandsoftwaresound.net/perf/perf-tutorial-hot-spots/
// http://sandsoftwaresound.net/perf/perf-tut-profile-hw-events/
// http://www.spinics.net/lists/linux-perf-users/msg03122.html

// TODO this shouldn't be header only because if DISABLEJSPLIBPERFEVENTCOUNTER is set in one compile, and not another then ??

// README, design: It's deisgned as a C++ tenmpalte to maximise compiler time checking and to adapt the generated class as much as possible to the paraemeters, so since it is known how many events are to be monitored by a PerfEventCount there's no reason to hold a static array with a #define'able array size, or, worse, a vector.  By using the template the class is just as big as it has to be, and there's also no need for a variable to hold the length, and for loops can also be potentially unrolled.  The drawback is ofcourse code bloat.

namespace jsplib { namespace perf {
/*!
\class PerfEventCount
\brief A simple class to do scoped Linux Performance Events/PMU data capture

\code
TODO fix this
jsplib::PerfEventCount<2> pc1{ pehs::HW_CACHE_REFERENCES, pehs::HW_CACHE_MISSES };
{
	jsplib::ScopedPerfEventTrigger<1> trig ( pc1 );
	// some code here for which cache references and misses are to be monitored
}
std::cout << "HW_CACHE_REFERENCES : " << pc1.getValue( 0 ) << "\tHW_CACHE_MISSES : " << pc1.getValue( 1 ) << "\tratio: " << 100.0 * pc1.getRatio( 1, 0 ) << " %\t sum : " << sum << "\n\n";

\endcode
*/

#define EVENTTYPESWITCHCASE(SCOPE, TYPESTRING, CONFIGSTRING) case SCOPE::CONFIGSTRING: m_type = TYPESTRING; m_config = PERF_COUNT_ ## CONFIGSTRING; break;
#define EVENTCOUNTSWITCHCASE(VARNAME, SCOPE, TYPESTRING) case SCOPE::TYPESTRING: VARNAME = PERF_COUNT_ ## TYPESTRING; break;

#define MAKE_HW_CACHE_CONFIG(perf_hw_cache_id,perf_hw_cache_op_id,perf_hw_cache_op_result_id) ((perf_hw_cache_id) | (perf_hw_cache_op_id << 8) | (perf_hw_cache_op_result_id << 16))

// TODO: create an std::map< std::pair<config,type> , std::string > which is [always] set in EVENTCOUNTSWITCHCASE above
// event container must return the pair when asked by index - and there must ba a free function to do lookup in map

struct linux_perf_event_counter_t {
	__u64 m_config;
	__u64 m_count;
	__u32 m_type;
	float m_last_read_scaling_factor;
	int m_fd;
	bool m_noop;

	const static std::pair< std::pair<__u32,__u64>, const char * > event_names_lut[];

#include "internals/HWSW_EVENT_T.enum.snippet"
#include "internals/HWCACHE_EVENT_T.enum.snippet"
#include "internals/HWCACHE_OPID_EVENT_T.enum.snippet"
#include "internals/HWCACHE_OPRESULT_EVENT_T.enum.snippet"


	linux_perf_event_counter_t (): m_config(0), m_count(0), m_type(0), m_last_read_scaling_factor(0.0), m_fd(-1), m_noop(true) { }
	
	linux_perf_event_counter_t (HWSW_EVENT_T event_type): m_config(0), m_count(0), m_type(0), m_last_read_scaling_factor(0.0), m_fd(-1), m_noop(false) {
		switch(event_type) {
#include "internals/HWSW_EVENT_T.cases.snippet"
		}
	}

	linux_perf_event_counter_t (HWCACHE_EVENT_T cache_event, HWCACHE_OPID_EVENT_T cache_opid, HWCACHE_OPRESULT_EVENT_T cache_opresult ) : m_config(0), m_count(0), m_type(PERF_TYPE_HW_CACHE), m_last_read_scaling_factor(0.0), m_fd(-1), m_noop(false) {
		__u64 perf_hw_cache_id = 0;
		__u64 perf_hw_cache_op_id = 0;
		__u64 perf_hw_cache_op_result_id = 0;
		switch (cache_event) {
#include "internals/HWCACHE_EVENT_T.cases.snippet"
		}
		switch (cache_opid) {
#include "internals/HWCACHE_OPID_EVENT_T.cases.snippet"
		}
		switch (cache_opresult) {
#include "internals/HWCACHE_OPRESULT_EVENT_T.cases.snippet"
		}
		m_config = MAKE_HW_CACHE_CONFIG(perf_hw_cache_id, perf_hw_cache_op_id, perf_hw_cache_op_result_id);	
	}

	linux_perf_event_counter_t ( __u64 raw_config ) : m_config(raw_config), m_count(0), m_type(PERF_TYPE_RAW), m_last_read_scaling_factor(0.0), m_fd(-1), m_noop(false) {
	}

	bool sameEvent ( const linux_perf_event_counter_t & other ) const {
		return ((m_type==other.m_type)&&(m_config==other.m_config));
	}

	const std::string lookup_description() const {
		switch(m_type) {
			case PERF_TYPE_HARDWARE :
			case PERF_TYPE_SOFTWARE :
			case PERF_TYPE_HW_CACHE :
				for ( size_t i = 0; event_names_lut[i].second[0] != '\0'; ++i ) {
					if( event_names_lut[i].first == std::make_pair(m_type,m_config) )
						return event_names_lut[i].second;
				}
				break;
			case PERF_TYPE_RAW :
				std::string result ("PERF_TYPE_RAW");
				result += std::to_string(m_config);
				return result;
		}
	return "";
	}

};

const std::pair< std::pair<__u32,__u64>, const char * > linux_perf_event_counter_t::event_names_lut[] = {

#include "internals/event_names_lut.snippet"

};

#ifndef PERFEVENTCOUNTMAXEVENTS
#define PERFEVENTCOUNTMAXEVENTS 8
#endif


class PerfEventCount : public EventBase {

protected:

std::array < linux_perf_event_counter_t, PERFEVENTCOUNTMAXEVENTS > m_evs;
unsigned m_nevs;
float m_last_read_scaling_factor;
bool m_enabled;		// true if it is currently running ie between a start() and stop()
bool m_triggered;	// true if it has ever been triggered ie whether start() has ever been called

public:

PerfEventCount( std::initializer_list<linux_perf_event_counter_t> evs ) : m_nevs(evs.size()), m_last_read_scaling_factor(0.0), m_enabled(false), m_triggered(false) {

	assert(evs.size() <= PERFEVENTCOUNTMAXEVENTS);
	
/*
std::cout << "m_read_buf_size is " << m_read_buf_size << std::endl;
std::cout << "sizeof(read_format_group) is " << sizeof(read_format_group) << std::endl;
std::cout << "sizeof(read_format_group_counter) is " << sizeof(read_format_group_counter) << std::endl;
*/
	std::copy_if( evs.begin(), evs.end(), m_evs.begin(), [](const linux_perf_event_counter_t & e){ return (!e.m_noop ); } );
	// check for duplicates
	for(unsigned i = 0; i < m_nevs; ++i) {
		for(unsigned j = i+1; j < m_nevs; ++j)
			if ( m_evs[i].sameEvent(m_evs[j]) ) throw std::invalid_argument("Duplicate event types discovered in CPerfEventCount::CPerfEventCount( ... )");
	}

	int group_fd = -1;
	for(unsigned i = 0; i < m_nevs; ++i) {
		struct perf_event_attr pe;
		memset( &pe, 0, sizeof(struct perf_event_attr) );
		pe.type = m_evs[i].m_type;
		pe.size = sizeof(struct perf_event_attr);
		pe.config = m_evs[i].m_config;
		pe.disabled = 1;
		pe.exclude_kernel = 1;
		pe.exclude_hv = 1;
		pe.read_format = PERF_FORMAT_TOTAL_TIME_ENABLED | PERF_FORMAT_TOTAL_TIME_RUNNING | PERF_FORMAT_ID | PERF_FORMAT_GROUP;
		int fd = perf_event_open ( &pe, 0, -1, group_fd, 0 );
		if (fd == -1) {
			throw std::invalid_argument(std::string("Error opening event with config ") + std::to_string(pe.config) + ". Description is \"" + getDescription(i) +  "\". Error was \"" + strerror(errno) + "\"");
			}
		if (group_fd==-1) group_fd = fd;
		m_evs[i].m_fd = fd;
		m_evs[i].m_count = 0;
	}
	if(m_nevs) ioctl ( m_evs[0].m_fd, PERF_EVENT_IOC_RESET, PERF_IOC_FLAG_GROUP );
}
// TODO make PerfEventCount copyable so it can do split timing?
PerfEventCount(PerfEventCount const &) = delete;
PerfEventCount& operator= ( PerfEventCount const & ) = delete;

// TODO a meaningful PerfEventCount& operator=(PerfEventCount&& other)

~PerfEventCount() {
	assert(!m_enabled);
	for(unsigned i = 0; i < m_nevs; ++i) {
		// TODO check each one to see if they are running?
		close(m_evs[i].m_fd);
	}
}

void start() {
	assert(!m_enabled);
	m_triggered = true;
	m_enabled = true; 
	if(m_nevs)
		if (-1 == ioctl(m_evs[0].m_fd, PERF_EVENT_IOC_ENABLE, PERF_IOC_FLAG_GROUP) )
			throw std::invalid_argument(std::string("Error enabling event group. Error was \"") + strerror(errno) + "\"");
}

void stop() {

	struct read_format_group {
		__u64 nr;
		__u64 time_enabled;
		__u64 time_running;
		struct {
				__u64 value;
				__u64 id;
		} cntr[PERFEVENTCOUNTMAXEVENTS];
	};
	
	if(m_nevs)
		if( -1 == ioctl(m_evs[0].m_fd, PERF_EVENT_IOC_DISABLE, PERF_IOC_FLAG_GROUP) )
			throw std::invalid_argument(std::string("Error disabling event group. Error was \"") + strerror(errno) + "\"");

	assert(m_enabled);
	m_enabled = false;

	read_format_group rgrp;
	memset( &rgrp, 0, sizeof(struct read_format_group) );
	rgrp.nr = m_nevs;

	if ( read ( m_evs[0].m_fd, &rgrp, sizeof(read_format_group) ) < 0 )
		throw std::invalid_argument(std::string("Error reading event group. Error was \"") + strerror(errno) + "\"");

	assert ( rgrp.nr == m_nevs );

	m_last_read_scaling_factor = (static_cast<float>(rgrp.time_running)/static_cast<float>(rgrp.time_enabled));
//	std::cout << "time_running delta = " << rgrp.time_running << " time_enabled delta = " << rgrp.time_enabled << '\n';
	
	for(unsigned i = 0; i < m_nevs; ++i) {
		m_evs[i].m_count += rgrp.cntr[i].value/m_last_read_scaling_factor;
	}
	if(m_nevs)
		if ( -1 == ioctl(m_evs[0].m_fd, PERF_EVENT_IOC_RESET, PERF_IOC_FLAG_GROUP) )
			throw std::invalid_argument(std::string("Error reseting event group after disabling. Error was \"") + strerror(errno) + "\"");
}

double getRatio ( const unsigned numerator_index, const unsigned denominator_index ) const {
	assert ( numerator_index < m_nevs );
	assert ( denominator_index < m_nevs );
//	assert ( m_triggered );
	return ( static_cast<double> ( m_evs[numerator_index].m_count ) / static_cast<double> ( m_evs[denominator_index].m_count) );	// TODO test for div0??
}

__u64 getValue ( const unsigned counter_index ) const {
	assert(counter_index < m_nevs);
//	assert(m_triggered);
	return m_evs[counter_index].m_count;
}

void reset() {	// TODO check if running ?
	for(unsigned i = 0; i < m_nevs; ++i) m_evs[i].m_count = 0;
}

float getLastScaling() const {
	assert(m_triggered);
	return m_last_read_scaling_factor;
	}
	
bool lastReadingScaled() const {
	assert(m_triggered);
	return (1.0 != m_last_read_scaling_factor);
	}

unsigned getNumEvents() const {
	return m_nevs;
	}

const std::string getDescription( const unsigned counter_index ) const {
	assert(counter_index < m_nevs);
	return m_evs[counter_index].lookup_description();
}

};

// TODO add move contrustors.
// TODO disable heap allocation for ScopedPerfEventTrigger
} } // namespace jsplib::perf

#endif	// _JSPLIBPERFSCOPEDPERFCOUNTER_H_
