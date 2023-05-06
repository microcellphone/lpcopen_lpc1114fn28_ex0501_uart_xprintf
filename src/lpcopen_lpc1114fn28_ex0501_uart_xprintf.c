/*
===============================================================================
 Name        : lpcopen_lpc1114fn28_ex0501_uart_xprintf.c
 Author      : $(author)
 Version     :
 Copyright   : $(copyright)
 Description : main definition
===============================================================================
*/

#if defined (__USE_LPCOPEN)
#if defined(NO_BOARD_LIB)
#include "chip.h"
#else
#include "board.h"
#endif
#endif

#include <cr_section_macros.h>

// TODO: insert other include files here
#include "my_lpc1114fn28.h"
#include "xprintf.h"

// TODO: insert other definitions and declarations here
uint8_t menu1[] = "Hello NXP Semiconductors \n\r";
uint8_t menu2[] = "UART xprintf demo \n\r\t MCU LPC1114FN28 - ARM Cortex-M0 \n\r\t UART - 115200bps \n\r";
uint8_t menu3[] = "UART demo terminated!";

/* Transmit and receive ring buffers */
STATIC RINGBUFF_T txring, rxring;

/* Transmit and receive ring buffer sizes */
#define UART_SRB_SIZE 128	/* Send */
#define UART_RRB_SIZE 32	/* Receive */

/* Transmit and receive buffers */
static uint8_t rxbuff[UART_RRB_SIZE], txbuff[UART_SRB_SIZE];

void print_menu(void)
{
  xprintf((char*)menu1);
  xprintf((char*)menu2);
}

void uart_putc(uint8_t data)
{
	Chip_UART_SendRB(LPC_USART, &txring, &data, 1);
}

uint8_t uart_getc(void)
{
	uint8_t rcv_data;
	int bytes;

	while(1) {
		bytes = Chip_UART_ReadRB(LPC_USART, &rxring, &rcv_data, 1);
		if(bytes == 1) break;
		else if(bytes == 0);
		else while(1);
	}

	return rcv_data;
}

void UART_IRQHandler(void)
{
	/* Want to handle any errors? Do it here. */

	/* Use default ring buffer handler. Override this with your own
	   code if you need more capability. */
	Chip_UART_IRQRBHandler(LPC_USART, &rxring, &txring);
}

int main(void) {

#if defined (__USE_LPCOPEN)
    // Read clock settings and update SystemCoreClock variable
    SystemCoreClockUpdate();
#if !defined(NO_BOARD_LIB)
    // Set up and initialize all required blocks and
    // functions related to the board hardware
    Board_Init();
    // Set the LED to the state of "On"
    Board_LED_Set(0, true);
#endif
#endif

    // TODO: insert code here
    uint32_t idx, len, cnt;
    uint8_t exitflag;
    uint8_t buffer[10];

    IOCON_Config_Request();

//    UART_Config_Request(115200);
	/* Setup UART for 115.2K8N1 */
	Chip_UART_Init(LPC_USART);
	Chip_UART_SetBaud(LPC_USART, 115200);
	Chip_UART_ConfigData(LPC_USART, (UART_LCR_WLEN8 | UART_LCR_SBS_1BIT));
	Chip_UART_SetupFIFOS(LPC_USART, (UART_FCR_FIFO_EN | UART_FCR_TRG_LEV2));
	Chip_UART_TXEnable(LPC_USART);

	/* Before using the ring buffers, initialize them using the ring　buffer init function */
	RingBuffer_Init(&rxring, rxbuff, 1, UART_RRB_SIZE);
	RingBuffer_Init(&txring, txbuff, 1, UART_SRB_SIZE);

	/* Enable receive data and line status interrupt */
	Chip_UART_IntEnable(LPC_USART, (UART_IER_RBRINT | UART_IER_RLSINT));

	/* preemption = 1, sub-priority = 1 */
	NVIC_SetPriority(UART0_IRQn, 1);
	NVIC_EnableIRQ(UART0_IRQn);

    xdev_out(uart_putc);
    xdev_in(uart_getc);
    xprintf ("lpcopen_lpc1114fn28_ex05_uart_xprintf\n") ;

    print_menu();

    exitflag = 0;

    while (exitflag == 0) {
    	for(cnt = 0; cnt < sizeof(buffer); cnt++){
    		buffer[cnt] = (uint8_t)'\0';
    	}
    	xgets((char*)buffer, sizeof(buffer));

    	idx = 0;
    	len = sizeof(buffer);
    	while (idx < len) {
    		if (buffer[idx] == 27) {
    			/* ESC key, set exit flag */
    			xprintf((char*)menu3);
    			exitflag = 1;
    		} else if (buffer[idx] == 'r') {
    			print_menu();
    		} else  {
    			/* Echo it back */
//    			xputc(buffer[idx]);
    		}
    		idx++;
    	}
    }

    // Force the counter to be placed into memory
    volatile static int i = 0 ;
    // Enter an infinite loop, just incrementing a counter
    while(1) {
		  i++ ;
        // "Dummy" NOP to allow source level single
        // stepping of tight while() loop
        __asm volatile ("nop");
    }
    return 0 ;
}
