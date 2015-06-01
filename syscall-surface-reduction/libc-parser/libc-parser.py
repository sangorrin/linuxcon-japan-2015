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
import subprocess

if __name__ == "__main__":
	if len(sys.argv) < 2:
		print "not enough arguments"
		print "Usage: ./libc-parser.py binary.exe"
		sys.exit()

	output = subprocess.check_output(['nm', '-u', sys.argv[1]]).split('\n')
	for line in output:
		line = line.strip()
		if not line:
			continue
		parts = re.search('U (\w+)@', line)
		if not parts:
			continue
		syscall = parts.group(1)
		print syscall


