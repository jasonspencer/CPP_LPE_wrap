
// naive transpose by in place swap
template < typename T > void transpose_inplace( T * arr, const unsigned w ) {
	for(unsigned i=0;i<w;i++)
		for(unsigned j=i+1;j<w;j++)
			std::swap(arr[j*w+i],arr[i*w+j]);
}
