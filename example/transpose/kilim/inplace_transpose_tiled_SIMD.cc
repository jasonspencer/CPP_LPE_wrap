
template < typename T > void inplace_transpose_tiled_SIMD( T * const m, const unsigned w) {
constexpr unsigned T_per_simd = sizeof(__m128i) / sizeof(T);	// tile_width is the number of T units per SIMD tile
assert((sizeof(T)*T_per_simd)==sizeof(__m128i));				// check that there is no remainder
const unsigned simds_per_row = w / T_per_simd;
std::cout << "T_per_simd = " << T_per_simd << "  w = " << w << "  simds_per_row = " << simds_per_row << std::endl;

__m128i * pa, * pb, * m_simd;
m_simd = reinterpret_cast<__m128i *>(m);

for ( unsigned a = 0; a < simds_per_row; a++ ) {
	pa = m_simd + ( a * w ) + a;	// here w is shortcut of simds_per_row * T_per_simd
	simd_4x4_transpose( pa, simds_per_row );
	for ( unsigned b = a + 1; b < simds_per_row; b++ ) {
		pa = m_simd + ( a * w ) + b;
		pb = m_simd + ( b * w ) + a;
		simd_4x4_transpose_swap ( pa, pb, simds_per_row );
	}
}
}
