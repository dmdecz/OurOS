#include "buddy.h"
#include <ouros/utils.h>
#include <driver/vga.h>

struct page *all_pages;
struct buddy_zone buddy_mm;

/**
 * @brief Initialize buddy system.
 */
void init_buddy()
{
	// allocate memory for buddy pages
	uint base_addr = (uint)bootmm_alloc_page(&boot_mm, boot_mm.page_num * sizeof(struct page), MMINFO_TYPE_KERNEL, 1 << PAGE_SHIFT);
	if (!base_addr) {
		
	}
	void *virtual_addr = get_kernel_vaddr(base_addr);
	all_pages = (struct page *)virtual_addr;
	// init buddy pages
	for (uint i = 0; i < boot_mm.page_num; i++) {
		all_pages[i].bplevel = 0;
		all_pages[i].used_info = BUDDY_RESERVED;
		INIT_LIST_HEAD(&(all_pages[i].list));
	}
	
	// init buddy_mm
	uint kernel_reserved_btyes = boot_mm.info[0].start_addr + boot_mm.info[0].length;
	uint kernel_reserved_pages = upper_align(kernel_reserved_btyes, 1 << PAGE_SHIFT) >> PAGE_SHIFT;
	buddy_mm.start_pfn = upper_align(kernel_reserved_pages, 1 << MAX_BUDDY_ORDER);
	buddy_mm.page_num = lower_align(boot_mm.page_num - buddy_mm.start_pfn, 1 << MAX_BUDDY_ORDER);
	buddy_mm.pages = &all_pages[buddy_mm.start_pfn];
	for (int i = 0; i < MAX_BUDDY_ORDER + 1; i++) {
		INIT_LIST_HEAD(&(buddy_mm.free_area[i].freelist));
	}

	// init lock

	// init free buddy pages
	for (uint i = 0; i < buddy_mm.page_num; i++) {
		__free_pages(&buddy_mm, &buddy_mm.pages[i]);
	}
}

/**
 * @brief Check if a page is in a buddy zone.
 * @param mm	buddy zone
 * @param page	page pointer
 */
bool page_is_in_freelist(struct buddy_zone *mm, struct page *page)
{
	bool ret = true;
	if (page_is_in_zone(mm, page)) {
		for (int i = 0; i < MAX_BUDDY_ORDER; i++) {
			if (!list_contain_node(&(page->list), &(mm->free_area[i].freelist))) {
				ret = false;
				break;
			}
		}
	}
	return ret;
}

/**
 * @brief Free a page belong to mm.
 * @param mm	buddy zone
 * @param page	page pointer
 */
void __free_pages(struct buddy_zone *mm, struct page *page)
{
	uint this_pgn = get_pgn(page);
	if (!pgn_is_in_zone(mm, this_pgn)) {
		kernel_printf("Free a page not in this buddy zone!\n");
		return;		
	}
	int bplevel = page->bplevel;
	if (page->used_info == BUDDY_FREE) {
		return;
	}
	while (bplevel < MAX_BUDDY_ORDER) {
		uint buddy_pgn = get_buddy_pgn(this_pgn, bplevel);
		struct page *buddy_page = get_page_by_pgn(buddy_pgn);
		/* Check if buddy page is also free and in this level. */
		if (bplevel != buddy_page->bplevel || buddy_page->used_info != BUDDY_FREE) {
			break;
		}
		list_del_init(&(buddy_page->list));
		this_pgn &= buddy_pgn;
		buddy_page = get_page_by_pgn(this_pgn);
		bplevel++;
	}
	page = get_page_by_pgn(this_pgn);
	page->bplevel = bplevel;
	page->used_info = BUDDY_FREE;
	list_add_tail(&(page->list), &(mm->free_area[bplevel].freelist));
}

/**
 * @brief Free a page from physical address.
 * @param addr	physical address
 */
void free_pages(void *addr)
{
	if (!get_low_bits((uint)addr, 12)) {
		__free_pages(&buddy_mm, get_page_by_paddr(addr));
	} else {
		kernel_printf("Buddy free invalid address!\n");
	}
}

/**
 * @brief Allocate pages in given level with given type from given buddy zone.
 * @param mm		buddy zone
 * @param bplevel	level
 * @param type		type
 * @return allocated struct pages
 */
struct page *__alloc_pages(struct buddy_zone *mm, uint bplevel, int type)
{
	struct page *ret = nullptr;
	if (bplevel > MAX_BUDDY_ORDER) {
		return ret;
	}
	int free_bplevel = -1;
	/* Find lowest level higher than given level with a free node. */
	for (int i = bplevel; i <= MAX_BUDDY_ORDER; i++) {
		if (!list_empty(&(mm->free_area[i].freelist))) {
			free_bplevel = i;
			break;
		}
	}
	if (free_bplevel != -1) {
		/**
		 * If find proper level to allocate, split and send half of the buddy pair
		 * to lower level, until reaching given level.
		 */
		struct page *free_page = list_last_entry(&(mm->free_area[free_bplevel].freelist), struct page, list);
		list_del_init(&(free_page->list));
		uint free_pgn = get_pgn(free_page);
		while (free_bplevel > bplevel) {
			free_bplevel--;
			uint page_not_used_idx = get_buddy_pgn(free_pgn, free_bplevel);
			struct page *page_not_used = get_page_by_pgn(page_not_used_idx);
			page_not_used->bplevel = free_bplevel;
			page_not_used->used_info = BUDDY_FREE;
			list_add_tail(&(page_not_used->list), &(mm->free_area[free_bplevel].freelist));
		}
		/* Set parameters. */
		struct page *page_alloc = get_page_by_pgn(free_pgn);
		page_alloc->bplevel = bplevel;
		page_alloc->used_info = type;
		page_alloc->virtual = nullptr;
		page_alloc->cachep = nullptr;
		ret = page_alloc;
	} else {
		kernel_printf("Buddy no enough memory!\n");
	}
	return ret;
}

/**
 * @brief Allocate pages covered given size.
 * @param size	size
 * @return allocated physical address
 */
void *alloc_pages(uint size)
{
	int bplevel = 0;
	int n = 1;
	int page_num = upper_align(size, 1 << PAGE_SHIFT) >> PAGE_SHIFT;
	while (n < page_num) {
		n *= 2;
		bplevel++;
	}
	struct page *pagep = __alloc_pages(&buddy_mm, bplevel, BUDDY_ALLOCED);
	void *ret = nullptr;
	if (pagep) {
		ret = get_page_paddr(pagep);
	}
	return ret;
}

/**
 * @brief Allocate 1 page.
 * @return allocated physical address
 */
void *alloc_one_page()
{
	return alloc_pages(PAGE_SIZE);
}

void print_buddy_info()
{
	kernel_printf("<=== buddy info ===>\n");

	print_freelist_info();

	kernel_printf("<=== buddy info end ===>\n");
}

void print_freelist_info()
{
	for (int i = 0; i <= MAX_BUDDY_ORDER; i++) {
		kernel_printf("\t%d", i);
	}
	kernel_printf("\n");
	for (int i = 0; i <= MAX_BUDDY_ORDER; i++) {
		struct page *p;
		int cnt = 0;
		list_for_each_entry(p, &(buddy_mm.free_area[i].freelist), list) {
			cnt ++;
		}
		kernel_printf("\t%d", cnt);
	}
	kernel_printf("\n");
}

void test_buddy(int level)
{
	void * addr1 = alloc_pages(4096 << level);
	print_buddy_info();
    free_pages(addr1);
	print_buddy_info();
}
