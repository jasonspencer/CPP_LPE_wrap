
// naive transpose by direct copy
template < typename T > void transpose_1D_naive( const T * const in, T * out, const unsigned w ) {
	for(unsigned i=0;i<w;i++)
		for(unsigned j=0;j<w;j++)
			out[j*w+i]=in[i*w+j];
}
