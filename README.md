# About CPP\_LPE\_wrap

This is a *pre-alpha* release of my **CPP\_LPE\_wrap** C++ library.
It is a collection of classes to configure and read Linux Performance Events (LPE) counters, and includes a convenient RAII-style counter trigger for the runtime interrogation of hardware and software performance events.

**CPP\_LPE\_wrap** is designed to be as lightweight as possible on top of the actual LPE subsystem, which perhaps isn't the most lightweight, so don't expect to profile a few instructions inside a loop, rather all the iterations of the loop (profiling a few instructions is hard enough as it is, considering compiler and CPU re-ordering).

Perhaps in the future there will be a version of this library that uses direct access to the Performance Monitoring Units (PMUs) (via RDPMC on x86), but that won't be architecture neutral like LPE is and there is a fair amount of work to implement such a wrapper as privileged (Ring-0) access is required to configure (but not read) the PMUs.

Since this library uses Linux Performance Events, it naturally only works under Linux.

As this is a pre-alpha release you should expect API breakage, renaming and design changes in the future.


## Compared to perf

**CPP\_LPE\_wrap** is designed to complement the **perf** command line tool.  **perf-record** and **perf-report** work more like a standard debugger annotating the disassembled code with the percentage of event counts at various functions or individual instructions. While **perf-stat** captures events for the entire run of an executable, and outputs totals at the end. **jsplib::PerfEventCount**, on the other hand, is inserted into the code, triggered arbitrarily, and makes the event counts available at runtime.

## Use cases

The use cases for **CPP\_LPE\_wrap** include the optimization of critical paths, while not capturing events for less critical code. **CPP\_LPE\_wrap** could also be used as part of the testing framework, alongside unit testing: it would be possible to test for an increase in cache misses, or page faults, and when code is changed a test fail is triggered if these are found to be different from nominal values.
Another use case would be to build a benchmarking routine into a program that after collecting event data would then optimise the configuration of the program, for example a program may test for page faults to guide the selection of a different data structure to optimise performance to the given hardware, or based on context-switch events recommend a change in the environment's scheduler.

If using hardware counters (PMUs), which are limited in number (to about four, but this depends on CPU and event type), it is possible to count one type for a certain section of code, and count another event type in another section, without the need for event multiplexing.

## Usage

There are two main classes: **jsplib::perf::PerfEventCount** and **jsplib::perf::ScopedEventTrigger**.
**PerfEventCount** holds the counter details, and the count, while **ScopedEventTrigger** is a class that starts and stops the counting when it is created and destroyed.

The basic usage is simple:

	using pehs = jsplib::perf::linux_perf_event_counter_t::HWSW_EVENT_T;

	jsplib::perf::PerfEventCount pc {
		{ pehs::HW_CACHE_REFERENCES  }, 
		{ pehs::HW_CACHE_MISSES },
		{ pehs::HW_REF_CPU_CYCLES }
	};

	{
	jsplib::perf::ScopedEventTrigger trig { &pc };
	// code to profile
	}
	
	std::cout << pc.getDescription ( 0 ) << " : " << pc.getValue ( 0 ) << '\t' << pc.getDescription ( 1 ) << " : " << pc.getValue ( 1 ) << "\tratio: " << 100.0 * pc.getRatio ( 1, 0 ) << " %\n";

	std::cout << pc.getDescription ( 2 ) << " : " << pc.getValue ( 2 ) << '\n';

	std::cout << "Multiplexing scaling factor: " << pc.getLastScaling() << '\n';

To minimise system calls, and therefore the overhead of context switches, all the events passed to the **PerfEventCount** are initialised as a group - this also means that they can be compared directly, so in the example above the count of **HW\_CACHE\_REFERENCES** can be compared against **HW\_CACHE\_MISSES**, and the same is true of the example below:

	using cev    = jsplib::perf::linux_perf_event_counter_t::HWCACHE_EVENT_T;
	using copid  = jsplib::perf::linux_perf_event_counter_t::HWCACHE_OPID_EVENT_T;
	using copres = jsplib::perf::linux_perf_event_counter_t::HWCACHE_OPRESULT_EVENT_T;

	jsplib::perf::PerfEventCount perf_counter {
		{ cev::HW_CACHE_L1D, 	copid::HW_CACHE_OP_READ,	copres::HW_CACHE_RESULT_ACCESS },
		{ cev::HW_CACHE_L1D, 	copid::HW_CACHE_OP_READ,	copres::HW_CACHE_RESULT_MISS },
	};

Here we have configured perf_counter to count access and miss events in the Level 1 data cache only.

## Available counters and their configuration

The constructor for **jsplib::perf::PerfEventCount** takes a single argument - an initializer_list of **jsplib::perf::linux\_perf\_event\_counter\_t** objects.  These objects encapsulate the configuration of the event counter, be it hardware or software events.

Please read the [man page for perf\_event\_open](http://man7.org/linux/man-pages/man1/perf.1.html) to understand the events types.  **CPP\_LPE\_wrap** supports events of type **PERF\_TYPE\_HARDWARE**, **PERF\_TYPE\_SOFTWARE**, **PERF\_TYPE\_HW\_CACHE** and **PERF\_TYPE\_RAW**.

<br />

One of the **linux\_perf\_event\_counter\_t** constructors takes a value from the **HWSW\_EVENT\_T** enum - which map from the the values in the Linux kernel header (and listed on the man page), but without the **PERF\_COUNT\_** prefix, so:

**PERF\_COUNT\_SW\_CONTEXT\_SWITCHES** becomes **jsplib::perf::linux\_perf\_event\_counter\_t::HWSW\_EVENT\_T::SW\_CONTEXT\_SWITCHES**

**PERF\_COUNT\_HW\_BRANCH\_MISSES** becomes **jsplib::perf::linux\_perf\_event\_counter\_t::HWSW\_EVENT\_T::HW\_BRANCH\_MISSES**.

Currently the accepted tokens from the jsplib::perf::linux\_perf\_event\_counter\_t::HWSW\_EVENT\_T enum are:

* HW\_CPU\_CYCLES
* HW\_INSTRUCTIONS
* HW\_CACHE\_REFERENCES
* HW\_CACHE\_MISSES
* HW\_BRANCH\_INSTRUCTIONS
* HW\_BRANCH\_MISSES
* HW\_BUS\_CYCLES
* HW\_STALLED\_CYCLES\_FRONTEND
* HW\_STALLED\_CYCLES\_BACKEND
* HW\_REF\_CPU\_CYCLES
* SW\_CPU\_CLOCK
* SW\_TASK\_CLOCK
* SW\_PAGE\_FAULTS
* SW\_CONTEXT\_SWITCHES
* SW\_CPU\_MIGRATIONS
* SW\_PAGE\_FAULTS\_MIN
* SW\_PAGE\_FAULTS\_MAJ
* SW\_ALIGNMENT\_FAULTS
* SW\_EMULATION\_FAULTS

Another **linux\_perf\_event\_counter\_t** constructor takes three enums which map from the hardware cache event names in the man page prefixed by **PERF\_COUNT\_HW\_CACHE\_**.

**PERF\_COUNT\_HW\_CACHE\_L1D** becomes **jsplib::perf::linux\_perf\_event\_counter\_t::HWCACHE\_EVENT\_T::HW\_CACHE\_L1D**

**PERF\_COUNT\_HW\_CACHE\_OP\_READ** becomes **jsplib::perf::linux\_perf\_event\_counter\_t::HWCACHE\_OPID\_EVENT\_T::HW\_CACHE\_OP\_READ**

**PERF\_COUNT\_HW\_CACHE\_RESULT\_ACCESS** becomes **jsplib::perf::linux\_perf\_event\_counter\_t::HWCACHE\_OPRESULT\_EVENT\_T::HW\_CACHE\_RESULT\_ACCESS**

Currently the first enum (jsplib::perf::linux\_perf\_event\_counter\_t::HWCACHE\_EVENT\_T) accepted tokens are:

* HW\_CACHE\_L1D
* HW\_CACHE\_L1I
* HW\_CACHE\_LL
* HW\_CACHE\_DTLB
* HW\_CACHE\_ITLB
* HW\_CACHE\_BPU
* HW\_CACHE\_NODE

The second token from jsplib::perf::linux\_perf\_event\_counter\_t::HWCACHE\_OPID\_EVENT\_T is one of:

* HW\_CACHE\_OP\_READ
* HW\_CACHE\_OP\_WRITE
* HW\_CACHE\_OP\_PREFETCH

And the third token from jsplib::perf::linux\_perf\_event\_counter\_t::HWCACHE\_OPRESULT\_EVENT\_T is:

* HW\_CACHE\_RESULT\_ACCESS
* HW\_CACHE\_RESULT\_MISS

There is also a constructor to which you can supply raw configuration values as per a call to **perf\_event\_open** with type **PERF\_TYPE\_RAW**.

All events listed in the **jsplib::perf::PerfEventCount** constructor are grouped together, and when the counter is triggered all the events are passed to the OS as a group and all start counting together.

Once constructed the **jsplib::perf::PerfEventCount** contains the running count for the configured event types and can be interrogated with ::getValue( index ) and ::getDescription( index ), where index is the zero based index into the events listed in the intializer_list passed to the constructor.

## WallTimeEvent

Included in this project is also a related class to count elapsed wall time.

**jsplib::perf::WallTimeEvent** works in both Windows and Linux and uses the system clock (most likely tied to RDTSC) to count the elapsed wall clock time between its creation and destruction.

## TODO

Things that need work:

1. a nicer interface to map event names to strings.
2. less reliance on explicit template parameters (new C++17 features will help with that).

