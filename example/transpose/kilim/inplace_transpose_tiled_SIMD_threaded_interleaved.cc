
template < typename T > void inplace_transpose_tiled_SIMD_threaded_interleaved_THREAD( T * const m, const unsigned w, const unsigned off, const unsigned step  ){
	constexpr unsigned tile_width=sizeof(__m128i)/sizeof(T);
	assert((sizeof(T)*tile_width)==sizeof(__m128i));
	T * pa, * pb;

	for(unsigned a=(off*tile_width);a<w;a+=(step*tile_width)) {
		pa = m+(a*w)+a;
		simd_4x4_transpose(reinterpret_cast<__m128i *>(pa), w/tile_width );
		for(unsigned b=a+tile_width;b<w;b+=tile_width) {
			pa = m+(a*w)+b;
			pb = m+(b*w)+a;
			simd_4x4_transpose_swap(reinterpret_cast<__m128i *>(pa), reinterpret_cast<__m128i *>(pb), w/tile_width );
		}
	}
}

template < typename T > void inplace_transpose_tiled_SIMD_threaded_interleaved( T * const m, const unsigned w ) {
	constexpr unsigned tile_width=sizeof(__m128i)/sizeof(T);
	assert((sizeof(T)*tile_width)==sizeof(__m128i));
	std::cout << "tile_width=" << tile_width << std::endl;
	std::cout << "w=" << w << std::endl;
	unsigned nthreads = std::thread::hardware_concurrency();
	std::vector<std::thread> threads;

	std::cout << "Starting " << nthreads << " threads." << std::endl;

	for(unsigned i=0;i<nthreads;i++) {
		std::cout << "Starting thread " << i << " to compute at offset " << i << " step " << nthreads << "." << std::endl;
		threads.push_back ( std::thread(inplace_transpose_tiled_SIMD_threaded_interleaved_THREAD<T>, m, w, i, nthreads ) );
	}
	std::cout << "Joining threads.." << std::endl;
	for( auto & t : threads ) t.join();
}
