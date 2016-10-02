#include <string.h>
#include <avr/pgmspace.h>

#include "sdhc_fat.h"
#include "sdhc_fat_config.h"
#include "sdhc_partition.h"
#include "sdhc_sd_raw.h"

#include "../sram.h"

#include "sdhc_sd_raw_config.h"
#include "../uart.h"

#define DEBUG 1

struct partition_struct* partition;
struct fat_fs_struct* fs;
struct fat_dir_struct* dd;

uint8_t sdhc_init() {

	uart_puts_p(PSTR("SDHC init\n"));

   /* setup sd card slot */
    if(!sd_raw_init()) {
		#if DEBUG
			uart_puts_p(PSTR("MMC/SD initialization failed\n"));
		#endif
        return 1;
    }

    /* open first partition */
    partition = partition_open(sd_raw_read,sd_raw_read_interval,0,0,0);

    if(!partition) {
        /* If the partition did not open, assume the storage device
         * is a "superfloppy", i.e. has no MBR.
         */
        partition = partition_open(sd_raw_read,sd_raw_read_interval,0,0,-1);
        if(!partition) {
			#if DEBUG
                uart_puts_p(PSTR("opening partition failed\n"));
			#endif
            return 2;
        }
    }

    /* open file system */
    fs = fat_open(partition);
    if(!fs) {
		#if DEBUG
        	uart_puts_p(PSTR("opening filesystem failed\n"));
		#endif
        return 3;
    }

    /* open root directory */
    struct fat_dir_entry_struct directory;
    fat_get_dir_entry_of_path(fs, "/", &directory);

    dd = fat_open_dir(fs, &directory);
    if(!dd) {
		#if DEBUG
        	uart_puts_p(PSTR("opening root directory failed\n"));
		#endif
        return 4;
    }
    
	return 0;
}

uint8_t find_file_in_dir(struct fat_fs_struct* fs, struct fat_dir_struct* dd, const char* name, struct fat_dir_entry_struct* dir_entry) {
    while(fat_read_dir(dd, dir_entry)) {
        if(strcmp(dir_entry->long_name, name) == 0) {
            fat_reset_dir(dd);
            return 1;
        }
    }

    return 0;
}

struct fat_file_struct* open_file_in_dir(struct fat_fs_struct* fs, struct fat_dir_struct* dd, const char* name) {
    struct fat_dir_entry_struct file_entry;
    if(!find_file_in_dir(fs, dd, name, &file_entry))
        return 0;

    return fat_open_file(fs, &file_entry);
}




