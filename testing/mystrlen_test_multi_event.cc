#include <iostream>
#include <sstream>
#include <cstring>	// std strlen
#include <functional>
#include <memory>

#include <jsplib/perf/PerfEventCount.hh>
#include <jsplib/perf/ScopedEventTrigger.hh>
#include <jsplib/perf/WallTimeEvent.hh>

#ifndef TOTALSTRINGLENGTH
#define TOTALSTRINGLENGTH (1<<30)
#endif

// g++ -O3 -Wall -Wextra ScopedPerfEventCounter_mystrlen_test_multi_event.cc -std=c++0x -DUSELONGWORDS && ./a.out

size_t mystrlen ( const char * str ) {
	size_t len = 0;
	while(*str++) ++len;
	return len;
}

int main() {

std::unique_ptr<char []> str (new char[TOTALSTRINGLENGTH]);	// deliberately not aligned - and the strings within the array will mostly be unaligned anyway.
std::fill( str.get(), str.get()+TOTALSTRINGLENGTH, 'a' );

for(unsigned j = 1, i = 1 ; i < TOTALSTRINGLENGTH ; ++j ) {
	str[i] = '\0';
#ifdef USELONGWORDS
	i += 64+(j%64);
#else
	i += 2+(j%8);
#endif
}
str[TOTALSTRINGLENGTH-1] = '\0';

using pehs = jsplib::perf::linux_perf_event_counter_t::HWSW_EVENT_T;
using pehcev = jsplib::perf::linux_perf_event_counter_t::HWCACHE_EVENT_T;
using pehcopid = jsplib::perf::linux_perf_event_counter_t::HWCACHE_OPID_EVENT_T;
using pehcopre = jsplib::perf::linux_perf_event_counter_t::HWCACHE_OPRESULT_EVENT_T;

{
	jsplib::perf::PerfEventCount pc1{ pehs::HW_CACHE_REFERENCES, pehs::HW_CACHE_MISSES, pehs::HW_BRANCH_INSTRUCTIONS, pehs::HW_BRANCH_MISSES, pehs::HW_REF_CPU_CYCLES };
	jsplib::perf::WallTimeEvent timer;
	std::cout << ">>> 1 group of 5 performance events." << std::endl;
	uintmax_t sum = 0;
	{
		jsplib::perf::ScopedEventTrigger trig2 { &timer };
		jsplib::perf::ScopedEventTrigger trig1 { &pc1 };
		unsigned j = 0;
		size_t len = 1;
		while( j < TOTALSTRINGLENGTH ) {
			len = mystrlen ( str.get() + j );
			sum += len;
			j += (len+1);
		}
	}
	std::cout << "mystrlen took: " << timer.getDuration() << '\n';
	std::cout << "sum : " << sum << '\n';
	std::cout << "HW_CACHE_REFERENCES: " << pc1.getValue( 0 ) << "\tHW_CACHE_MISSES: " << pc1.getValue( 1 ) << "\tratio: " << 100.0 * pc1.getRatio( 1, 0 ) << " %\n";
	std::cout << "HW_BRANCH_INSTRUCTIONS: " << pc1.getValue( 2 ) << "\tHW_BRANCH_MISSES: " << pc1.getValue( 3 ) << "\tratio: " << 100.0 * pc1.getRatio( 3, 2 ) << " %\n";
	std::cout << "HW_REF_CPU_CYCLES: " << pc1.getValue( 4 ) << '\n';
	std::cout << "Multiplexing scaling factor: " << pc1.getLastScaling() << "\n\n";
}

{
	jsplib::perf::PerfEventCount pc1{ pehs::HW_CACHE_REFERENCES, pehs::HW_CACHE_MISSES, pehs::HW_BRANCH_INSTRUCTIONS, pehs::HW_BRANCH_MISSES, {pehcev::HW_CACHE_L1D,pehcopid::HW_CACHE_OP_PREFETCH,pehcopre::HW_CACHE_RESULT_MISS} };
	jsplib::perf::WallTimeEvent timer;
	std::cout << ">>> 1 group of 5 performance events, swapping HW_REF_CPU_CYCLES for a cache event." << std::endl;
	uintmax_t sum = 0;
	{
		jsplib::perf::ScopedEventTrigger trig { &pc1, &timer };
		unsigned j = 0;
		size_t len = 1;
		while( j < TOTALSTRINGLENGTH ) {
			len = mystrlen ( str.get() + j );
			sum += len;
			j += (len+1);
		}
	}
	std::cout << "mystrlen took: " << timer.getDuration() << '\n';
	std::cout << "sum : " << sum << '\n';
	std::cout << "HW_CACHE_REFERENCES: " << pc1.getValue( 0 ) << "\tHW_CACHE_MISSES: " << pc1.getValue( 1 ) << "\tratio: " << 100.0 * pc1.getRatio( 1, 0 ) << " %\n";
	std::cout << "HW_BRANCH_INSTRUCTIONS: " << pc1.getValue( 2 ) << "\tHW_BRANCH_MISSES: " << pc1.getValue( 3 ) << "\tratio: " << 100.0 * pc1.getRatio( 3, 2 ) << " %\n";
	std::cout << "L1D_PREFETCH_MISSES: " << pc1.getValue( 4 ) << '\n';
	std::cout << "Multiplexing scaling factor: " << pc1.getLastScaling() << "\n\n";
}


{
	jsplib::perf::PerfEventCount pc1{ pehs::HW_CACHE_REFERENCES, pehs::HW_CACHE_MISSES, {pehcev::HW_CACHE_L1D,pehcopid::HW_CACHE_OP_PREFETCH,pehcopre::HW_CACHE_RESULT_MISS} };
	jsplib::perf::PerfEventCount pc2{ pehs::HW_BRANCH_INSTRUCTIONS, pehs::HW_BRANCH_MISSES, pehs::HW_REF_CPU_CYCLES };
	jsplib::perf::WallTimeEvent timer;
	std::cout << ">>> 2 groups of 3 and 3 performance events, including the cache event and HW_REF_CPU_CYCLES. Triggered by two ScopedEventTriggers." << std::endl;
	uintmax_t sum = 0;
	{
		jsplib::perf::ScopedEventTrigger trig1 { &pc1 };
		jsplib::perf::ScopedEventTrigger trig2 { &pc2 };
		jsplib::perf::ScopedEventTrigger trig3 { &timer };
		unsigned j = 0;
		size_t len = 1;
		while( j < TOTALSTRINGLENGTH ) {
			len = mystrlen ( str.get() + j );
			sum += len;
			j += (len+1);
		}
	}
	std::cout << "mystrlen took: " << timer.getDuration() << '\n';
	std::cout << "sum : " << sum << '\n';
	std::cout << "HW_CACHE_REFERENCES: " << pc1.getValue( 0 ) << "\tHW_CACHE_MISSES: " << pc1.getValue( 1 ) << "\tratio: " << 100.0 * pc1.getRatio( 1, 0 ) << " %\n";
	std::cout << "HW_BRANCH_INSTRUCTIONS: " << pc2.getValue( 0 ) << "\tHW_BRANCH_MISSES: " << pc2.getValue( 1 ) << "\tratio: " << 100.0 * pc2.getRatio( 1, 0 ) << " %\n";
	std::cout << "L1D_PREFETCH_MISSES: " << pc2.getValue( 0 ) << '\n';
	std::cout << "HW_REF_CPU_CYCLES: " << pc2.getValue( 1 ) << '\n';
	std::cout << "Multiplexing scaling factor for pc1: " << pc1.getLastScaling() << '\n';
	std::cout << "Multiplexing scaling factor for pc2: " << pc2.getLastScaling() << "\n\n";
}

{
	jsplib::perf::PerfEventCount pc1{ pehs::HW_CACHE_REFERENCES, pehs::HW_CACHE_MISSES, {pehcev::HW_CACHE_L1D,pehcopid::HW_CACHE_OP_PREFETCH,pehcopre::HW_CACHE_RESULT_MISS} };
	jsplib::perf::PerfEventCount pc2{ pehs::HW_BRANCH_INSTRUCTIONS, pehs::HW_BRANCH_MISSES, pehs::HW_REF_CPU_CYCLES };
	jsplib::perf::WallTimeEvent timer;
	std::cout << ">>> 2 groups of 3 and 3 performance events, including the cache event and HW_REF_CPU_CYCLES. Triggered by a single ScopedEventTrigger." << std::endl;
	uintmax_t sum = 0;
	{
		jsplib::perf::ScopedEventTrigger trig1 { &timer, &pc1, &pc2 };
		unsigned j = 0;
		size_t len = 1;
		while( j < TOTALSTRINGLENGTH ) {
			len = mystrlen ( str.get() + j );
			sum += len;
			j += (len+1);
		}
	}
	std::cout << "mystrlen took: " << timer.getDuration() << '\n';
	std::cout << "sum : " << sum << '\n';
	std::cout << "HW_CACHE_REFERENCES: " << pc1.getValue( 0 ) << "\tHW_CACHE_MISSES: " << pc1.getValue( 1 ) << "\tratio: " << 100.0 * pc1.getRatio( 1, 0 ) << " %\n";
	std::cout << "HW_BRANCH_INSTRUCTIONS: " << pc2.getValue( 0 ) << "\tHW_BRANCH_MISSES: " << pc2.getValue( 1 ) << "\tratio: " << 100.0 * pc2.getRatio( 1, 0 ) << " %\n";
	std::cout << "L1D_PREFETCH_MISSES: " << pc2.getValue( 0 ) << '\n';
	std::cout << "HW_REF_CPU_CYCLES: " << pc2.getValue( 1 ) << '\n';
	std::cout << "Multiplexing scaling factor for pc1: " << pc1.getLastScaling() << '\n';
	std::cout << "Multiplexing scaling factor for pc2: " << pc2.getLastScaling() << "\n\n";
}


{
	jsplib::perf::PerfEventCount pc1{ pehs::HW_CACHE_REFERENCES, pehs::HW_CACHE_MISSES, {pehcev::HW_CACHE_L1D,pehcopid::HW_CACHE_OP_PREFETCH,pehcopre::HW_CACHE_RESULT_MISS} };
	jsplib::perf::PerfEventCount pc2{ pehs::HW_BRANCH_INSTRUCTIONS, pehs::HW_BRANCH_MISSES, pehs::HW_REF_CPU_CYCLES };
	jsplib::perf::WallTimeEvent timer;
	std::cout << ">>> 2 groups of 3 and 3 performance events, including the cache event and HW_REF_CPU_CYCLES. ." << std::endl;
	uintmax_t sum = 0;
	{
		jsplib::perf::ScopedEventTrigger trig1 { &timer, &pc1 };
		unsigned j = 0;
		size_t len = 1;
		while( j < TOTALSTRINGLENGTH ) {
			len = mystrlen ( str.get() + j );
			sum += len;
			j += (len+1);
		}
	}
	std::cout << "mystrlen took: " << timer.getDuration() << '\n';
	timer.reset();
	std::cout << "sum : " << sum << '\n';
	sum = 0;
	{
		jsplib::perf::ScopedEventTrigger trig1 { &timer, &pc2 };
		unsigned j = 0;
		size_t len = 1;
		while( j < TOTALSTRINGLENGTH ) {
			len = mystrlen ( str.get() + j );
			sum += len;
			j += (len+1);
		}
	}
	std::cout << "mystrlen took: " << timer.getDuration() << '\n';
	std::cout << "sum : " << sum << '\n';

	std::cout << "HW_CACHE_REFERENCES: " << pc1.getValue( 0 ) << "\tHW_CACHE_MISSES: " << pc1.getValue( 1 ) << "\tratio: " << 100.0 * pc1.getRatio( 1, 0 ) << " %\n";
	std::cout << "HW_BRANCH_INSTRUCTIONS: " << pc2.getValue( 0 ) << "\tHW_BRANCH_MISSES: " << pc2.getValue( 1 ) << "\tratio: " << 100.0 * pc2.getRatio( 1, 0 ) << " %\n";
	std::cout << "L1D_PREFETCH_MISSES: " << pc2.getValue( 0 ) << '\n';
	std::cout << "HW_REF_CPU_CYCLES: " << pc2.getValue( 1 ) << '\n';
	std::cout << "Multiplexing scaling factor for pc1: " << pc1.getLastScaling() << '\n';
	std::cout << "Multiplexing scaling factor for pc2: " << pc2.getLastScaling() << "\n\n";
}



}
