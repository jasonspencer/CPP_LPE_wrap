

BEGIN {
	if(length(enum_name)==0) {
		print "Variable enum_name must be set." >> "/dev/stderr";
		erroroccured = 1;
		exit 1;
	}
	n_events=0;
	}

($1 == enum_name) {
	for (i=2; i<=NF;i++) {
		event_names[n_events] = $i;
		n_events++;
		}
	}

END	{
	if(erroroccured) exit 1;
	printf "enum class " enum_name " { ";
	for ( i=0; i < (n_events-1); i++ ) printf event_names[i] ", ";
	if(n_events!=0) printf event_names[i];
	print " };\n";
}
