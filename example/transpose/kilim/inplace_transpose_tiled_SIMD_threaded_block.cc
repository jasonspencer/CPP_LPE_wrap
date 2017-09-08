
template < typename T > void inplace_transpose_tiled_SIMD_threaded_block_THREAD( T * const m, const unsigned w, const unsigned startcol, const unsigned endcol  ){
	constexpr unsigned tile_width=sizeof(__m128i)/sizeof(T);
	assert((sizeof(T)*tile_width)==sizeof(__m128i));
	T * pa, * pb;
	
	for(unsigned a=startcol;a<endcol;a+=tile_width) {
		pa = m+(a*w)+a;
		simd_4x4_transpose(reinterpret_cast<__m128i *>(pa), w/tile_width );
		for(unsigned b=a+tile_width;b<w;b+=tile_width) {
			pa = m+(a*w)+b;
			pb = m+(b*w)+a;
			simd_4x4_transpose_swap(reinterpret_cast<__m128i *>(pa), reinterpret_cast<__m128i *>(pb), w/tile_width );
		}
	}
}

template < typename T > void inplace_transpose_tiled_SIMD_threaded_block( T * const m, const unsigned w ) {
	constexpr unsigned tile_width=sizeof(__m128i)/sizeof(T);
	assert((sizeof(T)*tile_width)==sizeof(__m128i));
	std::cout << "tile_width=" << tile_width << std::endl;
	std::cout << "w=" << w << std::endl;
	unsigned nthreads = std::thread::hardware_concurrency();
	std::vector<std::thread> threads;
	unsigned columns_per_thread = (w/tile_width)/nthreads;

	std::cout << "Starting " << nthreads << " threads." << std::endl;

	for(unsigned i=0;i<nthreads;i++) {
		std::cout << "Starting thread " << i << " to compute [" << (i*columns_per_thread*tile_width) << "," << ((i+1)*columns_per_thread*tile_width) << ")" << std::endl;
		threads.push_back ( std::thread(inplace_transpose_tiled_SIMD_threaded_block_THREAD<T>, m, w, i*columns_per_thread*tile_width, (i+1)*columns_per_thread*tile_width ) );
	}
	std::cout << "Joining threads.." << std::endl;
	for( auto & t : threads ) t.join();
}
