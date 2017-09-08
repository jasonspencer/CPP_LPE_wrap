
template < typename T > void inplace_transpose_tiled( T * const m, const unsigned w ) {
constexpr unsigned tile_width = LEVEL1_DCACHE_LINESIZE / sizeof(T);
assert((sizeof(T)*tile_width)==LEVEL1_DCACHE_LINESIZE);
for( unsigned a = 0; a < w; a += tile_width ) {
	for( unsigned i = 0; i < tile_width; i++ ) {
		for( unsigned j = i; j < tile_width; j++ ) {
			unsigned x = a + i;
			unsigned y = a + j;
			std::swap( m[x*w+y], m[y*w+x] );
			}
		}
	for( unsigned b = a + tile_width; b < w; b += tile_width ) {
		for( unsigned i = 0; i < tile_width; i++ ) {
			for( unsigned j = 0; j < tile_width; j++ ) {
		// swap values at i,j within tile a,b
				unsigned x = a + i;
				unsigned y = b + j;
				std::swap( m[x*w+y], m[y*w+x] );
			}
		}
	}
}
}

