
template < typename T > void transpose_2D_naive( const T * const * const in, T ** out, const unsigned w ) {
	for(unsigned i=0;i<w;i++)
		for(unsigned j=0;j<w;j++)
			out[j][i]=in[i][j];
}
