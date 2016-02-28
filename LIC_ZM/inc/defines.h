#ifndef __DEFINES_H
#define __DEFINES_H

/***********************
 *         CPU         *
 ***********************/

/* CPU frequency */
#define F_CPU 14745600UL

// cpu cycles per microsecond
#define CYCLES_PER_US ((F_CPU+500000)/1000000)


/***********************
 *    Communication    *
 ***********************/

/* UART baud rate */
#define UART_BAUD  115200
#define UART_BAUD2  38400

/***********************
 *       Camera        *
 ***********************/

#define check_timer_SET      20   // ×284.4ms = 5.69 s

/* delay before send data from SPIbuf */
#define SPIbuf_TimeOut_SET 10 // ×0.1389ms = 1.4 ms


/***********************
 *    Pins assigment   *
 ***********************/

#define TXD_EN PD2

#endif
