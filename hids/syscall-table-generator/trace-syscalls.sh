#!/bin/bash
# root> ./trace-syscalls.sh command
if [[ $EUID -ne 0 ]]; then
	echo "Please run as root: sudo -s" 1>&2
	exit 1
fi

DIR=$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )
trace-cmd record -e 'syscalls:sys_enter_*' -F $@ > /dev/null 2>&1
trace-cmd report > ftrace.log

ARCH=$( uname -m )
if [[ "$ARCH" = "x86_64" ]]; then
	$DIR/parse-syscalls.py /usr/include/x86_64-linux-gnu/asm/unistd_64.h ftrace.log $@
else
	$DIR/parse-syscalls.py /usr/include/i386-linux-gnu/asm/unistd_32.h ftrace.log $@
fi

