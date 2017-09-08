
// takes four rows of __m128i (4x4 of 4-byte integers) and transposes inplace
inline static void simd_4x4_transpose( __m128i * a, const unsigned off ) {
	__m128i * a1, * a2, * a3, * a4;
//	std::cout << std::hex << "* simd_4x4_transpose ( " << a << " , " << off << " )" << std::endl;
	a1 = a;
	a2 = a + off;
	a3 = a + ( 2 * off );
	a4 = a + ( 3 * off );
//	std::cout << std::hex << "simd_4x4_transpose( " << a1 << ", " << a2 << ", " << a3 << ", " << a4 << " )" << std::dec << std::endl;
	__m128i tmp0, tmp1, tmp2, tmp3;

	__m128i rowa0 = _mm_load_si128 ( a1 );
	__m128i rowa1 = _mm_load_si128 ( a2 );
	__m128i rowa2 = _mm_load_si128 ( a3 );
	__m128i rowa3 = _mm_load_si128 ( a4 );

	tmp0 = _mm_unpacklo_epi32 ( rowa0, rowa1 );
	tmp1 = _mm_unpacklo_epi32 ( rowa2, rowa3 );
	tmp2 = _mm_unpackhi_epi32 ( rowa0, rowa1 );
	tmp3 = _mm_unpackhi_epi32 ( rowa2, rowa3 );
	rowa0 = _mm_unpacklo_epi64 ( tmp0, tmp1 );
	rowa1 = _mm_unpackhi_epi64 ( tmp0, tmp1 );
	rowa2 = _mm_unpacklo_epi64 ( tmp2, tmp3 );
	rowa3 = _mm_unpackhi_epi64 ( tmp2, tmp3 );

	_mm_store_si128 ( a1, rowa0 );
	_mm_store_si128 ( a2, rowa1 );
	_mm_store_si128 ( a3, rowa2 );
	_mm_store_si128 ( a4, rowa3 );

}

// takes four rows of __m128i (4x4 of 4-byte integers) from a and transposes into b and vice-versa
inline static void simd_4x4_transpose_swap ( __m128i * a, __m128i * b, const unsigned off ) {
	__m128i * a1, * a2, * a3, * a4;
	__m128i * b1, * b2, * b3, * b4;
//	std::cout << std::hex << "* simd_4x4_transpose_swap ( " << a << " , " << b << " , " << off << " )" << std::endl;
	a1 = a;
	a2 = a + off;
	a3 = a + ( 2 * off );
	a4 = a + ( 3 * off );
	b1 = b;
	b2 = b + off;
	b3 = b + (2 * off );
	b4 = b + (3 * off );
//	std::cout << std::hex << "simd_4x4_transpose_swap( " << a1 << ", " << a2 << ", " << a3 << ", " << a4 << ", "  << b1 << ", " << b2 << ", " << b3 << ", " << b4 << " )" << std::dec << std::endl;
	
	__m128i tmp0 ,tmp1, tmp2, tmp3;

	__m128i rowa0 = _mm_load_si128 ( a1 );
	__m128i rowa1 = _mm_load_si128 ( a2 );
	__m128i rowa2 = _mm_load_si128 ( a3 );
	__m128i rowa3 = _mm_load_si128 ( a4 );
	__m128i rowb0 = _mm_load_si128 ( b1 );
	__m128i rowb1 = _mm_load_si128 ( b2 );
	__m128i rowb2 = _mm_load_si128 ( b3 );
	__m128i rowb3 = _mm_load_si128 ( b4 );

	tmp0 = _mm_unpacklo_epi32 ( rowa0, rowa1 );
	tmp1 = _mm_unpacklo_epi32 ( rowa2, rowa3 );
	tmp2 = _mm_unpackhi_epi32 ( rowa0, rowa1 );
	tmp3 = _mm_unpackhi_epi32 ( rowa2, rowa3 );
	rowa0 = _mm_unpacklo_epi64 ( tmp0, tmp1 );
	rowa1 = _mm_unpackhi_epi64 ( tmp0, tmp1 );
	rowa2 = _mm_unpacklo_epi64 ( tmp2, tmp3 );
	rowa3 = _mm_unpackhi_epi64 ( tmp2, tmp3 );
	
	_mm_store_si128 ( b1, rowa0 );
	_mm_store_si128 ( b2, rowa1 );
	_mm_store_si128 ( b3, rowa2 );
	_mm_store_si128 ( b4, rowa3 );

	tmp0 = _mm_unpacklo_epi32 ( rowb0, rowb1 );
	tmp1 = _mm_unpacklo_epi32 ( rowb2, rowb3 );
	tmp2 = _mm_unpackhi_epi32 ( rowb0, rowb1 );
	tmp3 = _mm_unpackhi_epi32 ( rowb2, rowb3 );
	rowb0 = _mm_unpacklo_epi64 ( tmp0, tmp1 );
	rowb1 = _mm_unpackhi_epi64 ( tmp0, tmp1 );
	rowb2 = _mm_unpacklo_epi64 ( tmp2, tmp3 );
	rowb3 = _mm_unpackhi_epi64 ( tmp2, tmp3 );

	_mm_store_si128 ( a1, rowb0 );
	_mm_store_si128 ( a2, rowb1 );
	_mm_store_si128 ( a3, rowb2 );
	_mm_store_si128 ( a4, rowb3 );

}

// takes four rows of __m128i (4x4 of 4-byte integers) from a and transposes into b and vice-versa
inline static void simd_4x4_transpose_outofplace ( const __m128i * src, __m128i * dest, const unsigned off ) {
	const __m128i * src1, * src2, * src3, * src4;
	__m128i * dest1, * dest2, * dest3, * dest4;
//	std::cout << std::hex << "* simd_4x4_transpose_swap ( " << a << " , " << b << " , " << off << " )" << std::endl;
	src1 = src;
	src2 = src + off;
	src3 = src + ( 2 * off );
	src4 = src + ( 3 * off );
	dest1 = dest;
	dest2 = dest + off;
	dest3 = dest + ( 2 * off );
	dest4 = dest + ( 3 * off );
//	std::cout << std::hex << "simd_4x4_transpose_swap( " << a1 << ", " << a2 << ", " << a3 << ", " << a4 << ", "  << b1 << ", " << b2 << ", " << b3 << ", " << b4 << " )" << std::dec << std::endl;
	
	__m128i tmp0 ,tmp1, tmp2, tmp3;

	__m128i rowa0 = _mm_load_si128 ( src1 );
	__m128i rowa1 = _mm_load_si128 ( src2 );
	__m128i rowa2 = _mm_load_si128 ( src3 );
	__m128i rowa3 = _mm_load_si128 ( src4 );
/*	__m128i rowb0 = _mm_load_si128 ( b1 );
	__m128i rowb1 = _mm_load_si128 ( b2 );
	__m128i rowb2 = _mm_load_si128 ( b3 );
	__m128i rowb3 = _mm_load_si128 ( b4 );
*/
	tmp0 = _mm_unpacklo_epi32 ( rowa0, rowa1 );
	tmp1 = _mm_unpacklo_epi32 ( rowa2, rowa3 );
	tmp2 = _mm_unpackhi_epi32 ( rowa0, rowa1 );
	tmp3 = _mm_unpackhi_epi32 ( rowa2, rowa3 );
	rowa0 = _mm_unpacklo_epi64 ( tmp0, tmp1 );
	rowa1 = _mm_unpackhi_epi64 ( tmp0, tmp1 );
	rowa2 = _mm_unpacklo_epi64 ( tmp2, tmp3 );
	rowa3 = _mm_unpackhi_epi64 ( tmp2, tmp3 );
	
	_mm_store_si128 ( dest1, rowa0 );
	_mm_store_si128 ( dest2, rowa1 );
	_mm_store_si128 ( dest3, rowa2 );
	_mm_store_si128 ( dest4, rowa3 );

}
