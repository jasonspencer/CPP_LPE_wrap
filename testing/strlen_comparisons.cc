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
#define TOTALSTRINGLENGTH (1<<30)
// #define TOTALSTRINGLENGTH (1<<20)
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

#ifndef DISABLESSE4STRLEN
size_t sse4strlen ( const char * str ) {
	__m128i zero = _mm_setzero_si128();
	int len = 0;
	int i = 0;
	do {
		__m128i s = _mm_lddqu_si128( (__m128i *)(str + len) );
		i = _mm_cmpistri ( s, zero, _SIDD_SBYTE_OPS | _SIDD_CMP_EQUAL_EACH | _SIDD_LEAST_SIGNIFICANT );
		len += i;
	} while ( i == sizeof(__m128i));
	return len;
}

size_t sse4strlen_aligned ( const char * str ) {
	__m128i zero = _mm_setzero_si128();
	unsigned len = 0;
	int i = sizeof(__m128i);
	unsigned alignment_offset = (uintmax_t)str%sizeof(__m128i);
	if(alignment_offset) {
		// not aligned
		__m128i s = _mm_lddqu_si128( (__m128i *)(str+len) );
		i = _mm_cmpistri ( s, zero, _SIDD_SBYTE_OPS | _SIDD_CMP_EQUAL_EACH | _SIDD_LEAST_SIGNIFICANT );
		len += i;
		if(i==sizeof(__m128i)) len -= alignment_offset;	// bring the pointer back to the nearest __m128i alignment
	}
	if( i == sizeof(__m128i) ) {
		// the string is longer than 16 chars, continue as aligned from earlier aligned pointer
		do {
			__m128i s = _mm_load_si128( (__m128i *)(str+len) );
			i = _mm_cmpistri ( s, zero, _SIDD_SBYTE_OPS | _SIDD_CMP_EQUAL_EACH | _SIDD_LEAST_SIGNIFICANT );
			len += i;
		} while ( i == sizeof(__m128i));
	}
	return len;
}
#endif

// inspiration from https://www.strchr.com/sse2_optimised_strlen
size_t sse2strlen ( const char * str ) {
	__m128i zero = _mm_setzero_si128();
	int len = 0;
	int i = 0;
	do {
		__m128i s = _mm_loadu_si128( (__m128i *)(str + len) );
        s = _mm_cmpeq_epi8(s, zero);
		int mask = _mm_movemask_epi8(s);
		if(mask) {
			i = __builtin_ctz(mask);
		} else {
			i = sizeof(__m128i);
		}
		len += i;
	} while ( i == sizeof(__m128i) );
	return len;
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

jsplib::perf::PerfEventCount pc1 { FIRSTMETRIC, SECONDMETRIC, THIRDMETRIC, FOURTHMETRIC, FIFTHMETRIC };

uintmax_t sum = 0;
std::cout << '\n';
{
	jsplib::CScopedTiming timer("mystrlen with prefetch took");
	jsplib::perf::ScopedEventTrigger trig { &pc1 };
	unsigned j = 0;
	size_t len = 1;
	unsigned prev_j = j;
	while( j < TOTALSTRINGLENGTH ) {
		if((j>>6)!=(prev_j>>6)) _mm_prefetch ( str.get() + j + 128, _MM_HINT_T0 );
		len = mystrlen ( str.get() + j );
		sum += len;
		prev_j = j;
		j += (len+1);
	}
}
std::cout << STRINGIFY(FIRSTMETRIC) << " : " << pc1.getValue( 0 ) << '\t' << STRINGIFY(SECONDMETRIC) << " : " << pc1.getValue( 1 ) << "\tratio: " << 100.0 * pc1.getRatio( 1, 0 ) << " %\t sum : " << sum << "\n";
std::cout << STRINGIFY(THIRDMETRIC) << " : " << pc1.getValue( 2 ) << '\t' << STRINGIFY(FOURTHMETRIC) << " : " << pc1.getValue( 3 ) << "\tratio: " << 100.0 * pc1.getRatio( 3, 2 ) << " %\n";
std::cout << "fifthmetric: " << pc1.getValue( 4 ) << '\n';
std::cout << "Multiplexing scaling factor: " << pc1.getLastScaling() << '\n';
std::cout << '\n';

std::tuple < const char *, std::function<size_t(const char *)> > functions_to_test [] = {
	std::make_tuple( "mystrlen", mystrlen ),
	std::make_tuple( "bsdstrlen", bsdstrlen ),
	std::make_tuple( "std::strlen", std::strlen ),
#ifndef DISABLESSE4STRLEN
	std::make_tuple( "sse4strlen", sse4strlen ),
	std::make_tuple( "sse4strlen_aligned", sse4strlen_aligned ),
#endif
	std::make_tuple( "sse2strlen", sse2strlen )
	};


// for(unsigned i = 0; i < (sizeof(functions_to_test)/sizeof(functions_to_test[0])); ++i ) {
for(unsigned i = 0; i < arraySize(functions_to_test); ++i ) {
	pc1.reset();
	sum = 0;
	{
		std::function<size_t(const char *)> f;
		const char * p;
		std::tie(p,f) = functions_to_test[i];
		std::string s (p);
		s += " took";
		jsplib::CScopedTiming timer(s.c_str());
		jsplib::perf::ScopedEventTrigger trig { &pc1 };
		size_t len = 1;
		unsigned j = 0;
		while( j < TOTALSTRINGLENGTH ) {
//			jsplib::perf::ScopedEventTrigger<1> trig { pc1 };
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

