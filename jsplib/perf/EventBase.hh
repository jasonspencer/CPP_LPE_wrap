#ifndef _JSPLIBPERFEVENTBASE_H_
#define _JSPLIBPERFEVENTBASE_H_

namespace jsplib { namespace perf {
		
	class EventBase {

	public:
		virtual void start() = 0;
		virtual void stop() = 0;
		virtual void reset() = 0;
		virtual ~EventBase() {};
	};

} } // namespace jsplib::perf

#endif	// _JSPLIBPERFEVENTBASE_H_
