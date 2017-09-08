#!/bin/sh

GAWK=/usr/bin/gawk
SNIPPETS_DIR=../internals/
EVENTS_LIST=events.master.list

${GAWK} -f generate_HWSW_EVENT_T_enum.awk  ${EVENTS_LIST} > ${SNIPPETS_DIR}HWSW_EVENT_T.enum.snippet
${GAWK} -f generate_HWCACHE_EVENT_T_enum.awk -v enum_name=HWCACHE_EVENT_T ${EVENTS_LIST} > ${SNIPPETS_DIR}HWCACHE_EVENT_T.enum.snippet
${GAWK} -f generate_HWCACHE_EVENT_T_enum.awk -v enum_name=HWCACHE_OPID_EVENT_T ${EVENTS_LIST} > ${SNIPPETS_DIR}HWCACHE_OPID_EVENT_T.enum.snippet
${GAWK} -f generate_HWCACHE_EVENT_T_enum.awk -v enum_name=HWCACHE_OPRESULT_EVENT_T ${EVENTS_LIST} > ${SNIPPETS_DIR}HWCACHE_OPRESULT_EVENT_T.enum.snippet

${GAWK} -f generate_HWCACHE_EVENT_T_cases.awk -v enum_name=HWCACHE_EVENT_T ${EVENTS_LIST} > ${SNIPPETS_DIR}HWCACHE_EVENT_T.cases.snippet
${GAWK} -f generate_HWCACHE_EVENT_T_cases.awk -v enum_name=HWCACHE_OPID_EVENT_T ${EVENTS_LIST} > ${SNIPPETS_DIR}HWCACHE_OPID_EVENT_T.cases.snippet
${GAWK} -f generate_HWCACHE_EVENT_T_cases.awk -v enum_name=HWCACHE_OPRESULT_EVENT_T ${EVENTS_LIST} > ${SNIPPETS_DIR}HWCACHE_OPRESULT_EVENT_T.cases.snippet


${GAWK} -f generate_HWSW_EVENT_T_cases.awk ${EVENTS_LIST} > ${SNIPPETS_DIR}HWSW_EVENT_T.cases.snippet


${GAWK} -f generate_event_names_lut.awk -v ev_prefix=PERF_COUNT_ ${EVENTS_LIST} > ${SNIPPETS_DIR}event_names_lut.snippet

