#include "nemu.h"
#include "cpu/reg.h"
#include "monitor/breakpoint.h"
#include "memory/memory.h"

#include <stdlib.h>

#define NR_BP 32

static BP bp_list[NR_BP];
static BP *head, *free_;

void init_bp_list()
{
	int i;
	for (i = 0; i < NR_BP; i++)
	{
		bp_list[i].NO = i;
		bp_list[i].in_use = false;
		bp_list[i].next = &bp_list[i + 1];
	}
	bp_list[NR_BP - 1].next = NULL;

	head = NULL;
	free_ = bp_list;
}

static BP *new_BP()
{
	assert(free_ != NULL);
	BP *p = free_;
	free_ = free_->next;
	assert(p->in_use == false);
	p->in_use = true;
	return p;
}

static void free_BP(BP *p)
{
	assert(p >= bp_list && p < bp_list + NR_BP);
	assert(p->in_use == true);
	if (p->type == WATCHPOINT)
		free(p->expr);
	else 
		vaddr_write(p->addr, SREG_CS, 1, p->ori_byte);
	p->in_use = false;
	p->next = free_;
	free_ = p;
}

int set_breakpoint(vaddr_t addr)
{
	BP *p = new_BP();
	p->type = BREAKPOINT;
	p->addr = addr;

	p->ori_byte = instr_fetch(p->addr, 1);
	vaddr_write(p->addr, SREG_CS, 1, INT3_CODE);
	p->enable = true;

	p->next = head;
	head = p;
	return p->NO;
}

int set_watchpoint(char *e)
{
	uint32_t val;
	bool success;
	val = expr(e, &success);
	if (!success)
		return -1;

	BP *p = new_BP();
	p->type = WATCHPOINT;
	p->expr = strdup(e);
	p->old_val = val;
	p->enable = true;

	p->next = head;
	head = p;

	return p->NO;
}

bool delete_breakpoint(int NO)
{
	BP *p, *prev = NULL;
	for (p = head; p != NULL; prev = p, p = p->next)
	{
		if (p->NO == NO)
		{
			break;
		}
	}

	if (p == NULL)
	{
		return false;
	}
	if (prev == NULL)
	{
		head = p->next;
	}
	else
	{
		prev->next = p->next;
	}

	free_BP(p);
	return true;
}

void delete_all_breakpoint_recursively(BP *p)
{
	if (p == NULL)
		return;
	if (p->next != NULL)
		delete_all_breakpoint_recursively(p->next);

	free_BP(p);
}

void delete_all_breakpoint()
{
	delete_all_breakpoint_recursively(head);
	head = NULL;
}

void list_breakpoint()
{
	if (head == NULL)
	{
		printf("No breakpoints\n");
		return;
	}

	printf("%8s\t%6s\t%8s\t%8s\n", "NO", "Type", "Address", "Enable");
	BP *p;
	for (p = head; p != NULL; p = p->next)
	{
		if (p->type == BREAKPOINT)
		{
			printf("%8d\t%6s\t%#08x\t%8s\t%x\n", p->NO, "B", p->addr, p->enable ? "Yes" : "No", p->ori_byte);
		}
		else
		{
			printf("%8d\t%6s\t%s\t%#08x\n", p->NO, "W", p->expr, p->old_val);
		}
	}
}

BP *find_breakpoint(vaddr_t addr)
{
    // printf("searching for %.8x \n", addr);
	BP *p;
	for (p = head; p != NULL; p = p->next)
	{
		assert(p->in_use == true);
		if (p->type == BREAKPOINT && p->addr == addr)
		{
			break;
		}
	}

	return p;
}

void resume_breakpoints()
{
	BP *p;
	for (p = head; p != NULL; p = p->next)
	{
		assert(p->in_use == true);
		if (p->type == BREAKPOINT)
		{
			vaddr_write(p->addr, SREG_CS, 1, INT3_CODE);
			p->enable = true;
		}
	}
}

void fetch_breakpoints()
{
	BP *p;
	for (p = head; p != NULL; p = p->next)
	{
		assert(p->in_use == true);
		if (p->type == BREAKPOINT)
		{
		    p->ori_byte = instr_fetch(p->addr, 1);
			vaddr_write(p->addr, SREG_CS, 1, INT3_CODE);
			p->enable = true;
		}
	}
}

void mask_breakpoints()
{
	BP *p;
	for (p = head; p != NULL; p = p->next)
	{
		assert(p->in_use == true);
		if (p->type == BREAKPOINT)
		{
			vaddr_write(p->addr, SREG_CS, 1, p->ori_byte);
		}
	}
}

BP *scan_watchpoint()
{
	BP *p;
	for (p = head; p != NULL; p = p->next)
	{
		assert(p->in_use == true);
		if (p->type == WATCHPOINT)
		{
			bool success;
			p->new_val = expr(p->expr, &success);
			if (p->old_val != p->new_val)
			{
				return p;
			}
		}
	}

	return NULL;
}
