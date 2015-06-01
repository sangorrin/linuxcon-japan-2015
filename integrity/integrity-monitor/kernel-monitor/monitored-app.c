/*
 * Copyright (C) 2015 TOSHIBA corp.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */
#include <unistd.h>
#include <sys/mman.h> /* for mlockall */
#include <assert.h>
#include <stdio.h>

int main()
{
	int ret;

	ret = mlockall(MCL_CURRENT | MCL_FUTURE);
	if (ret != 0)
		perror("mlockall failed:");

	while(1)
	{
		sleep(1);
	}
	return 0;
}
