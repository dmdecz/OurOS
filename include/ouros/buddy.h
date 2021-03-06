#ifndef OUROS_BUDDY_H
#define OUROS_BUDDY_H

#include <ouros/mm.h>
#include <linux/list.h>

enum buddy_page_info {
	BUDDY_FREE, BUDDY_SLAB, BUDDY_ALLOCED, BUDDY_RESERVED
};

struct page {
	struct list_head list;
	uint used_info;
	void *virtual;
	// Buddy
	int bplevel;
	// Slab
	void *cachep;
};

struct buddy_free_area {
	struct list_head freelist;
	int size;
};

#define MAX_BUDDY_ORDER 4

struct buddy_zone {
	uint start_pfn;			// start page frame number
	uint page_num;			// total page frames
	struct page *pages;		// page struct of the start page frame
	struct buddy_free_area free_area[MAX_BUDDY_ORDER + 1];	// 0, 1, ..., MAX_BUDDY_ORDER
};

extern struct page *all_pages;
extern struct buddy_zone buddy_mm;

#define get_page_by_pgn(pgn) ((all_pages) + (pgn))
#define get_page_by_paddr(page_paddr) ( get_page_by_pgn( ((uint)(page_paddr)) >> PAGE_SHIFT ) )
#define get_pgn(page) ((uint)((page) - (all_pages)))
#define get_page_paddr(page) ((void *)(get_pgn(page) << PAGE_SHIFT))

void init_buddy();

void free_pages(void *addr);
void *alloc_pages(uint size);
void *alloc_one_page();

void print_buddy_info();

void test_buddy(int level);

#endif // OUROS_BUDDY_H
