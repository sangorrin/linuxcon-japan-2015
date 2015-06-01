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
#include <linux/kernel.h>   /* for KERN_ALERT */
#include <linux/sched.h>    /* for task_struct */
#include <linux/mm_types.h> /* for mm_struct */
#include <linux/highmem.h>  /* for walking the page table */
#include <asm/pgtable.h>    /* for walking the page table */
#include <linux/vmalloc.h>  /* for vmap */
#include <linux/kthread.h>

static struct task_struct *task_check_xor_value;
static int pid;
module_param(pid, int, 0644);

struct page *walk_page_table( struct mm_struct *mm , unsigned long addr )
{
	pgd_t *pgd;
	pud_t *pud;
	pmd_t *pmd;
	pte_t *ptep;
	pte_t pte;
	struct page *page=NULL;

	pgd=pgd_offset(mm,addr);
	if(pgd_bad(*pgd) || pgd_none(*pgd))
		goto out;

	pud=pud_offset(pgd,addr);
	if(pud_bad(*pud) || pud_none(*pud))
		goto out;

	pmd=pmd_offset(pud,addr);
	if(pmd_bad(*pmd) || pmd_none(*pmd))
		goto out;

	ptep=pte_offset_map(pmd,addr);
	if(!ptep)
		goto out;
	pte = *ptep;

	page = pte_page(pte);

	pte_unmap(ptep);
out:
		return page;
}

void thread_check_xor_value(void)
{
	struct task_struct *task;
	struct mm_struct *mm;
	unsigned long addr;
	struct page *page, *pages[1];
	int i;
	int temp1, temp2, temp3;
	unsigned long  xor_value=0;
	unsigned long test_xor_value=0;
	u8 *kernel_va;
	int timecount=0;

	task = pid_task(find_vpid(pid), PIDTYPE_PID);
	mm=task->mm;
	addr = mm->mmap->vm_start;
	page=walk_page_table(mm,addr);
	pages[0]=page;
	kernel_va=(u8*)vmap(pages , 1 , VM_MAP, PAGE_KERNEL);

/*****************************************
when we use '$ objdump' command, 
the output is like this: aefd73d3.
here the output is like this: d373fdae , 
so we need to change the output order
****************************************/
	for(i=16*0; i<16*256; i=i+4)
	{
		temp1=(unsigned long)((int)(*(kernel_va+3))|(((int)(*(kernel_va+2))<<8)));
		temp2=((unsigned long)temp1)|(((int)(*(kernel_va+1))<<16));
		temp3=((unsigned long)temp2)|(((int)(*(kernel_va))<<24));
		xor_value = xor_value ^ temp3;
		kernel_va = kernel_va+4;
	}

	addr=mm->mmap->vm_start;
	do
	{
		test_xor_value=0;
		page=walk_page_table(mm,addr);
		pages[0]=page;
		kernel_va=(u8*)vmap(pages , 1 , VM_MAP, PAGE_KERNEL);

		for(i=16*0; i<16*256; i=i+4)
		{
			temp1=(unsigned long)((int)(*(kernel_va+3))|(((int)(*(kernel_va+2))<<8)));
			temp2=((unsigned long)temp1)|(((int)(*(kernel_va+1))<<16));
			temp3=((unsigned long)temp2)|(((int)(*(kernel_va))<<24));
			test_xor_value = test_xor_value ^ temp3;
			kernel_va = kernel_va+4;

		}

		//check the original xor value and the current one
		if(xor_value != test_xor_value)
		{
			printk("xor_value are not same!	original xor_value:%lx	current xor_value:%lx\n",  xor_value , test_xor_value);
			break;
		}
		else
		{
			printk("[%d]	xor_value are same	original xor_value:%lx	current xor_value:%lx\n", timecount, xor_value , test_xor_value);
		}

		timecount++;
		msleep(3000);
	}
	while(!kthread_should_stop());
}

int init_module(void)
{

	int err;
	printk("\n\n\n");
	
	//create a kthread to monitor the page periodically
	task_check_xor_value = kthread_create(thread_check_xor_value, 0, "check_xor_value");
	if(IS_ERR(task_check_xor_value))
	{ 
		printk("Unable to start kernel thread.\n");
		err = PTR_ERR(task_check_xor_value);
		task_check_xor_value = NULL;
		return err;
	}
    	
	wake_up_process(task_check_xor_value);
 
	return 0;
}

void cleanup_module(void)
{
	if(task_check_xor_value)
	{
		kthread_stop(task_check_xor_value);
		task_check_xor_value = NULL;
	}
}

MODULE_LICENSE("GPL");
