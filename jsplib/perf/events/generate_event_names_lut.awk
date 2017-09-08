#	{ {1,2}, "foo" },#
#	{ {1,2}, "foo" }#,
#	{ {1,2}, "foo" },
#	{ {1,2}, "foo" }

BEGIN {
n_output_lines = 0;
n_cache_events = 0;
n_cache_opid = 0;
n_cache_opresult = 0;
}

($1 == "HWSW_EVENT_T") {
	for (i=3; i<=NF;i++) {
		output_lines[n_output_lines] = "\t{ {" $2 ", " ev_prefix $i" }, \"" $i "\" }";
		n_output_lines++;
		}
	}

($1 == "HWCACHE_EVENT_T") {
	for (i=2; i<=NF;i++) {
		cache_events[n_cache_events] = $i;
		n_cache_events++;
		}
}

($1 == "HWCACHE_OPID_EVENT_T") {
	for (i=2; i<=NF;i++) {
		cache_opid[n_cache_opid] = $i;
		n_cache_opid++;
		}
}

($1 == "HWCACHE_OPRESULT_EVENT_T") {
	for (i=2; i<=NF;i++) {
		cache_opresult[n_cache_opresult] = $i;
		n_cache_opresult++;
		}
}

END {
for(ev_i=0;ev_i<n_cache_events;ev_i++)
	for(opid_i=0;opid_i<n_cache_opid;opid_i++)
		for(opre_i=0;opre_i<n_cache_opresult;opre_i++) {
			ev = cache_events[ev_i];
			opid = cache_opid[opid_i];
			opre = cache_opresult[opre_i];
			desc = ev "/" opid "/" opre;
			output_lines[n_output_lines] = "\t{ { PERF_TYPE_HW_CACHE , MAKE_HW_CACHE_CONFIG( " ev_prefix ev ", " ev_prefix opid ", " ev_prefix opre ") }, \"" desc "\" }";
			n_output_lines++;
		}

for(line=0;line<n_output_lines;line++) print output_lines[line] ",";
print "\t{ { 0, 0 }, \"\\0\" }";
# // print output_lines[line];
}
