#include "sdreader/sdhc_lib.h"

uint8_t print_disk_info(const struct fat_fs_struct* fs);
void print_song_info();
void dump_memory();
void ls();
void ramtest();
uint8_t load_file_to_ram(char* filename);
