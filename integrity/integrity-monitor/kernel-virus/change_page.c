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
#include <linux/module.h>   /* Needed by all modules */
#include <asm-generic/bitsperlong.h>   /* for KERN_ALERT */
#include <linux/sched.h>    /* for task_struct */
#include <linux/mm_types.h> /* for mm_struct */
#include <linux/highmem.h>  /* for walking the page table */
#include <asm/pgtable.h>    /* for walking the page table */
#include <linux/vmalloc.h>  /* for vmap*/

static int pid;
module_param(pid, int, S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);
MODULE_PARM_DESC(pid, "PID of the process to monitor");

static struct page *walk_page_table(struct mm_struct *mm, unsigned long addr)
{
	pgd_t *pgd;
	pte_t *ptep, pte;
	pud_t *pud;
	pmd_t *pmd;
	struct page *page = NULL;

	pgd = pgd_offset(mm, addr);
	if (pgd_none(*pgd) || pgd_bad(*pgd))
		goto out;

	pud = pud_offset(pgd, addr);
	if (pud_none(*pud) || pud_bad(*pud))
		goto out;

	pmd = pmd_offset(pud, addr);
	if (pmd_none(*pmd) || pmd_bad(*pmd))
		goto out;

	ptep = pte_offset_map(pmd, addr);
	if (!ptep)
		goto out;
	pte = *ptep;

	pte=pte_mkwrite(pte);

	page = pte_page(pte);

	pte_unmap(ptep);
out:
	return page;
}

int init_module(void)
{
	struct task_struct *task;
	struct mm_struct *mm;
	struct page *page;
	struct page *pages[1];
	u8 *kernel_va;
	unsigned long addr;
	unsigned long _pfn;
	task = pid_task(find_vpid(pid), PIDTYPE_PID);

	mm = get_task_mm(task);
	addr = mm->mmap->vm_start;
	page = walk_page_table(mm, addr);
	BUG_ON(page == NULL );
	pages[0] = page;
	kernel_va = (u8 *)vmap(pages, 1, VM_MAP, PAGE_KERNEL);

	*(kernel_va+16*30+4) = 144;	//change the instruction into 'NOP' , 144=90(hex)=nop

	_pfn=page_to_pfn(walk_page_table(mm, mm->mmap->vm_next->vm_start));
	flush_cache_page(mm->mmap,addr,_pfn);	//flush the memory.

	return 0;
}

void cleanup_module(void)
{
	printk("Change instruction completed!");
}

MODULE_LICENSE("GPL");
