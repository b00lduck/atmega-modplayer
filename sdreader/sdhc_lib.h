#include <avr/io.h>
#include "sdhc_fat.h"
#include "sdhc_sd_raw.h"

uint8_t sdhc_init();
uint8_t sdhc_shell();

extern struct partition_struct* partition;
extern struct fat_fs_struct* fs;
extern struct fat_dir_struct* dd;

uint8_t find_file_in_dir(
		struct fat_fs_struct* fs,
		struct fat_dir_struct* dd,
		const char* name,
		struct fat_dir_entry_struct* dir_entry);

struct fat_file_struct* open_file_in_dir(
		struct fat_fs_struct* fs,
		struct fat_dir_struct* dd,
		const char* name); 
