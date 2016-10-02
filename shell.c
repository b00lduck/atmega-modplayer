#include <string.h>
#include <stdlib.h>
#include "shell.h"
#include "shell_functions.h"
#include "sram.h"
#include "uart.h"
#include "sdreader/sdhc_lib.h"
#include "modplayer/modplayer.h"


extern uint8_t playing_chan;

void run_shell() {

    /* provide a simple shell */
    char buffer[24];
    while(1) {
        /* print prompt */
        uart_puts_p(PSTR("# "));        

        /* read command */
        char* command = buffer;
        if(uart_read_line(command, sizeof(buffer)) < 1)
            continue;

        /* execute command */
        if(strcmp_P(command, PSTR("init")) == 0) {
            sdhc_init();
        } else if(strcmp_P(command, PSTR("ls")) == 0) {

			ls();

        } else if(strncmp_P(command, PSTR("load "), 5) == 0) {

			command += 5;
			if(command[0] == '\0') continue;

			if (load_file_to_ram(command) == 0) {

				modplayer_init();
				print_song_info();
			
			} else {
			

			}

        } else if(strncmp_P(command, PSTR("play"), 4) == 0) {

			modplayer_play();	

        } else if(strncmp_P(command, PSTR("stop"), 4) == 0) {

			modplayer_stop();	

        } else if(strncmp_P(command, PSTR("ramt"), 4) == 0) {

			ramtest();	

        } else if(strcmp_P(command, PSTR("disk")) == 0) {
            if(!print_disk_info(fs))
                uart_puts_p(PSTR("error reading disk info\n"));
        } else if(strcmp_P(command, PSTR("dump")) == 0) {
			
			dump_memory();

        } else if(strncmp_P(command, PSTR("set "), 4) == 0) {
			command += 4;

			uint32_t addr = atoi(command);
			
			sram_write(addr,0xff);

        } else {
            uart_puts_p(PSTR("unknown command: "));
            uart_puts(command);
            uart_putc('\n');
        }
    }
}

