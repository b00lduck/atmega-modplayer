#include <string.h>
#include <avr/interrupt.h>
#include <stdio.h>
#include "shell.h"
#include "sram.h"
#include "uart.h"
#include "modplayer/modplayer.h"


uint8_t load_file_to_ram(char* filename) {

    /* search file in current directory and open it */
    struct fat_file_struct* fd = open_file_in_dir(fs, dd, filename);
    if(!fd) {
        uart_puts_p(PSTR("error opening "));
        uart_puts(filename);
        uart_putc('\n');
        return 1;
    }

    /* print file contents */
	#define READBUFFER_SIZE 64

    uint8_t buffer[READBUFFER_SIZE];
    uint32_t offset = 0;
    while(fat_read_file(fd, buffer, sizeof(buffer)) > 0) {             
        for(uint16_t i=0; i < READBUFFER_SIZE; ++i) {
		 	sram_write(offset + i,buffer[i]); 
        }
        offset += READBUFFER_SIZE;
    }

	uart_puts_p(PSTR("0x"));			
	uart_putdw_hex(offset);
	uart_puts_p(PSTR(" bytes loaded.\n"));

    fat_close_file(fd);

	return 0;

}

uint8_t print_disk_info(const struct fat_fs_struct* fs) {
/*
    if(!fs) return 0;

    struct sd_raw_info disk_info;
    if(!sd_raw_get_info(&disk_info)) return 0;

    uart_puts_p(PSTR("manuf:  0x")); uart_putc_hex(disk_info.manufacturer); uart_putc('\n');
    uart_puts_p(PSTR("oem:    ")); uart_puts((char*) disk_info.oem); uart_putc('\n');
    uart_puts_p(PSTR("prod:   ")); uart_puts((char*) disk_info.product); uart_putc('\n');
    uart_puts_p(PSTR("rev:    ")); uart_putc_hex(disk_info.revision); uart_putc('\n');
    uart_puts_p(PSTR("serial: 0x")); uart_putdw_hex(disk_info.serial); uart_putc('\n');
    uart_puts_p(PSTR("date:   ")); uart_putw_dec(disk_info.manufacturing_month); uart_putc('/');
                                   uart_putw_dec(disk_info.manufacturing_year); uart_putc('\n');
    uart_puts_p(PSTR("size:   ")); uart_putdw_dec(disk_info.capacity / 1024 / 1024); uart_puts_p(PSTR("MB\n"));
    uart_puts_p(PSTR("copy:   ")); uart_putw_dec(disk_info.flag_copy); uart_putc('\n');
    uart_puts_p(PSTR("wr.pr.: ")); uart_putw_dec(disk_info.flag_write_protect_temp); uart_putc('/');
                                   uart_putw_dec(disk_info.flag_write_protect); uart_putc('\n');
    uart_puts_p(PSTR("format: ")); uart_putw_dec(disk_info.format); uart_putc('\n');
    //uart_puts_p(PSTR("free:   ")); uart_putdw_dec(fat_get_fs_free(fs)); uart_putc('/');
    //                               uart_putdw_dec(fat_get_fs_size(fs)); uart_putc('\n');
	*/
	
    return 1;
}

void print_song_info() {

    uart_puts_p(PSTR("song name:   ")); uart_puts(songinfo.name); uart_putc('\n');
	uart_puts_p(PSTR("patterns:    ")); uart_putw_dec(songinfo.numpatterns); uart_putc('\n');
	uart_puts_p(PSTR("song length: ")); uart_putw_dec(songinfo.numsongpos); uart_putc('\n');
	uart_puts_p(PSTR("max samples: ")); uart_putw_dec(songinfo.numsamples); uart_putc('\n');
	uart_puts_p(PSTR("samples offset:    0x")); uart_putdw_hex(songinfo.offset_samples); uart_putc('\n');
	uart_puts_p(PSTR("songheader offset: 0x")); uart_putdw_hex(songinfo.offset_songheader); uart_putc('\n');
	uart_puts_p(PSTR("patterns offset:   0x")); uart_putdw_hex(songinfo.offset_patterns); uart_putc('\n');

	char buf[23];	

	for(uint8_t i=0; i<songinfo.numsamples; i++) {
		get_sample_name(buf, i);
		uart_puts_p(PSTR("sample ")); uart_putw_dec(i); uart_puts_p(PSTR(": "));  uart_puts(buf); uart_putc('\n');
	}

}

void dump_memory() {

	unsigned int i;

	for(i=0;i<MEMORY_SIZE;i++) {

		char buffer[32];
		char buffer2[64];

		sprintf(buffer,"%02x",sram_read(i));

		if ((i % 16) == 0) {
			sprintf(buffer2,"%03x",i);						
			uart_puts_p(PSTR("0x"));	
			uart_puts(buffer2);		
			uart_puts_p(PSTR("  "));		
		}

		uart_puts(buffer);		

		if ((i+1) % 16) {
			uart_puts_p(PSTR(" "));
		} else {
			uart_puts_p(PSTR("\n"));
		}
	
	}	

}

/**
 * print directory listing
 */
void ls() {

    uart_puts_p(PSTR("contents of SDHC card:\n")); 

    struct fat_dir_entry_struct dir_entry;
    while(fat_read_dir(dd, &dir_entry)) {
        uint8_t spaces = sizeof(dir_entry.long_name) - strlen(dir_entry.long_name) + 4;

        uart_puts(dir_entry.long_name);
        uart_putc(dir_entry.attributes & FAT_ATTRIB_DIR ? '/' : ' ');
        while(spaces--) uart_putc(' ');
        uart_putdw_dec(dir_entry.file_size);
        uart_putc('\n');
    }
}

void ramtest() {
	
	uint32_t i;
	uint32_t fails = 0;
	uint16_t local_fails = 0;
	uint8_t v = 0;

	for(i=0;i<MEMORY_SIZE;i++) {

		v = 0xff;

		sram_write(i,v);	

		if (sram_read(i) != v) {			
			fails++;
			local_fails++;
		} else {
			
			v = 0x00;
			sram_write(i,v);	

			if (sram_read(i) != v) {			
				fails++;
				local_fails++;
			}
		}

		if ((i % 0x1000) == 0) {
			uart_putdw_hex(i);
			uart_puts_p(PSTR("-"));
			uart_putdw_hex(i + 0x0999);
			uart_puts_p(PSTR(" "));
			uart_putdw_dec(local_fails);
			uart_puts_p(PSTR(" errors\n"));
			local_fails = 0;
		}			 
	
	}

	
	uart_putdw_dec(fails);
	uart_puts_p(PSTR(" memory cells damaged\n")); 
	

}
