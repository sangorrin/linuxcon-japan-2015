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
//TEST: Seccomp BPF, test that calling a not-allowed (negative) syscall gives error
#define _GNU_SOURCE 1

#include <stdio.h>
#include <stddef.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <linux/audit.h>

#include <sys/prctl.h>
#include <linux/filter.h>
#include <linux/seccomp.h>
#include <asm/unistd.h>
#include <sys/syscall.h>   /* For SYS_xxx definitions */

# define REG_SYSCALL	REG_EAX
# define ARCH_NR	AUDIT_ARCH_I386

#define syscall_nr (offsetof(struct seccomp_data, nr))
#define arch_nr (offsetof(struct seccomp_data, arch))

#define VALIDATE_ARCHITECTURE \
	BPF_STMT(BPF_LD+BPF_W+BPF_ABS, arch_nr), \
	BPF_JUMP(BPF_JMP+BPF_JEQ+BPF_K, ARCH_NR, 1, 0), \
	BPF_STMT(BPF_RET+BPF_K, SECCOMP_RET_KILL)

#define EXAMINE_SYSCALL \
	BPF_STMT(BPF_LD+BPF_W+BPF_ABS, syscall_nr)

#define ALLOW_SYSCALL(name) \
	BPF_JUMP(BPF_JMP+BPF_JEQ+BPF_K, __NR_##name, 0, 1), \
	BPF_STMT(BPF_RET+BPF_K, SECCOMP_RET_ALLOW)

#define KILL_PROCESS \
	BPF_STMT(BPF_RET+BPF_K, SECCOMP_RET_KILL)

static int install_syscall_filter(void)
{
	struct sock_filter filter[] = {
		VALIDATE_ARCHITECTURE,
		EXAMINE_SYSCALL,
		ALLOW_SYSCALL(rt_sigreturn),
		ALLOW_SYSCALL(exit_group),
		ALLOW_SYSCALL(exit),
		ALLOW_SYSCALL(read),
		ALLOW_SYSCALL(write),
		ALLOW_SYSCALL(close),
		ALLOW_SYSCALL(clone),
		ALLOW_SYSCALL(fstat),
		ALLOW_SYSCALL(fstat64),
		ALLOW_SYSCALL(mmap),
		ALLOW_SYSCALL(mmap2),
		ALLOW_SYSCALL(mprotect),
		ALLOW_SYSCALL(rt_sigprocmask),
		ALLOW_SYSCALL(rt_sigaction),
		ALLOW_SYSCALL(nanosleep),
		ALLOW_SYSCALL(getpid),
		BPF_JUMP(BPF_JMP+BPF_JEQ+BPF_K, -69, 0, 1),
		BPF_STMT(BPF_RET+BPF_K, SECCOMP_RET_TRAP),
		KILL_PROCESS,
	};
	struct sock_fprog prog = {
		.len = (unsigned short)(sizeof(filter)/sizeof(filter[0])),
		.filter = filter,
	};

	if (prctl(PR_SET_NO_NEW_PRIVS, 1, 0, 0, 0)) {
		perror("prctl(NO_NEW_PRIVS)");
		goto failed;
	}

	if (prctl(PR_SET_SECCOMP, SECCOMP_MODE_FILTER, &prog)) {
		perror("prctl(SECCOMP)");
		goto failed;
	}
	return 0;

failed:
	if (errno == EINVAL)
		fprintf(stderr, "SECCOMP_FILTER is not available. :(\n");
	return 1;
}


int main(int argc, char *argv[])
{
	long ret;

	install_syscall_filter();
	printf("Hi there!\n");

	ret = syscall(69);
	printf ("should not arrive here (check dmesg)\n");

	return 0;
}