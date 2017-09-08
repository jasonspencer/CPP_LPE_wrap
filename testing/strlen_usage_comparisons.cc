#include <iostream>
#include <sstream>
#include <cstring>	// std strlen
#include <functional>
#include <xmmintrin.h>	// _mm_prefetch

#include <jsplib/perf/PerfEventCount.hh>
#include <jsplib/perf/ScopedEventTrigger.hh>
#include <jsplib/CScopedTiming.hh>

#ifndef DISABLESSE4STRLEN
#include <nmmintrin.h>	// _mm_cmpistri
#endif

#include <jsplib/pp/Stringify.hh>

#ifndef TOTALSTRINGLENGTH
#define TOTALSTRINGLENGTH (1<<20)
#endif

#ifndef FIRSTMETRIC
#define FIRSTMETRIC linux_perf_event_counter_t::HWSW_EVENT_T::HW_CACHE_REFERENCES
#endif

#ifndef SECONDMETRIC
#define SECONDMETRIC linux_perf_event_counter_t::HWSW_EVENT_T::HW_CACHE_MISSES
#endif

#ifndef THIRDMETRIC
#define THIRDMETRIC linux_perf_event_counter_t::HWSW_EVENT_T::HW_BRANCH_INSTRUCTIONS
#endif

#ifndef FOURTHMETRIC
#define FOURTHMETRIC linux_perf_event_counter_t::HWSW_EVENT_T::HW_BRANCH_MISSES
#endif

#ifndef FIFTHMETRIC
#define FIFTHMETRIC {linux_perf_event_counter_t::HWCACHE_EVENT_T::HW_CACHE_L1D,linux_perf_event_counter_t::HWCACHE_OPID_EVENT_T::HW_CACHE_OP_PREFETCH,linux_perf_event_counter_t::HWCACHE_OPRESULT_EVENT_T::HW_CACHE_RESULT_MISS}
#endif


// to build: g++ -O3 -Wall -Wextra ScopedPerfEventCounter_strlenTest1.cc -std=c++0x -msse4.2
// optionally -DDISABLESSE4STRLEN and then -msse4.2 isn't required.
// optionally set FIRSTMETRIC and SECONDMETRIC to monitor other events: -DFIRSTMETRIC=HW_BRANCH_INSTRUCTIONS -DSECONDMETRIC=HW_BRANCH_MISSES

// http://www.stdlib.net/~colmmacc/strlen.c.html

size_t mystrlen ( const char * str ) {
	size_t len = 0;
	while(*str++) ++len;
	return len;
}

size_t bsdstrlen ( const char * str ) {
	const char *s;
	for (s = str; *s; ++s) ;
	return (s - str);
}

template <typename T, size_t N> size_t arraySize(T (&)[N]) { return N; }


int main() {

using namespace jsplib::perf;

std::unique_ptr<char []> str (new char[TOTALSTRINGLENGTH]);	// deliberately not aligned - and the strings within the array be unaligned anyway.
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

jsplib::perf::PerfEventCount pc1{ FIRSTMETRIC, SECONDMETRIC, THIRDMETRIC, FOURTHMETRIC, FIFTHMETRIC };

uintmax_t sum = 0;
std::cout << '\n';

std::tuple < const char *, std::function<size_t(const char *)> > functions_to_test [] = {
	std::make_tuple( "mystrlen", mystrlen ),
	std::make_tuple( "bsdstrlen", bsdstrlen ),
	std::make_tuple( "std::strlen", std::strlen ),
	};

for(unsigned i = 0; i < arraySize(functions_to_test); ++i ) {
//for(unsigned i = 0; i < (sizeof(functions_to_test)/sizeof(functions_to_test)[0]); ++i ) {
	std::function<size_t(const char *)> f;
	const char * p;

	pc1.reset();
	sum = 0;

	{
		std::tie(p,f) = functions_to_test[i];
		std::string s (p);
		s += " with measurement outside of loop took";
		jsplib::CScopedTiming timer(s.c_str());
		jsplib::perf::ScopedEventTrigger trig { &pc1 };
		size_t len = 1;
		unsigned j = 0;
		while( j < TOTALSTRINGLENGTH ) {
			len = f(str.get()+j);
			sum += len;
			j += (len+1);
		}
	}
	std::cout << "sum : " << sum << "\n";
	std::cout << STRINGIFY(FIRSTMETRIC) << " : " << pc1.getValue( 0 ) << '\t' << STRINGIFY(SECONDMETRIC) << " : " << pc1.getValue( 1 ) << "\tratio: " << 100.0 * pc1.getRatio( 1, 0 ) << " %\n";
	std::cout << STRINGIFY(THIRDMETRIC) << " : " << pc1.getValue( 2 ) << '\t' << STRINGIFY(FOURTHMETRIC) << " : " << pc1.getValue( 3 ) << "\tratio: " << 100.0 * pc1.getRatio( 3, 2 ) << " %\n";
	std::cout << "fifthmetric: " << pc1.getValue( 4 ) << '\n';
	std::cout << "Multiplexing scaling factor: " << pc1.getLastScaling() << '\n';
//	std::cout << '\n';

	pc1.reset();
	sum = 0;
	{
		std::tie(p,f) = functions_to_test[i];
		std::string s (p);
		s += " with measurement inside of loop took";
		jsplib::CScopedTiming timer(s.c_str());
		size_t len = 1;
		unsigned j = 0;
		while( j < TOTALSTRINGLENGTH ) {
			jsplib::perf::ScopedEventTrigger trig { &pc1 };
			len = f(str.get()+j);
			sum += len;
			j += (len+1);
		}
	}
	std::cout << "sum : " << sum << "\n";
	std::cout << STRINGIFY(FIRSTMETRIC) << " : " << pc1.getValue( 0 ) << '\t' << STRINGIFY(SECONDMETRIC) << " : " << pc1.getValue( 1 ) << "\tratio: " << 100.0 * pc1.getRatio( 1, 0 ) << " %\n";
	std::cout << STRINGIFY(THIRDMETRIC) << " : " << pc1.getValue( 2 ) << '\t' << STRINGIFY(FOURTHMETRIC) << " : " << pc1.getValue( 3 ) << "\tratio: " << 100.0 * pc1.getRatio( 3, 2 ) << " %\n";
	std::cout << "fifthmetric: " << pc1.getValue( 4 ) << '\n';
	std::cout << "Multiplexing scaling factor: " << pc1.getLastScaling() << '\n';
	std::cout << '\n';



}


}


/* Linux perf events have a few problems - one, is the documentation. There's some around - just it doesn' always match the system's behaviour, and */

