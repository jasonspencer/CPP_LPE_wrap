
BEGIN {
	if(length(enum_name)==0) {
	print "Variable enum_name must be set." >> "/dev/stderr";
	erroroccured = 1;
	}
	if(enum_name=="HWCACHE_EVENT_T") varname = "perf_hw_cache_id";
	else if(enum_name=="HWCACHE_OPID_EVENT_T") varname = "perf_hw_cache_op_id";
	else if(enum_name=="HWCACHE_OPRESULT_EVENT_T") varname = "perf_hw_cache_op_result_id";
	else erroroccured = 1;
	if(erroroccured) exit 1;
	}

($1 == enum_name) {
	for (i=2; i<=NF;i++) {
		print "EVENTCOUNTSWITCHCASE ( " varname ", " $1 ", " $i " )"
		}
	}
