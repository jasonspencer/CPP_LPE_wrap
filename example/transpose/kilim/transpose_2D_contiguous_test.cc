// N is the width of the square matrix and cmp is the matrix to compare the transposition against
template < typename T > void transpose_2D_contiguous_test( const unsigned N, const T * const * const cmp ) {

T ** data = new T * [N];
T ** dataT = new T * [N];
// allocate one block for each of input data and transposed output
data[0] = new T[N*N];
dataT[0] = new T[N*N];
// set up pointers into the contiguous block
for(unsigned a=1;a<N;a++) {
	data[a]=data[a-1]+N;
	dataT[a]=dataT[a-1]+N;
}

// populate it with test values
for(unsigned a=0;a<N;a++)
	for(unsigned b=0;b<N;b++)
		data[a][b]=a*N+b;

{
jsplib::CScopedTiming tim1("transpose_2D_contiguous took ");
transpose_2D_naive (data, dataT, N);
}

// compare the resulting array and the expected result
#ifdef SANITYCHECKS
bool cmpok = true;
for(unsigned a=0;(a<N) && cmpok; a++)
	cmpok = std::equal( cmp[a], cmp[a]+N, dataT[a] );
std::cout << "Result arrays found " << (cmpok?"":"NOT ") << "to be equal." << std::endl;
#else
if(**dataT) std::cout << "This should never be printed" << std::endl;
#endif

// dump_2D(std::cout, dataT, N);

delete [] dataT[0];
delete [] data[0];
delete [] dataT;
delete [] data;
}
