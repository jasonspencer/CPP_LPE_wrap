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

template <typename T> void CHECK_ARRAY_EQUALITY( const T * const a__, const unsigned w, const unsigned row_length, const T * const b__) {
#ifdef SANITYCHECKS
	std::cout << "Result arrays found ";
	if(!std::equal(&a__[0],&a__[w*row_length],&b__[0])) std::cout << "NOT ";
	std::cout << "to be equal." << std::endl;
#endif
}

template <typename T> void CHECK_ARRAY_EQUALITY_WITH_DUMP( const T * const a__, const unsigned w, const unsigned row_length, const T * const b__) {
#ifdef SANITYCHECKS
	bool inequal = false;
	std::cout << "Result arrays found ";
	if(!std::equal(&a__[0],&a__[w*row_length],&b__[0])) {
		std::cout << "NOT ";
		inequal = true;
	} else inequal = false;
	std::cout << "to be equal." << std::endl;
	if(inequal) dump_array_comparison( std::cout, a__, b__, w );
#endif
}

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

#include "simd.common.cc"

template < typename T > void inplace_transpose_tiled_SIMD_padded( T * const m, const unsigned w, const unsigned row_length ) {
constexpr unsigned T_per_simd = sizeof(__m128i) / sizeof(T);	// tile_width is the number of T units per SIMD tile
assert((sizeof(T)*T_per_simd)==sizeof(__m128i));				// check that there is no remainder
const unsigned simds_per_row = w / T_per_simd;
const unsigned simds_per_storage_row = row_length / T_per_simd;
std::cout << "T_per_simd = " << T_per_simd << "  w = " << w << "  row_length = " << row_length << "  simds_per_row = " << simds_per_row << "  simds_per_storage_row = " << simds_per_storage_row << std::endl;

__m128i * pa, * pb, * m_simd;
m_simd = reinterpret_cast<__m128i *>(m);

for ( unsigned a = 0; a < simds_per_row; a++ ) {
	pa = m_simd + ( a * row_length ) + a;
	simd_4x4_transpose( pa, simds_per_storage_row );
	for ( unsigned b = a + 1; b < simds_per_row; b++ ) {
		pa = m_simd + ( a * row_length ) + b;
		pb = m_simd + ( b * row_length ) + a;
		simd_4x4_transpose_swap ( pa, pb, simds_per_storage_row );
	}
}
}


int main (int, char **) {
const unsigned w = MATWIDTH;
const unsigned row_length = ((w%256)?w:(w + LEVEL1_DCACHE_LINESIZE/sizeof(dtype)));
const unsigned alignsize = std::max( sizeof(__m128i), (decltype(sizeof(__m128i))) LEVEL1_DCACHE_LINESIZE );
const unsigned alignpadding = alignsize / sizeof(dtype);

std::unique_ptr<dtype[]> updata ( new dtype[w*row_length+alignpadding] );
std::unique_ptr<dtype[]> updatat ( new dtype[w*row_length+alignpadding] );
std::unique_ptr<dtype[]> uptransposed ( new dtype[w*row_length+alignpadding] );

dtype * data = align_to ( updata.get(), alignsize );
dtype * datat = align_to ( updatat.get(), alignsize );
dtype * transposed = align_to ( uptransposed.get(), alignsize );

std::cout << std::hex;
std::cout << "updata = " << updata.get() << " updatat = " << updatat.get() << " uptransposed = " << uptransposed.get() << std::endl;
std::cout << "data = " << data << " datat = " << datat << " transposed = " << transposed << std::endl;
std::cout << std::dec;

std::cout << "N = " << w << " alignsize = " << alignsize << " alignpadding = " << alignpadding << " LEVEL1_DCACHE_LINESIZE = " << LEVEL1_DCACHE_LINESIZE << std::endl;
std::cout << " sizeof(dtype) = " << sizeof(dtype) << " sizeof(__m128i) = " << sizeof(__m128i) << std::endl;

std::fill( &data[0], &data[w*row_length], 0 );
std::fill( &datat[0], &datat[w*row_length], 0 );
for ( unsigned a=0; a<w; a++ )
	for( unsigned b=0; b<w; b++ ) {
			data[a+(b*row_length)] = a+(b*w);
			datat[a+(b*row_length)] = b+(a*w);
			}

#ifdef TILESIMDPAD
std::fill( &transposed[0], &transposed[w*row_length], 0 );
std::cout << "Doing inplace_transpose_tiled_SIMD.." << std::endl;
{
	jsplib::CScopedTiming timer1("inplace_transpose_tiled_SIMD_padded took ");
	std::copy ( &data[0], &data[w*row_length], transposed );
	inplace_transpose_tiled_SIMD_padded ( transposed, w, row_length );
}
CHECK_ARRAY_EQUALITY( transposed, w, row_length, datat );
#endif


std::cout << "End of main." << std::endl;

return 0;
}
