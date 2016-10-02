#include "stdinc.h"

#include "uart.h"
#include "spi.h"
#include "tools.h"
#include "sram.h"
#include "sdreader/sdhc_lib.h"
#include "shell.h"
#include "modplayer/modplayer_paula_emu.h"


int main(void) {

    /* we will just use ordinary idle mode */
    //set_sleep_mode(SLEEP_MODE_IDLE);

    /* setup uart */
    uart_init();

	//uart_puts("usart inited\n");

	sram_init();

	//uart_puts("sram inited\n");
	
	while (sdhc_init() != 0);

	//uart_puts("sdhc inited\n");

	//print_disk_info(fs);

	sram_clear();

	paula_init();

	run_shell(); 

}




