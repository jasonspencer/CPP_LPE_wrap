
template < typename T > void transpose_tiled_bis( const T * const src, T * const dest, const unsigned w ) {

constexpr const unsigned tile_width = LEVEL1_DCACHE_LINESIZE / sizeof(T);

static_assert((sizeof(T)*tile_width)==LEVEL1_DCACHE_LINESIZE);

for( unsigned a = 0; a < w; a += tile_width ) {
	for( unsigned i = 0; i < tile_width; i++ ) {
		for( unsigned j = 0; j < tile_width; j++ ) {
			unsigned x = a + i;	// TODO - factor this in to for loop
			unsigned y = a + j;
//			std::swap( m[x*w+y], m[y*w+x] );
			dest[x*w+y] = src[y*w+x];
			}
		}
/*	for( unsigned b = a + tile_width; b < w; b += tile_width ) {
		for( unsigned i = 0; i < tile_width; i++ ) {
			for( unsigned j = 0; j < tile_width; j++ ) {
		// swap values at i,j within tile a,b
				unsigned x = a + i;
				unsigned y = b + j;
				std::swap( m[x*w+y], m[y*w+x] );
			}
		}
	} */
}
}



























/*
Erm.. but Apple were sued for storing weeks of GPS history - in Android it's an opt-in feature.

And now there's this issue with Uber tracking iPhone users - they got caught, but how many more didn't?

And then there's the whole issue of Apple previously removing apps from the app store that duplicate Apple functionality - so you have to rely on them, and they're the least transparent company going.

And then there's stuff like "Apple’s banned an iPhone app that let you know if you were being spied on" on thenextweb.

You're either deeply ignorant or just trolling or perhaps both.

On average iOS and Android are about as good/bad as each other when it comes to privacy, so you might as well choose the better priced, higher specced, more choice and more open platform and dump iOS completely.



*/
