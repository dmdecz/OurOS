#ifndef OUROS_BOOTMM_H
#define OUROS_BOOTMM_H

#include <arch.h>
#include <ouros/type.h>

#define MAX_MMINFO_NUM	16
#define PAGE_SIZE		4096
#define PAGE_SHIFT		12
#define PAGE_FREE		0x00
#define PAGE_USED		0xFF

#define KERNEL_PAGE_NUM	(MACHINE_MMSIZE >> PAGE_SHIFT)

enum bootmm_info_type {
	MMINFO_TYPE_KERNEL
};

struct bootmm_info_struct {
	uint addr_start;
	uint length;
	uint type;
};

struct bootmm_sys_struct {
	uint phymm_size;
	uint max_pfn;
	uchar page_map[KERNEL_PAGE_NUM];
	uint last_alloc_end;
	uint info_cnt;
	struct bootmm_info_struct info[MAX_MMINFO_NUM];
};

typedef struct bootmm_info_struct bootmm_info;
typedef struct bootmm_sys_struct bootmm_sys;

void init_bootmm();

void* bootmm_alloc_page(bootmm_sys *mm, uint size, uint type, uint addr_align);
void bootmm_free_page(bootmm_sys *mm, void *addrr, uint size);

#endif // OUROS_BOOTMM_H
