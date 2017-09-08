
// naive transpose by direct copy with strength reduction
template < typename T > void transpose_1D_naive_sr( const T * const in, T * out, const unsigned w ) {
	for(unsigned i=0, iw=0;i<w;i++, iw+=w)
		for(unsigned j=0, jw=0;j<w;j++, jw+=w)
			out[jw+i]=in[iw+j];
}
