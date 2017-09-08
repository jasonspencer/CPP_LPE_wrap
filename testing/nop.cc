#include <iostream>
#include <sstream>

#include <jsplib/perf/PerfEventCount.hh>
#include <jsplib/perf/ScopedEventTrigger.hh>

#include <jsplib/CScopedTiming.hh>

#include <jsplib/pp/Stringify.hh>

#ifndef FIRSTMETRIC
#define FIRSTMETRIC linux_perf_event_counter_t::HWSW_EVENT_T::HW_CACHE_REFERENCES
#endif

#ifndef SECONDMETRIC
#define SECONDMETRIC linux_perf_event_counter_t::HWSW_EVENT_T::HW_CACHE_MISSES
#endif

#ifndef THIRDMETRIC
#define THIRDMETRIC linux_perf_event_counter_t::HWSW_EVENT_T::HW_BRANCH_INSTRUCTIONS
#endif

#ifndef FOURTHMETRIC
#define FOURTHMETRIC linux_perf_event_counter_t::HWSW_EVENT_T::HW_BRANCH_MISSES
#endif

#ifndef FIFTHMETRIC
#define FIFTHMETRIC linux_perf_event_counter_t::HWSW_EVENT_T::HW_INSTRUCTIONS
#endif

#ifndef SIXTHMETRIC
#define SIXTHMETRIC linux_perf_event_counter_t::HWSW_EVENT_T::HW_REF_CPU_CYCLES
#endif

int main() {

using namespace jsplib::perf;

jsplib::perf::PerfEventCount pc1{ FIRSTMETRIC, SECONDMETRIC, THIRDMETRIC, FOURTHMETRIC, FIFTHMETRIC, SIXTHMETRIC };

{
	jsplib::CScopedTiming timer("nop(s)");
	jsplib::perf::ScopedEventTrigger trig { &pc1 };
#ifdef MANYNOPS
	for(unsigned i = 0; i< 100000; ++i)
#endif
		__asm__ __volatile__ ("nop");
	;
}

std::cout << STRINGIFY(FIRSTMETRIC) << " : " << pc1.getValue( 0 ) << '\t' << STRINGIFY(SECONDMETRIC) << " : " << pc1.getValue( 1 ) << "\tratio: " << 100.0 * pc1.getRatio( 1, 0 ) << " %\n";
std::cout << STRINGIFY(THIRDMETRIC) << " : " << pc1.getValue( 2 ) << '\t' << STRINGIFY(FOURTHMETRIC) << " : " << pc1.getValue( 3 ) << "\tratio: " << 100.0 * pc1.getRatio( 3, 2 ) << " %\n";
std::cout << STRINGIFY(FIFTHMETRIC) << " : " << pc1.getValue( 4 ) << '\t' << STRINGIFY(SIXTHMETRIC) << " : " << pc1.getValue( 5 ) << '\n';
std::cout << "Multiplexing scaling factor: " << pc1.getLastScaling() << '\n';
std::cout << '\n';

return 0;

}


/* Linux perf events have a few problems - one, is the documentation. There's some around - just it doesn' always match the system's behaviour, and */

