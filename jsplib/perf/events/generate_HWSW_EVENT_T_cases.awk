

$1 ~ /HWSW_EVENT_T/ {
	for (i=3; i<=NF;i++) {
		print "\t\t\tEVENTTYPESWITCHCASE ( HWSW_EVENT_T, " $2 ", " $i " )"
		}
	}
