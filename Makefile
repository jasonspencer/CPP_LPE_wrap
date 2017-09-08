
# CC = clang++
CC = g++
RM = rm -f
CFLAGS = -O3 -Wall -Wextra -Werror -Wpedantic -std=c++11
SSE4FLAGS = -msse4.2
JSPLIBDIR = jsplib

STRLENCOMPOPTS = -DFIFTHMETRIC=linux_perf_event_counter_t::HWSW_EVENT_T::HW_REF_CPU_CYCLES
SIMDTESTOPTS = # -DFIRSTMETRIC=HW_CACHE_REFERENCES -DSECONDMETRIC=HW_CACHE_MISSES
SIMDTESTOPTS = # -DFIFTHMETRIC=HW_INSTRUCTIONS
SIMDTESTOPTS = -DFIFTHMETRIC=HW_REF_CPU_CYCLES
INCLUDEPATH = -I.

# targets = mystrlen_test_multi_event mystrlen_test_multi_event_long_words strlen_comparisons strlen_comparisons_long_words strlen_comparisons_nosse4 strlen_comparisons_nosse4_long_words simd_test4_pmu strlen_usage_comparisons strlen_usage_comparisons_long_words nop nop_many overhead bitmask_test transpose
targets = mystrlen_test_multi_event mystrlen_test_multi_event_long_words strlen_comparisons strlen_comparisons_long_words strlen_comparisons_nosse4 strlen_comparisons_nosse4_long_words simd_test4_pmu strlen_usage_comparisons strlen_usage_comparisons_long_words nop nop_many overhead transpose

.DEFAULT_GOAL := all

.PHONY: all clean run_tests

all: $(targets)

mystrlen_test_multi_event: $(JSPLIBDIR)/perf/PerfEventCount.hh $(JSPLIBDIR)/perf/ScopedEventTrigger.hh $(JSPLIBDIR)/perf/WallTimeEvent.hh $(JSPLIBDIR)/pp/Stringify.hh testing/mystrlen_test_multi_event.cc
	$(CC) $(CFLAGS) $(INCLUDEPATH) -o $@ testing/mystrlen_test_multi_event.cc $(STRLENCOMPOPTS)

mystrlen_test_multi_event_long_words: $(JSPLIBDIR)/perf/PerfEventCount.hh $(JSPLIBDIR)/perf/ScopedEventTrigger.hh $(JSPLIBDIR)/perf/WallTimeEvent.hh $(JSPLIBDIR)/pp/Stringify.hh testing/mystrlen_test_multi_event.cc
	$(CC) $(CFLAGS) $(INCLUDEPATH) -o $@ testing/mystrlen_test_multi_event.cc -DUSELONGWORDS $(STRLENCOMPOPTS)

strlen_comparisons: $(JSPLIBDIR)/perf/PerfEventCount.hh $(JSPLIBDIR)/perf/ScopedEventTrigger.hh $(JSPLIBDIR)/CScopedTiming.hh $(JSPLIBDIR)/pp/Stringify.hh testing/strlen_comparisons.cc
	$(CC) $(CFLAGS) $(SSE4FLAGS) $(INCLUDEPATH) -o $@ testing/strlen_comparisons.cc $(STRLENCOMPOPTS)

strlen_comparisons_long_words: $(JSPLIBDIR)/perf/PerfEventCount.hh $(JSPLIBDIR)/perf/ScopedEventTrigger.hh $(JSPLIBDIR)/CScopedTiming.hh $(JSPLIBDIR)/pp/Stringify.hh testing/strlen_comparisons.cc
	$(CC) $(CFLAGS) $(SSE4FLAGS) $(INCLUDEPATH) -o $@ testing/strlen_comparisons.cc -DUSELONGWORDS $(STRLENCOMPOPTS)

strlen_comparisons_nosse4: $(JSPLIBDIR)/perf/PerfEventCount.hh $(JSPLIBDIR)/perf/ScopedEventTrigger.hh $(JSPLIBDIR)/CScopedTiming.hh $(JSPLIBDIR)/pp/Stringify.hh testing/strlen_comparisons.cc
	$(CC) $(CFLAGS) $(INCLUDEPATH) -o $@ testing/strlen_comparisons.cc -DDISABLESSE4STRLEN $(STRLENCOMPOPTS)

strlen_comparisons_nosse4_long_words: $(JSPLIBDIR)/perf/PerfEventCount.hh $(JSPLIBDIR)/perf/ScopedEventTrigger.hh $(JSPLIBDIR)/CScopedTiming.hh $(JSPLIBDIR)/pp/Stringify.hh testing/strlen_comparisons.cc
	$(CC) $(CFLAGS) $(INCLUDEPATH) -o $@ testing/strlen_comparisons.cc -DUSELONGWORDS -DDISABLESSE4STRLEN $(STRLENCOMPOPTS)

simd_test4_pmu: $(JSPLIBDIR)/perf/PerfEventCount.hh $(JSPLIBDIR)/perf/ScopedEventTrigger.hh $(JSPLIBDIR)/CScopedTiming.hh $(JSPLIBDIR)/pp/Stringify.hh testing/simd_test4_pmu.cc
	$(CC) $(CFLAGS) $(SSE4FLAGS) $(INCLUDEPATH) -o $@ testing/simd_test4_pmu.cc -DUSELONGWORDS -DDISABLESSE4STRLEN $(SIMDTESTOPTS)

strlen_usage_comparisons: $(JSPLIBDIR)/perf/PerfEventCount.hh $(JSPLIBDIR)/perf/ScopedEventTrigger.hh $(JSPLIBDIR)/CScopedTiming.hh $(JSPLIBDIR)/pp/Stringify.hh testing/strlen_usage_comparisons.cc
	$(CC) $(CFLAGS) $(SSE4FLAGS) $(INCLUDEPATH) -o $@ testing/strlen_usage_comparisons.cc $(STRLENCOMPOPTS)

strlen_usage_comparisons_long_words: $(JSPLIBDIR)/perf/PerfEventCount.hh $(JSPLIBDIR)/perf/ScopedEventTrigger.hh $(JSPLIBDIR)/CScopedTiming.hh $(JSPLIBDIR)/pp/Stringify.hh testing/strlen_usage_comparisons.cc
	$(CC) $(CFLAGS) $(SSE4FLAGS) $(INCLUDEPATH) -o $@ testing/strlen_usage_comparisons.cc -DUSELONGWORDS $(STRLENCOMPOPTS)

nop: $(JSPLIBDIR)/perf/PerfEventCount.hh $(JSPLIBDIR)/perf/ScopedEventTrigger.hh $(JSPLIBDIR)/CScopedTiming.hh $(JSPLIBDIR)/pp/Stringify.hh testing/nop.cc
	$(CC) $(CFLAGS) $(INCLUDEPATH) -o $@ testing/nop.cc

nop_many: $(JSPLIBDIR)/perf/PerfEventCount.hh $(JSPLIBDIR)/perf/ScopedEventTrigger.hh $(JSPLIBDIR)/pp/Stringify.hh $(JSPLIBDIR)/CScopedTiming.hh testing/nop.cc
	$(CC) $(CFLAGS) $(INCLUDEPATH) -o $@ testing/nop.cc -DMANYNOPS

overhead: $(JSPLIBDIR)/perf/PerfEventCount.hh $(JSPLIBDIR)/perf/ScopedEventTrigger.hh $(JSPLIBDIR)/CScopedTiming.hh $(JSPLIBDIR)/pp/Stringify.hh testing/overhead.cc
	$(CC) $(CFLAGS) $(INCLUDEPATH) -o $@ testing/overhead.cc

# bitmask_test: $(JSPLIBDIR)/bitmask_test.cc $(JSPLIBDIR)/bitmask.hh
#	$(CC) $(CFLAGS) $(INCLUDEPATH) -o $@ $(JSPLIBDIR)/bitmask_test.cc

transpose: $(JSPLIBDIR)/perf/PerfEventCount.hh $(JSPLIBDIR)/perf/ScopedEventTrigger.hh $(JSPLIBDIR)/CScopedTiming.hh $(JSPLIBDIR)/pp/Stringify.hh example/transpose.cc example/transpose_tiled_SIMD.cc
	$(CC) $(CFLAGS) $(INCLUDEPATH) -o $@ example/transpose.cc -DPCCACHESIMPLE

run_tests: bitmask_test
	@./bitmask_test | diff bitmask_test.correct_output.txt - > /dev/null && echo "bitmask_test OK" || echo "bitmask_test FAILED"
#	[ $? -ne 0 ] echo 
# jsp@machina:~/Desktop/VBox_share/CScopedPerfEventCounter$ echo $?
# 0

clean:
	$(RM) $(targets)
