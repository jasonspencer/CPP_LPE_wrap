// cache analysis example.

#include <iostream>
#include <memory>
#include <thread>
#include <iterator>
#include <algorithm>
#include <cassert>

#include <jsplib/perf/PerfEventCount.hh>
#include <jsplib/perf/WallTimeEvent.hh>
#include <jsplib/perf/ScopedEventTrigger.hh>


#ifndef LEVEL1_DCACHE_LINESIZE
#define LEVEL1_DCACHE_LINESIZE 64
#endif

#include "transpose/transpose_1D_naive.cc"
#include "transpose/inplace_transpose_tiled.cc"
#include "transpose/simd.common.cc"
#include "transpose_tiled_SIMD.cc"


// run tiled transpose method and counters on L1-dcache-load-misses, L1-dcache-loads, LLC-load-misses and LLC-load-misses. maybe also dTLB-load-misses

// TODO when doing tiled non-in-place then try pre-fetching the memory being written to in the previous block.

#ifndef MATWIDTH
#define MATWIDTH 16384
#endif

using dtype = int;		// the data type in the arrays to be transposed.

#if ! defined( PCCACHESIMPLE ) && ! defined( PCREAD ) && ! defined( PCWRITE ) && ! defined( PCL1R ) && ! defined( PCL1W ) && ! defined( PCLLR ) && ! defined( PCLLW ) && ! defined( PCTLBR )  && ! defined( PCTLBW )
#define PCCACHESIMPLE
#endif

#ifdef PCCACHESIMPLE
using pehs = jsplib::perf::linux_perf_event_counter_t::HWSW_EVENT_T;
#endif

#if defined( PCREAD ) || defined( PCWRITE ) || defined( PCL1R ) || defined( PCL1W ) || defined( PCLLR ) || defined( PCLLW ) || defined( PCTLBR ) || defined( PCTLBW )
using cev = jsplib::perf::linux_perf_event_counter_t::HWCACHE_EVENT_T;
using copid = jsplib::perf::linux_perf_event_counter_t::HWCACHE_OPID_EVENT_T;
using copres = jsplib::perf::linux_perf_event_counter_t::HWCACHE_OPRESULT_EVENT_T;
#endif


void printAllCounters ( jsplib::perf::PerfEventCount & pec ) {
	for(size_t i = 0; i < pec.getNumEvents(); ++i ) {
		std::cout << pec.getDescription(i) << " : " << pec.getValue(i) << '\n';
		if(i%2==1) std::cout << "Ratio: " << 100.0 * pec.getRatio(i,i-1) << "%\n";
	}
	std::cout << "Scaling: " << pec.getLastScaling() << '\n';
	}

template <typename NATIVE> NATIVE * align_to( NATIVE * p, const size_t alignsize ) {
	uint8_t * uip = reinterpret_cast<uint8_t *>(p);
	size_t off = reinterpret_cast<size_t>(uip)%alignsize;
	return reinterpret_cast<NATIVE *>( uip + (alignsize-off));
}

int main ( int, char * [] ) {

const unsigned w = MATWIDTH;
const unsigned alignsize = std::max( sizeof(__m128i), (decltype(sizeof(__m128i))) LEVEL1_DCACHE_LINESIZE );
const unsigned alignpadding = alignsize / sizeof(dtype);

std::unique_ptr<dtype[]> updata ( new dtype[w*w+alignpadding] );
std::unique_ptr<dtype[]> uptransposed ( new dtype[w*w+alignpadding] );

dtype * data = align_to ( updata.get(), alignsize );
dtype * transposed = align_to ( uptransposed.get(), alignsize );

for ( unsigned a=0; a<w*w; ++a ) data[a] = a;

jsplib::perf::PerfEventCount perf_counter {
#ifdef PCCACHESIMPLE
		{ pehs::HW_CACHE_REFERENCES  },
		{ pehs::HW_CACHE_MISSES },
//		{ pehs::HW_STALLED_CYCLES_FRONTEND }, 
		{ pehs::HW_REF_CPU_CYCLES }
#endif
#ifdef PCREAD
		{ cev::HW_CACHE_L1D, 	copid::HW_CACHE_OP_READ,	copres::HW_CACHE_RESULT_ACCESS },
		{ cev::HW_CACHE_L1D, 	copid::HW_CACHE_OP_READ,	copres::HW_CACHE_RESULT_MISS },
		{ cev::HW_CACHE_LL, 	copid::HW_CACHE_OP_READ,	copres::HW_CACHE_RESULT_ACCESS },
		{ cev::HW_CACHE_LL, 	copid::HW_CACHE_OP_READ,	copres::HW_CACHE_RESULT_MISS },
		{ cev::HW_CACHE_DTLB, 	copid::HW_CACHE_OP_READ,	copres::HW_CACHE_RESULT_ACCESS },
		{ cev::HW_CACHE_DTLB, 	copid::HW_CACHE_OP_READ, 	copres::HW_CACHE_RESULT_MISS }
#endif
#ifdef PCWRITE
		{ cev::HW_CACHE_L1D, 	copid::HW_CACHE_OP_WRITE,	copres::HW_CACHE_RESULT_ACCESS },
		{ cev::HW_CACHE_L1D, 	copid::HW_CACHE_OP_WRITE,	copres::HW_CACHE_RESULT_MISS },
		{ cev::HW_CACHE_LL, 	copid::HW_CACHE_OP_WRITE,	copres::HW_CACHE_RESULT_ACCESS },
		{ cev::HW_CACHE_LL, 	copid::HW_CACHE_OP_WRITE,	copres::HW_CACHE_RESULT_MISS },
		{ cev::HW_CACHE_DTLB, 	copid::HW_CACHE_OP_WRITE,	copres::HW_CACHE_RESULT_ACCESS },
		{ cev::HW_CACHE_DTLB, 	copid::HW_CACHE_OP_WRITE, 	copres::HW_CACHE_RESULT_MISS }
#endif
#ifdef PCL1R
		{ cev::HW_CACHE_L1D, 	copid::HW_CACHE_OP_READ,	copres::HW_CACHE_RESULT_ACCESS },
		{ cev::HW_CACHE_L1D, 	copid::HW_CACHE_OP_READ,	copres::HW_CACHE_RESULT_MISS }
#endif
#ifdef PCL1W
		{ cev::HW_CACHE_L1D, 	copid::HW_CACHE_OP_WRITE,	copres::HW_CACHE_RESULT_ACCESS },
		{ cev::HW_CACHE_L1D, 	copid::HW_CACHE_OP_WRITE,	copres::HW_CACHE_RESULT_MISS }
#endif
#ifdef PCLLR
		{ cev::HW_CACHE_LL, 	copid::HW_CACHE_OP_READ,	copres::HW_CACHE_RESULT_ACCESS },
		{ cev::HW_CACHE_LL, 	copid::HW_CACHE_OP_READ,	copres::HW_CACHE_RESULT_MISS }
#endif
#ifdef PCLLW
		{ cev::HW_CACHE_LL, 	copid::HW_CACHE_OP_WRITE,	copres::HW_CACHE_RESULT_ACCESS },
		{ cev::HW_CACHE_LL, 	copid::HW_CACHE_OP_WRITE,	copres::HW_CACHE_RESULT_MISS }
#endif
#ifdef PCTLBR
		{ cev::HW_CACHE_DTLB, 	copid::HW_CACHE_OP_READ,	copres::HW_CACHE_RESULT_ACCESS },
		{ cev::HW_CACHE_DTLB, 	copid::HW_CACHE_OP_READ, 	copres::HW_CACHE_RESULT_MISS }
#endif
#ifdef PCTLBW
		{ cev::HW_CACHE_DTLB, 	copid::HW_CACHE_OP_WRITE,	copres::HW_CACHE_RESULT_ACCESS },
		{ cev::HW_CACHE_DTLB, 	copid::HW_CACHE_OP_WRITE, 	copres::HW_CACHE_RESULT_MISS }
#endif
	};

jsplib::perf::WallTimeEvent wall_timer;

std::cout << "Matrix width = " << w << '\n';
std::cout << "Number of elements = " << w*w << "\n\n";

std::cout << "Doing transpose_1D_naive..\n";
{
	jsplib::perf::ScopedEventTrigger trig { &perf_counter, &wall_timer };
	transpose_1D_naive ( data, transposed, w );
}
printAllCounters(perf_counter);
std::cout << "Transpose took " << wall_timer.getDuration() << " seconds.\n\n";

wall_timer.reset();
perf_counter.reset();

// std::fill ( &transposed[0], &transposed[w*w], 0 );
std::cout << "Doing inplace_transpose_tiled..\n";
{
	jsplib::perf::ScopedEventTrigger trig { &perf_counter, &wall_timer };
	std::copy ( &data[0], &data[w*w], transposed );
	inplace_transpose_tiled ( transposed, w );
}
printAllCounters(perf_counter);
std::cout << "Transpose took " << wall_timer.getDuration() << " seconds.\n";


wall_timer.reset();
perf_counter.reset();

// std::fill ( &transposed[0], &transposed[w*w], 0 );
std::cout << "Doing transpose_tiled_SIMD_bis..\n";
{
	jsplib::perf::ScopedEventTrigger trig { &perf_counter, &wall_timer };
//	std::copy ( &data[0], &data[w*w], transposed );
//	inplace_transpose_tiled ( transposed, w );
	transpose_tiled_SIMD_bis ( data, transposed, w );
}
printAllCounters(perf_counter);
std::cout << "Transpose took " << wall_timer.getDuration() << " seconds.\n";

		
}
