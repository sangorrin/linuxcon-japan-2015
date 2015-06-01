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
// TEST: SECCOMP_MODE_IDS: benchmark system calls (use from python script)
#define _GNU_SOURCE 1

#include <stdio.h>
#include <stddef.h>
#include <stdlib.h>
#include <stdint.h>
#include <unistd.h>
#include <errno.h>
#include <linux/audit.h>
#include <syscall.h>
#include <time.h>	/* for clock_gettime */

#include <sys/prctl.h>
#include <linux/filter.h>
#include <linux/seccomp.h>
#include <asm/unistd.h>

# define REG_SYSCALL	REG_EAX
# define ARCH_NR	AUDIT_ARCH_I386

#define SECCOMP_MODE_IDS 3

#define NR_syscalls 350
#define BITS_TO_LONGS(nr)       DIV_ROUND_UP(nr, BITS_PER_BYTE * sizeof(long))
#define DECLARE_BITMAP(name,bits) unsigned long name[BITS_TO_LONGS(bits)]
#define BITS_PER_BYTE           8
#define DIV_ROUND_UP(n,d) (((n) + (d) - 1) / (d))
#define BITS_PER_LONG __WORDSIZE
#define BIT_MASK(nr)            (1UL << ((nr) % BITS_PER_LONG))
#define BIT_WORD(nr)            ((nr) / BITS_PER_LONG)

static inline void set_bit(int nr, volatile unsigned long *addr) {
	unsigned long mask = BIT_MASK(nr);
	unsigned long *p = ((unsigned long *)addr) + BIT_WORD(nr);
	*p  |= mask;
}

struct seccomp_idsentry {
	uint32_t syscall_nr;
	DECLARE_BITMAP(next_syscalls, NR_syscalls);
};

struct seccomp_idstable {
	uint32_t len;
	struct seccomp_idsentry *entries;
};

#define NUM_SYSCALLS 314

static int install_syscall_filter(void)
{
	int i, j;
	struct seccomp_idstable idstable;

	idstable.len = NUM_SYSCALLS;
	idstable.entries = (struct seccomp_idsentry *)
		calloc(idstable.len, sizeof(struct seccomp_idsentry));

	for (i=0; i < idstable.len; i++) {
		idstable.entries[i].syscall_nr = i;
		for (j=0; j < NUM_SYSCALLS; j++) {
			set_bit(j, idstable.entries[i].next_syscalls);
		}
	}

	if (prctl(PR_SET_NO_NEW_PRIVS, 1, 0, 0, 0)) {
		perror("prctl(NO_NEW_PRIVS)");
		goto failed;
	}

	if (prctl(PR_SET_SECCOMP, SECCOMP_MODE_IDS, &idstable)) {
		perror("prctl(SECCOMP)");
		goto failed;
	}
	return 0;

failed:
	if (errno == EINVAL)
		fprintf(stderr, "SECCOMP_FILTER is not available. :(\n");
	return 1;
}

#define BILLION 1000000000L

struct timespec timediff(struct timespec start, struct timespec end)
{
    struct timespec temp;
    if ((end.tv_nsec-start.tv_nsec)<0) {
        temp.tv_sec = end.tv_sec-start.tv_sec-1;
        temp.tv_nsec = 1000000000+end.tv_nsec-start.tv_nsec;
    } else {
        temp.tv_sec = end.tv_sec-start.tv_sec;
        temp.tv_nsec = end.tv_nsec-start.tv_nsec;
    }
    return temp;
}

int main(int argc, char *argv[])
{
	int i;
	struct timespec start, end, total;
	uint64_t diff;

	if (atoi(argv[1]) == 1)
		install_syscall_filter();

	clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &start);
	for (i=0; i<1000000; i++) {
		syscall(atol(argv[2]));
	}
	clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &end);

	total = timediff(start, end);
	diff  = total.tv_sec * BILLION + total.tv_nsec;
	//diff = BILLION * (end.tv_sec - start.tv_sec) + end.tv_nsec - start.tv_nsec;
	printf("%llu", (long long unsigned int) diff);

	return 0;
}

