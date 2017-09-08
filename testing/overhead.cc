#include <iostream>
#include <sstream>
#include <cstdlib> // strtoul
#include <cctype> // isdigit

#ifdef DISABLE_EVCOUNT_DESCRIPTION
#undefine DISABLE_EVCOUNT_DESCRIPTION
#endif

#include <jsplib/perf/PerfEventCount.hh>
#include <jsplib/perf/ScopedEventTrigger.hh>

#include <jsplib/CScopedTiming.hh>

#include <jsplib/pp/Stringify.hh>

namespace perf = jsplib::perf;

#ifndef FIRSTMETRIC
#define FIRSTMETRIC perf::linux_perf_event_counter_t::HWSW_EVENT_T::HW_CACHE_REFERENCES
#endif

#ifndef SECONDMETRIC
#define SECONDMETRIC perf::linux_perf_event_counter_t::HWSW_EVENT_T::HW_CACHE_MISSES
#endif

#ifndef THIRDMETRIC
#define THIRDMETRIC perf::linux_perf_event_counter_t::HWSW_EVENT_T::HW_BRANCH_INSTRUCTIONS
#endif

#ifndef FOURTHMETRIC
#define FOURTHMETRIC perf::linux_perf_event_counter_t::HWSW_EVENT_T::HW_BRANCH_MISSES
#endif

#ifndef FIFTHMETRIC
#define FIFTHMETRIC perf::linux_perf_event_counter_t::HWSW_EVENT_T::HW_REF_CPU_CYCLES
#endif

#ifndef SIXTHMETRIC
#define SIXTHMETRIC perf::linux_perf_event_counter_t::HWSW_EVENT_T::HW_INSTRUCTIONS
#endif

void printSixMetricEventCountStats ( std::ostream & os, const perf::PerfEventCount & pc ) {
os << pc.getDescription(0) << " : " << pc.getValue( 0 ) << '\t' << pc.getDescription(1) << " : " << pc.getValue( 1 ) << "\tratio: " << 100.0 * pc.getRatio( 1, 0 ) << " %\n";
os << pc.getDescription(2) << " : " << pc.getValue( 2 ) << '\t' << pc.getDescription(3) << " : " << pc.getValue( 3 ) << "\tratio: " << 100.0 * pc.getRatio( 3, 2 ) << " %\n";
os << pc.getDescription(4) << " : " << pc.getValue( 4 ) << '\t' << pc.getDescription(5) << " : " << pc.getValue( 5 ) << '\n';
os << "Multiplexing scaling factor: " << pc.getLastScaling() << '\n';
}


void usage(const char * exec_name) {
	std::cerr << exec_name << " [unsigned integer number of nop loops to perform]" << std::endl;
}

void resetCounts( std::initializer_list<perf::PerfEventCount *> pecs ) {
	for( auto & pec : pecs ) pec->reset();
}

inline void runInternalOverheadTestOnPEC(perf::PerfEventCount & pec ) {
	{
		perf::ScopedEventTrigger trig1 { &pec };
	}
std::cout << pec.getDescription(0) << " : " << pec.getValue( 0 ) << '\t' << pec.getDescription(1) << " : " << pec.getValue( 1 ) << "\tratio: " << 100.0 * pec.getRatio( 1, 0 ) << " %\n";
std::cout << "Multiplexing scaling factor: " << pec.getLastScaling() << '\n';
}

inline void runOverheadTestOnPECPair(perf::PerfEventCount & pecOuter, perf::PerfEventCount & pecInner ) {
	{
		perf::ScopedEventTrigger trig2 { &pecOuter };
		perf::ScopedEventTrigger trig1 { &pecInner };
	}
std::cout << pecOuter.getDescription(0) << " : " << pecOuter.getValue( 0 ) << '\t' << pecOuter.getDescription(1) << " : " << pecOuter.getValue( 1 ) << "\tratio: " << 100.0 * pecOuter.getRatio( 1, 0 ) << " %\n";
std::cout << "Multiplexing scaling factor: " << pecOuter.getLastScaling() << '\n';
}

inline void printPECDeltas(perf::PerfEventCount & firstPEC, perf::PerfEventCount & secondPEC ) {
std::cout << firstPEC.getDescription(0) << " : " << (firstPEC.getValue( 0 )-secondPEC.getValue( 0 )) << '\t' << firstPEC.getDescription(1) << " : " << (firstPEC.getValue( 1 )-secondPEC.getValue( 1 )) << '\n';
}

int main ( int, char * [] ) {

// using namespace perf;

perf::PerfEventCount pecSixTuple{ FIRSTMETRIC, SECONDMETRIC, THIRDMETRIC, FOURTHMETRIC, FIFTHMETRIC, SIXTHMETRIC };
{
	perf::ScopedEventTrigger trig { &pecSixTuple };
}
std::cout << "\n** Internal overhead for six events in one group:\n";
printSixMetricEventCountStats ( std::cout, pecSixTuple );

// for(unsigned i = 0; i < pecSixTuple.getNumEvents(); ++i ) std::cout << pecSixTuple.getDescription(i) << '\n';

perf::PerfEventCount pecPairInternalTest1 { FIRSTMETRIC, SECONDMETRIC };
perf::PerfEventCount pecPairInternalTest2 { THIRDMETRIC, FOURTHMETRIC };
perf::PerfEventCount pecPairInternalTest3 { FIFTHMETRIC, SIXTHMETRIC };
std::cout << "\n** Internal overheads for two events in one group:\n";
runInternalOverheadTestOnPEC ( pecPairInternalTest1  );
runInternalOverheadTestOnPEC ( pecPairInternalTest2 );
runInternalOverheadTestOnPEC ( pecPairInternalTest3 );

perf::PerfEventCount pecPairPairTest1 { FIRSTMETRIC, SECONDMETRIC };
perf::PerfEventCount pecPairPairTest2 { THIRDMETRIC, FOURTHMETRIC };
perf::PerfEventCount pecPairPairTest3 { FIFTHMETRIC, SIXTHMETRIC };
perf::PerfEventCount pecPairPairTest2dummy { THIRDMETRIC, FOURTHMETRIC };
perf::PerfEventCount pecPairPairTest3dummy { FIFTHMETRIC, SIXTHMETRIC };
std::cout << "\n** Internal overheads for two events in one group, and total overhead for two events in one group:\n";
resetCounts({&pecPairPairTest1, &pecPairPairTest3dummy});
runOverheadTestOnPECPair( pecPairPairTest1, pecPairPairTest2 );
resetCounts({&pecPairPairTest2, &pecPairPairTest3dummy});
runOverheadTestOnPECPair( pecPairPairTest2, pecPairPairTest3dummy );
resetCounts({&pecPairPairTest3, &pecPairPairTest2dummy});
runOverheadTestOnPECPair( pecPairPairTest3, pecPairPairTest2dummy );

std::cout << "\n** Deltas, ie total overhead for two events in one group:\n";
printPECDeltas ( pecPairPairTest1, pecPairInternalTest1 );
printPECDeltas ( pecPairPairTest2, pecPairInternalTest2 );
printPECDeltas ( pecPairPairTest3, pecPairInternalTest3 );

std::cout << std::endl;

return 0;
}

/* Linux perf events have a few problems - one, is the documentation. There's some around - just it doesn' always match the system's behaviour, and */

