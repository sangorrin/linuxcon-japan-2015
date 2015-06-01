#!/bin/bash
# Usage: ./seccomp-ids-wrapper.sh app <args>
if [[ $EUID -ne 0 ]]; then
	echo "Please run as root: sudo -s" 1>&2
	exit 1
fi
../../syscall-table-generator/trace-syscalls.sh $@ > parsed_trace
(cat parsed_trace | tail -n +2) > seccomp_settings
head -1 parsed_trace > num_syscalls
gcc seccomp-ids-wrapper.c -o seccomp-ids-wrapper
rm -rf parsed_trace num_syscalls seccomp_settings trace.dat ftrace.log
./seccomp-ids-wrapper $@


