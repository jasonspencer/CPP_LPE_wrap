#ifndef _JSPLIBPERFSCOPEDEVENTTRIGGER_HH_
#define _JSPLIBPERFSCOPEDEVENTTRIGGER_HH_

#include <array>	//std::array
#include <utility>	// std::move
#include "EventBase.hh"	// EventBase

#ifndef SCOPEDEVENTTRIGGERMAXSIZE
#define SCOPEDEVENTTRIGGERMAXSIZE 4
#endif

namespace jsplib { namespace perf {
	 
class ScopedEventTrigger {

	protected:

		std::array <EventBase *, SCOPEDEVENTTRIGGERMAXSIZE> m_ppecs;
		size_t m_npecs;

	public:

		explicit ScopedEventTrigger ( std::initializer_list<EventBase *> args ) : m_npecs(0) {
//			static_assert ( args.size() <= SCOPEDEVENTTRIGGERMAXSIZE, "List of events passed to ScopedEventTrigger::ScopedEventTrigger( .. ) is longer than SCOPEDEVENTTRIGGERMAXSIZE. Please adjust accordingly." );
			assert ( args.size() <= SCOPEDEVENTTRIGGERMAXSIZE );
			std::copy ( args.begin(), args.end(), m_ppecs.begin() );
			m_npecs = args.size();
			for(size_t i = 0; i < m_npecs; ++i) m_ppecs[i]->start();
			}

		// prevent copying of ScopedEventTrigger
		ScopedEventTrigger(ScopedEventTrigger const &) = delete;
		ScopedEventTrigger& operator= ( ScopedEventTrigger const & other ) = delete;
/*		// prevent dynamic allocation to maintain RAII semantics
		static void *operator new     (size_t) = delete;
		static void *operator new[]   (size_t) = delete;
		static void  operator delete  (void*)  = delete;
		static void  operator delete[](void*)  = delete; */

		ScopedEventTrigger (ScopedEventTrigger && other) noexcept : m_ppecs(std::move(other.m_ppecs)), m_npecs(other.m_npecs) {
			other.m_npecs = 0;
			}
		ScopedEventTrigger & operator=(ScopedEventTrigger && other) {
			if(this != &other) {
				m_ppecs = std::move(other.m_ppecs);
				}
			return *this;
			}
		~ScopedEventTrigger () {
			// TODO - stop in reverse? add direction as template parameter?
			for(size_t i = 0; i < m_npecs; ++i) m_ppecs[i]->stop();
			}
	};
} } // namespace jsplib::perf

#endif // _JSPLIBPERFSCOPEDEVENTTRIGGER_HH_
