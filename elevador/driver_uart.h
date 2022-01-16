#ifndef __DRIVER_UART_H__
#define __DRIVER_UART_H__

#include <stdint.h>

extern void UARTInit(void);
extern void sendUart(char msg[]);
extern void UART0_Handler(void);
extern char receiveUart(void);
extern void sendSimulator(char msg[]);

#endif // __DRIVERLEDS_H__
