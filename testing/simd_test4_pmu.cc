
#include <iostream>
#include <cstdlib>
#include <algorithm>
#include <sstream>
#include <limits>
#include <emmintrin.h>
#include <xmmintrin.h>
#include <smmintrin.h>

#include <jsplib/perf/PerfEventCount.hh>
#include <jsplib/perf/ScopedEventTrigger.hh>
#include <jsplib/CScopedTiming.hh>

#include <jsplib/pp/Stringify.hh>

#define ARR_SIZE 16384

#ifndef FIRSTMETRIC
#define FIRSTMETRIC HW_BRANCH_INSTRUCTIONS
#endif

#ifndef SECONDMETRIC
#define SECONDMETRIC HW_BRANCH_MISSES
#endif

#ifndef THIRDMETRIC
#define THIRDMETRIC HW_REF_CPU_CYCLES
#endif

#ifndef FOURTHMETRIC
#define FOURTHMETRIC HW_STALLED_CYCLES_FRONTEND
#endif

#ifndef FIFTHMETRIC
#define FIFTHMETRIC HW_STALLED_CYCLES_BACKEND
#endif

// Example 12.4a. Loop with branch
// Loop with branch
static inline void SelectAddMul(short int aa[], short int bb[], short int cc[]) {
	for (size_t i = 0; i < ARR_SIZE; i++) {
		aa[i] = (bb[i] > 0) ? (cc[i] + 2) : (bb[i] * cc[i]);
	}
}

// Example 12.4c. Same example, vectorized with SSE4.1
static inline void SelectAddMul_sse41(short int aa[], short int bb[], short int cc[]) {
__m128i zero = _mm_set1_epi16(0);
__m128i two = _mm_set1_epi16(2);
// Roll out loop by eight to fit the eight-element vectors:
for (size_t i = 0; i < ARR_SIZE; i += 8) {
	__m128i b = _mm_loadu_si128((__m128i const *)(bb + i)); // Load eight consecutive elements from bb into vector b
	__m128i c = _mm_loadu_si128((__m128i const *)(cc + i)); // Load eight consecutive elements from cc into vector c
	__m128i c2 = _mm_add_epi16(c, two);				// Add 2 to each element in vector c
	__m128i bc = _mm_mullo_epi16 (b, c); 			// Multiply b and c
	__m128i mask = _mm_cmpgt_epi16(b, zero);		// Compare each element in b to 0 and generate a bit-mask
	__m128i a = _mm_blendv_epi8(bc, c2, mask);		// Use mask to choose between c2 and bc for each element
	_mm_storeu_si128((__m128i *)(aa + i), a);		// Store the result vector in eight consecutive elements in aa
	}
}

inline short int make_rand() {
	short int out = (double)std::numeric_limits<short int>::max()*(((double)std::rand()/(double)RAND_MAX)-0.5);
	return out;
}

int main (int argc, char ** argv) {

size_t N;
int seed;

short int aa1 [ARR_SIZE] __attribute__((aligned(sizeof(__m128i))));
short int aa2 [ARR_SIZE] __attribute__((aligned(sizeof(__m128i))));
short int bb [ARR_SIZE] __attribute__((aligned(sizeof(__m128i))));
short int cc [ARR_SIZE] __attribute__((aligned(sizeof(__m128i))));

if(argc != 3) { std::cerr << "Usage: " << argv[0] << " number_of_loops seed" << std::endl; return -1; }

{
std::stringstream ss1(argv[1]);
ss1 >> N;
if(ss1.fail()) { std::cerr << " first argument is not an integer." << std::endl; return -1; }
std::stringstream ss2(argv[2]);
ss2 >> seed;
if(ss2.fail()) { std::cerr << " second argument is not an integer." << std::endl; return -1; }
}

std::cout << "Seeded RNG with " << seed << ". About to perform " << N << " loops." << std::endl;
std::srand(seed);

std::generate(&bb[0], &bb[ARR_SIZE], make_rand);
std::generate(&cc[0], &cc[ARR_SIZE], make_rand);

using pehs = jsplib::perf::linux_perf_event_counter_t::HWSW_EVENT_T;
jsplib::perf::PerfEventCount pc1{ pehs::FIRSTMETRIC, pehs::SECONDMETRIC, pehs::THIRDMETRIC, pehs::FOURTHMETRIC, pehs::FIFTHMETRIC };

{
// a dummy run for the purposes of filling caches
for(unsigned int i = 0; i < 2; ++i) SelectAddMul(aa1,bb,cc);
std::cout << (std::equal(&aa1[0], &aa1[ARR_SIZE], &aa2[0])?"":" ") << std::endl;
}

{
jsplib::CScopedTiming timer("SSE 4.1 version took");
jsplib::perf::ScopedEventTrigger trig { &pc1 };
for(unsigned int i = 0; i < N; ++i) SelectAddMul_sse41(aa2,bb,cc);
}

// std::cout << (std::equal(&aa1[0], &aa1[ARR_SIZE], &aa2[0])?"":".") << std::endl;
std::cout << STRINGIFY(FIRSTMETRIC) << " = " << pc1.getValue( 0 ) << '\t' << STRINGIFY(SECONDMETRIC) << " = " << pc1.getValue( 1 ) << "\tratio: " << 100.0 * pc1.getRatio( 1, 0 ) << " %\n";
std::cout << STRINGIFY(THIRDMETRIC) << " = " << pc1.getValue( 2 ) <<'\t' << STRINGIFY(FOURTHMETRIC) << " = " << pc1.getValue( 3 ) << '\t' << STRINGIFY(FIFTHMETRIC) << " : " << pc1.getValue( 4 ) << '\n';
std::cout << STRINGIFY(FOURTHMETRIC) << ":" << STRINGIFY(THIRDMETRIC) << " = " << 100.0 * pc1.getRatio( 3, 2 ) << " %\n";
std::cout << STRINGIFY(FIFTHMETRIC) << ":" << STRINGIFY(THIRDMETRIC) << " = " << 100.0 * pc1.getRatio( 4, 2 ) << " %\n";
std::cout << "Multiplexing scaling factor: " << pc1.getLastScaling() << "\n\n";

pc1.reset();
{
jsplib::CScopedTiming timer("non-SSE took");
jsplib::perf::ScopedEventTrigger trig { &pc1 };
for(unsigned int i = 0; i < N; ++i) SelectAddMul(aa1,bb,cc);
}

std::cout << STRINGIFY(FIRSTMETRIC) << " = " << pc1.getValue( 0 ) << '\t' << STRINGIFY(SECONDMETRIC) << " = " << pc1.getValue( 1 ) << "\tratio: " << 100.0 * pc1.getRatio( 1, 0 ) << " %\n";
std::cout << STRINGIFY(THIRDMETRIC) << " = " << pc1.getValue( 2 ) << '\t' << STRINGIFY(FOURTHMETRIC) << " = " << pc1.getValue( 3 ) << '\t' << STRINGIFY(FIFTHMETRIC) << " : " << pc1.getValue( 4 ) << '\n';
std::cout << STRINGIFY(FOURTHMETRIC) << ":" << STRINGIFY(THIRDMETRIC) << " = " << 100.0 * pc1.getRatio( 3, 2 ) << " %\n";
std::cout << STRINGIFY(FIFTHMETRIC) << ":" << STRINGIFY(THIRDMETRIC) << " = " << 100.0 * pc1.getRatio( 4, 2 ) << " %\n";
std::cout << "Multiplexing scaling factor: " << pc1.getLastScaling() << "\n\n";

std::cout << "outputs are " << (std::equal(&aa1[0], &aa1[ARR_SIZE], &aa2[0])?"":"NOT ") << "equal." << std::endl;

return 0;
}




