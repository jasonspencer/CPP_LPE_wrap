
template < typename T > void transpose_tiled_SIMD_bis( const T * const src, T * const dest, const unsigned w ) {
constexpr unsigned T_per_simd = sizeof(__m128i) / sizeof(T);
static_assert((sizeof(T)*T_per_simd)==sizeof(__m128i), "The size of type T must be exactly divisible into the size of __m128i");				// check that there is no remainder
const unsigned simds_per_row = w / T_per_simd;
assert ( w == (simds_per_row*T_per_simd) );				// w should be exactly divisible by the SIMD width (16 bytes)
std::cout << "T_per_simd = " << T_per_simd << "  w = " << w << "  simds_per_row = " << simds_per_row << std::endl;

__m128i * pb, * dest_simd;
const __m128i  * pa, * src_simd;
src_simd = reinterpret_cast<const __m128i *>(src);
dest_simd = reinterpret_cast<__m128i *>(dest);

for ( unsigned a = 0, aw = 0; a < simds_per_row; ++a, aw += w ) {
	for ( unsigned b = 0, bw = 0; b < simds_per_row; ++b, bw += w ) {
		pa = src_simd + aw + b;
		pb = dest_simd + bw + a;
		simd_4x4_transpose_outofplace ( pa, pb, simds_per_row );
		}
	}


}
