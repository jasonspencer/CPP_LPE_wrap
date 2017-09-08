#include <iostream>
#include <memory>
#include <thread>
#include <iterator>
#include <algorithm>
#include <cassert>
#include "CScopedTiming.hh"

#include <emmintrin.h>

#ifndef MATWIDTH
#define MATWIDTH 16384
#endif

#ifndef LEVEL1_DCACHE_LINESIZE
#define LEVEL1_DCACHE_LINESIZE 64
#endif

#ifdef ALLTESTS
#define NAIVE2DD 1
#define NAIVE2DC 1
#define NAIVE1D 1
#define NAIVE1DSR 1
#define NAIVE1DSRR 1
#define COPYINPLACE 1
#define INPLACER 1
#define INPLACEONLY 1
#define TILED 1
#define TILESIMD 1
#define TILESIMDUNROLL 1
#define TILESIMDTB 1
#define TILESIMDTI 1
#endif

// g++ -std=c++11 -pthread transpose.test.0001.cc -msse4.2 -Wall -O3 -DLEVEL1_DCACHE_LINESIZE=`getconf LEVEL1_DCACHE_LINESIZE`

using dtype = int;

template <typename T> void dump_array(std::ostream & os, const T * const arr, const unsigned w) {
	std::ostream_iterator<T> os_it(os," ");
	for(unsigned j=0;j<w;j++) {
		std::copy( &arr[j*w], &arr[(j+1)*w], os_it);
		os << std::endl;
	}
}

template <typename T> void dump_array_comparison(std::ostream & os, const T * const arr1, const T * const arr2, const unsigned w) {
	for(unsigned i=0;i<w;i++) {
		for(unsigned j=0;j<w;j++) {
			os << ((arr1[i*w+j]==arr2[i*w+j])?". ":"* ");
		}
		os << std::endl;
	}
}

template <typename T> void CHECK_ARRAY_EQUALITY( const T * const a__, const unsigned w, const T * const b__) {
#ifdef SANITYCHECKS
	std::cout << "Result arrays found ";
	if(!std::equal(&a__[0],&a__[w*w],&b__[0])) std::cout << "NOT ";
	std::cout << "to be equal." << std::endl;
#endif
}

template <typename T> void CHECK_ARRAY_EQUALITY_WITH_DUMP( const T * const a__, const unsigned w, const T * const b__) {
#ifdef SANITYCHECKS
	bool inequal = false;
	std::cout << "Result arrays found ";
	if(!std::equal(&a__[0],&a__[w*w],&b__[0])) {
		std::cout << "NOT ";
		inequal = true;
	} else inequal = false;
	std::cout << "to be equal." << std::endl;
	if(inequal) dump_array_comparison( std::cout, a__, b__, w );
#endif
}

#ifdef NAIVE1D
#include "transpose_1D_naive.cc"
#endif
#ifdef NAIVE1DR
#include "transpose_1D_naive_restrict.cc"
#endif
#ifdef NAIVE1DSR
#include "transpose_1D_naive_sr.cc"
#endif
#ifdef NAIVE1DSRR
#include "transpose_1D_naive_sr_restrict.cc"
#endif
#if defined(COPYINPLACE) || defined(INPLACEONLY) || defined(INPLACER)
#include "transpose_inplace.cc"
#endif
#if defined(COPYINPLACE) || defined(INPLACER)
#include "transpose_inplace_restrict.cc"
#endif

#ifdef TILED
#include "inplace_transpose_tiled.cc"
#endif

#if defined(TILESIMD) || defined(TILESIMDTB) || defined(TILESIMDTI)
#include "simd.common.cc"
#endif
#ifdef TILESIMD
#include "inplace_transpose_tiled_SIMD.cc"
#endif
#ifdef TILESIMDTB
#include "inplace_transpose_tiled_SIMD_threaded_block.cc"
#endif
#ifdef TILESIMDTI
#include "inplace_transpose_tiled_SIMD_threaded_interleaved.cc"
#endif

template <typename NATIVE> NATIVE * align_to( NATIVE * p, const unsigned alignsize ) {
	uint8_t * uip = reinterpret_cast<uint8_t *>(p);
	size_t off = reinterpret_cast<size_t>(uip)%alignsize;
	return reinterpret_cast<NATIVE *>( uip + (alignsize-off));
}

void print_m128i ( const __m128i & val ) {
	const int * const p = reinterpret_cast<const int * >(&val);	// hack
	std::cout << p[0] << " " << p[1] << " " << p[2] << " " << p[3];
}

template <typename T> void dump_2D(std::ostream & os, const T * const * const arr, const unsigned w) {
        for(unsigned j=0;j<w;j++) {
                std::copy( arr[j], arr[j]+w, std::ostream_iterator<T> (os," ") );
                os << std::endl;
        }
}

#if defined(NAIVE2DD) || defined(NAIVE2DC)
#include "transpose_2D_naive.cc"
#endif
#ifdef NAIVE2DD
#include "transpose_2D_disjoint_test.cc"
#endif
#ifdef NAIVE2DC
#include "transpose_2D_contiguous_test.cc"
#endif

int main (int, char **) {
const unsigned w = MATWIDTH;
const unsigned alignsize = std::max( sizeof(__m128i), (decltype(sizeof(__m128i))) LEVEL1_DCACHE_LINESIZE );
const unsigned alignpadding = alignsize / sizeof(dtype);

std::unique_ptr<dtype[]> updata ( new dtype[w*w+alignpadding] );
std::unique_ptr<dtype[]> updatat ( new dtype[w*w+alignpadding] );
std::unique_ptr<dtype[]> uptransposed ( new dtype[w*w+alignpadding] );

dtype * data = align_to ( updata.get(), alignsize );
dtype * datat = align_to ( updatat.get(), alignsize );
dtype * transposed = align_to ( uptransposed.get(), alignsize );

std::cout << std::hex;
std::cout << "updata = " << updata.get() << " updatat = " << updatat.get() << " uptransposed = " << uptransposed.get() << std::endl;
std::cout << "data = " << data << " datat = " << datat << " transposed = " << transposed << std::endl;
std::cout << std::dec;

std::cout << "N = " << w << " alignsize = " << alignsize << " alignpadding = " << alignpadding << " LEVEL1_DCACHE_LINESIZE = " << LEVEL1_DCACHE_LINESIZE << std::endl;
std::cout << " sizeof(dtype) = " << sizeof(dtype) << " sizeof(__m128i) = " << sizeof(__m128i) << std::endl;

for ( unsigned a=0; a<w*w; a++ ) data[a] = a;
for ( unsigned a=0; a<w; a++ ) for( unsigned b=0; b<w; b++ ) datat[a+(b*w)] = b+(a*w);

// dump_array(std::cout, datat, w);

#if defined(NAIVE2DD) || defined(NAIVE2DC)
{
	std::unique_ptr<dtype * []> upaoat(new dtype * [w]);
	dtype ** aoat = upaoat.get();
	for(unsigned a=0;a<w;a++) { aoat[a]=datat+(a*w); }
	
#ifdef NAIVE2DD
	transpose_2D_disjoint_test(w, aoat);
#endif

#ifdef NAIVE2DC
	transpose_2D_contiguous_test(w, aoat);
#endif
}
#endif // defined(NAIVE2DD) || defined(NAIVE2DC)

#ifdef NAIVE1D
std::cout << "Doing transpose_1D_naive.." << std::endl;
{
	jsplib::CScopedTiming timer1("transpose_1D_naive took ");
	transpose_1D_naive ( data, transposed, w );
}
CHECK_ARRAY_EQUALITY( transposed, w, datat );
#endif

#ifdef NAIVE1DR
std::cout << "Doing transpose_1D_naive_restrict.." << std::endl;
{
	jsplib::CScopedTiming timer1("transpose_1D_naive_restrict took ");
	transpose_1D_naive_restrict ( data, transposed, w );
}
CHECK_ARRAY_EQUALITY( transposed, w, datat );
#endif

#ifdef NAIVE1DSR
std::cout << "Doing transpose_1D_naive_sr.." << std::endl;
{
	jsplib::CScopedTiming timer1("transpose_1D_naive_sr took ");
	transpose_1D_naive_sr ( data, transposed, w );
}
CHECK_ARRAY_EQUALITY( transposed, w, datat );
#endif

#ifdef NAIVE1DSRR
std::cout << "Doing transpose_1D_naive_sr_restrict.." << std::endl;
{
	jsplib::CScopedTiming timer1("transpose_1D_naive_sr_restrict took ");
	transpose_1D_naive_sr_restrict( data, transposed, w);
}
CHECK_ARRAY_EQUALITY( transposed, w, datat );
#endif

#ifdef COPYINPLACE
std::fill( &transposed[0], &transposed[w*w], 0 );
std::cout << "Doing copy and transpose_inplace.." << std::endl;
{
	jsplib::CScopedTiming timer1("copy+transpose_inplace took ");
	std::copy( &data[0], &data[w*w], &transposed[0] );
	transpose_inplace( transposed, w );
}
CHECK_ARRAY_EQUALITY( transposed, w, datat );
#endif

#ifdef INPLACER
std::fill( &transposed[0], &transposed[w*w], 0 );
std::cout << "Doing copy and transpose_inplace_restrict.." << std::endl;
{
	jsplib::CScopedTiming timer1("copy+transpose_inplace_restrict took ");
	std::copy ( &data[0], &data[w*w], &transposed[0] );
	transpose_inplace_restrict ( transposed, w );
}
CHECK_ARRAY_EQUALITY( transposed, w, datat );
#endif

#ifdef INPLACEONLY
std::copy ( &data[0], &data[w*w], &transposed[0] );
std::cout << "Doing transpose_inplace.." << std::endl;
{
	jsplib::CScopedTiming timer1("transpose_inplace took ");
	transpose_inplace ( transposed, w );
}
CHECK_ARRAY_EQUALITY( transposed, w, datat );
#endif

#ifdef TILED
std::fill ( &transposed[0], &transposed[w*w], 0 );
std::cout << "Doing inplace_transpose_tiled.." << std::endl;
{
	jsplib::CScopedTiming timer1("inplace_transpose_tiled took ");
	std::copy ( &data[0], &data[w*w], transposed );
	inplace_transpose_tiled ( transposed, w );
}
CHECK_ARRAY_EQUALITY( transposed, w, datat );
// CHECK_ARRAY_EQUALITY_WITH_DUMP( transposed, w, datat );
#endif

#ifdef TILESIMD
std::fill( &transposed[0], &transposed[w*w], 0 );
std::cout << "Doing inplace_transpose_tiled_SIMD.." << std::endl;
{
	jsplib::CScopedTiming timer1("inplace_transpose_tiled_SIMD took ");
	std::copy ( &data[0], &data[w*w], transposed );
	inplace_transpose_tiled_SIMD ( transposed, w );
	// dump_array(std::cout, out, w);
}
CHECK_ARRAY_EQUALITY( transposed, w, datat );
// CHECK_ARRAY_EQUALITY_WITH_DUMP( transposed, w, datat );
#endif

#ifdef TILESIMDTB
std::fill ( &transposed[0], &transposed[w*w], 0 );
std::cout << "Doing inplace_transpose_tiled_SIMD_threaded_block.." << std::endl;
{
	jsplib::CScopedTiming timer1("inplace_transpose_tiled_SIMD_threaded_block took ");
	std::copy ( &data[0], &data[w*w], transposed );
	inplace_transpose_tiled_SIMD_threaded_block ( transposed, w );
}
// CHECK_ARRAY_EQUALITY_WITH_DUMP( transposed, w, datat );
CHECK_ARRAY_EQUALITY( transposed, w, datat );
#endif

#ifdef TILESIMDTI
std::fill ( &transposed[0], &transposed[w*w], 0 );
std::cout << "Doing inplace_transpose_tiled_SIMD_threaded_interleaved.." << std::endl;
{
	jsplib::CScopedTiming timer1("inplace_transpose_tiled_SIMD_threaded_interleaved took ");
	std::copy ( &data[0], &data[w*w], transposed );
	inplace_transpose_tiled_SIMD_threaded_interleaved ( transposed, w );
}
CHECK_ARRAY_EQUALITY( transposed, w, datat );
// CHECK_ARRAY_EQUALITY_WITH_DUMP( transposed, w, datat );
#endif

std::cout << "End of main." << std::endl;

return 0;
}
