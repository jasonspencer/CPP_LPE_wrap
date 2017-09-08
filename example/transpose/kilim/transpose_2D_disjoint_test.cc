// N is the width of the square matrix and cmp is the matrix to compare the transposition against
template < typename T > void transpose_2D_disjoint_test( const unsigned N, const T * const * const cmp ) {

T ** data = new T * [N];
T ** dataT = new T * [N];
// allocate separate blocks for each *row* in the data and transposed output
for ( unsigned i=0; i<N; ++i ) {
	data[i] = new T[N];
	dataT[i] = new T[N];
}

// populate it with test values
for(unsigned a=0;a<N;a++)
	for(unsigned b=0;b<N;b++)
		data[a][b]=a*N+b;

{
jsplib::CScopedTiming tim1("transpose_2D_disjoint took ");
transpose_2D_naive (data, dataT, N);
}

// compare the resulting array and the expected result
#ifdef SANITYCHECKS
bool cmpok = true;
for(unsigned a=0;(a<N) && cmpok;a++)
	cmpok = std::equal( cmp[a], cmp[a]+N, dataT[a] );
	
std::cout << "Result arrays found " << (cmpok?"":"NOT ") << "to be equal." << std::endl;
#else
if(**dataT) std::cout << "This should never be printed" << std::endl;
#endif

// dump_2D(std::cout, dataT, N);

for(size_t i=0;i<N;++i) { delete [] data[i]; delete [] dataT[i]; }
delete [] dataT;
delete [] data;
}
