

BEGIN {
	n_events=0;
	}

$1 ~ /HWSW_EVENT_T/ {
	for (i=3; i<=NF;i++) {
		event_names[n_events] = $i;
		n_events++;
		}
	}

END	{
	printf "enum class HWSW_EVENT_T { ";
	for ( i=0; i < (n_events-1); i++ ) printf event_names[i] ", ";
	if(n_events!=0) printf event_names[i];
	print " };\n";
}
