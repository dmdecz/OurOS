#ifndef DRIVER_SD_H
#define DRIVER_SD_H

#define SECSIZE 512
typedef unsigned long u32;

u32 sd_read_block(unsigned char *buf, unsigned long addr, unsigned long count);
u32 sd_write_block(unsigned char *buf, unsigned long addr, unsigned long count);

#endif  // DRIVER_SD_H