#!/usr/bin/env python
# Copyright (C) 2015 TOSHIBA corp.
#
# This program is free software; you can redistribute it and/or
# modify it under the terms of the GNU General Public License
# as published by the Free Software Foundation; either version 2
# of the License, or (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program; if not, write to the Free Software
# Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
import sys
import re
import pprint
import operator
from collections import Counter
import matplotlib.pyplot as plt

# get_syscall_table: open Linux unistd file and returns a dictionary that
# maps syscall names to syscall numbers
def get_syscall_table(unistd):
	syscall_table = dict()
	for line in open(unistd):
		line = line.strip()
		if not line:
			continue
		## Example: #define __NR_mbind		274
		parts = re.findall('^#define __NR_+(\w+)\W*(\d+)', line)
		if parts:
			syscall_table[parts[0][0]] = int(parts[0][1])
			continue
		# Example: define __NR_timer_settime	(__NR_timer_create+1)
		parts = re.findall('^#define __NR_(\w+)\W*\(__NR_(\w+)\+(\d+)', line)
		if parts:
			syscall_table[parts[0][0]] = syscall_table[parts[0][1]] + int(parts[0][2])
	return syscall_table

# get_syscall_seq: from a given tracelog (trace-cmd) it returns a list of
# syscall numbers in the order they were called.
def get_syscall_seq(tracelog, syscall_table):
	syscall_seq = list()
	for line in open(tracelog):
		line = line.strip()
		if not line:
			continue
		# lspci-6804  [003] 113436.967626: sys_enter_access: filename: 0xb779d105, mode: 0x0000
		parts = re.search('sys_enter_(\w+):', line)
		if not parts:
			continue
		key = parts.group(1)
		# sys_enter_xxx function names != syscall name sometimes
		if key == 'mmap_pgoff':
			key = 'mmap2'
		if key == 'newuname':
			key = 'uname'
		if key == 'newstat':
			key = 'stat'
		if key == 'newfstat':
			key = 'fstat'
		syscall_seq.append(syscall_table[key])
	return syscall_seq

# get_next_syscalls: it returns a list of syscall numbers that were executed
# just after the given syscall number.
def get_next_syscalls(syscall, syscall_seq):
	return [syscall_seq[i + 1] for i, x in enumerate(syscall_seq[:-1]) if x == syscall]

# get_next_syscalls_table: creates a table (a list of sets) containing, for
# each syscall number, a set with all the possible next syscall numbers
def get_next_syscalls_table(syscall_seq):
	next_syscalls_table = dict()
	for syscall in set(syscall_seq):
		next_syscalls_table[syscall] = set(get_next_syscalls(syscall, syscall_seq))
	return next_syscalls_table

# get_syscall_histogram: given a dictionary {syscall num : syscall name}
# and the syscall sequence, it returns a counter object with:
#   --> counter.keys(): syscall names
#   --> counter.values(): frequency (number of times the syscall was called)
def get_syscall_histogram(syscall_table_inv, syscall_seq):
	name_seq = [syscall_table_inv[x] for x in syscall_seq]
	return Counter(name_seq);

def plot_syscall_seq(syscall_seq, command):
	# [Alt] print "\nThe sequence was:"
	# [Alt] print ",".join(str(x) for x in syscall_seq)
	plt.plot(xrange(len(syscall_seq)), syscall_seq)
	plt.ylabel('syscall number id')
	plt.xlabel('timeline')
	plt.title('system call sequence for \"%r\"' % command)
	plt.savefig('syscall-seq-%s.png' % command.replace("/","-"))
	# [Alt]plt.show()

def plot_syscall_hist(syscall_histogram, command):
	h = plt.bar(xrange(len(syscall_histogram)), syscall_histogram.values(), label=syscall_histogram.keys())
	plt.subplots_adjust(bottom=0.3)
	xticks_pos = [0.65*patch.get_width() + patch.get_xy()[0] for patch in h]
	plt.ylabel('number of executions')
	plt.xlabel('syscalls')
	plt.title('Histogram of syscalls for \"%r\"' % command)
	plt.xticks(xticks_pos, syscall_histogram.keys(),  ha='right', rotation=45)
	plt.savefig('syscall-hist-%s.png' % command.replace("/","-"))

# MAIN
if __name__ == "__main__":
	if len(sys.argv) < 4:
		print "not enough arguments"
		print "Usage: ./parse-syscalls.py /path/to/unistd_32.h ftrace.log command"
		sys.exit()

	command = ' '.join(sys.argv[3:])

	# 1) parse the ftrace.log and extract the sequence of syscall numbers
	syscall_table     = get_syscall_table(sys.argv[1]) # name2num
	syscall_table_inv = {v: k for k, v in syscall_table.items()} # num2name
	syscall_seq       = get_syscall_seq(sys.argv[2], syscall_table)
	plt.figure(1)
	plot_syscall_seq(syscall_seq, command)

	# 2) get the histogram (number of times a system calls was executed)
	syscall_histogram = get_syscall_histogram(syscall_table_inv, syscall_seq)
	#print "Histogram data (total: %d syscalls)" % len(set(syscall_seq))
	#for x in syscall_histogram.items():
	#	print x[0],"(%d)" % syscall_table[x[0]],"\t", x[1], "times"
	plt.figure(2)
	plot_syscall_hist(syscall_histogram, command)

	# 3) obtain the next_syscalls_table
	next_syscalls_table = get_next_syscalls_table(syscall_seq)
	#print "\nThe next syscalls table:"
	i = 0
	print "#define NUM_SYSCALLS " + str(len(next_syscalls_table))
	for key in next_syscalls_table:
		print "idstable.entries[" + str(i) + "].syscall_nr = " + str(key) + ";"
		for x in next_syscalls_table[key]:
			print "set_bit(" + str(x) + ", idstable.entries[" + str(i) + "].next_syscalls);"
		i = i + 1
	# [Alt]
	# for key in next_syscalls_table:
	#	print str(key) + ":\t" + ",".join(str(x) for x in next_syscalls_table[key])







