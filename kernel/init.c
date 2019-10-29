#include <arch.h>
#include <exc.h>
#include <intr.h>
#include <page.h>

#include <driver/ps2.h>
#include <driver/vga.h>
#include <os/time.h>

#include <os/log.h>

#include <os/pc.h>
#include <os/slab.h>
#include <os/bootmm.h>
#include <os/buddy.h>

#include <os/syscall.h>

#include "../usr/ps.h"

void machine_info() {
    int row;
    int col;
    kernel_printf("\n%s\n", "412-UNIX V1.0");
    row = cursor_row;
    col = cursor_col;
    cursor_row = 29;
    kernel_printf("%s", "Created by Dorm 412 Block 32, Yuquan Campus, Zhejiang University.");
    cursor_row = row;
    cursor_col = col;
    kernel_set_cursor();
}

void init_kernel() {
    // init_done = 0;
    kernel_clear_screen(31);
    // Exception
    init_exception();
    // Page table
    init_pgtable();
    // Drivers
    init_vga();
    init_ps2();
    init_time();

    // Memory management
    log(LOG_START, "Memory Modules.");
    init_bootmm();
    log(LOG_OK, "Bootmem.");
    init_buddy();
    log(LOG_OK, "Buddy.");
    init_slab();
    log(LOG_OK, "Slab.");
    log(LOG_END, "Memory Modules.");

    // File system
    log(LOG_START, "File System.");
    init_fs();
    log(LOG_END, "File System.");

    // System call
    log(LOG_START, "System Calls.");
    init_syscall();
    log(LOG_END, "System Calls.");

    // Process control
    log(LOG_START, "Process Control Module.");
    init_pc();
    log(LOG_END, "Process Control Module.");
    // Interrupts
    log(LOG_START, "Enable Interrupts.");
    init_interrupts();
    log(LOG_END, "Enable Interrupts.");
    // Init finished
    machine_info();
    *GPIO_SEG = 0x11223344;

    ps();
}