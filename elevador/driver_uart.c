#include <stdbool.h>
#include <stdint.h>
#include <string.h>

#include "inc/hw_memmap.h"
#include "inc/hw_ints.h"

#include "driverlib/sysctl.h"
#include "driverlib/gpio.h"
#include "driverlib/uart.h"
#include "driverlib/pin_map.h"
#include "driverlib/interrupt.h"

#include "utils/uartstdio.h"

#include "driver_uart.h"

#define __SYSTEM_CLOCK          (120000000UL)
//#define PART_TM4C1292NCPDT      1
//#define GPIO_PA0_U0RX           0x00000001
//#define GPIO_PA1_U0TX           0x00000401

void UARTInit(void){
  // Enable the GPIO Peripheral used by the UART.
  SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOA);
  while(!SysCtlPeripheralReady(SYSCTL_PERIPH_GPIOA));

  // Enable UART0
  SysCtlPeripheralEnable(SYSCTL_PERIPH_UART0);
  while(!SysCtlPeripheralReady(SYSCTL_PERIPH_UART0));

  // Configure GPIO Pins for UART mode.
  GPIOPinConfigure(GPIO_PA0_U0RX);
  GPIOPinConfigure(GPIO_PA1_U0TX);
  GPIOPinTypeUART(GPIO_PORTA_BASE, GPIO_PIN_0 | GPIO_PIN_1);
  
  // Ativa interrupção para recepção
  // IntRegister(INT_UART0, UART0_Handler);        // Aponta a rotina que ira tratar
  // IntEnable(INT_UART0);
  //UARTIntEnable(UART0_BASE, UART_INT_RX);       // Ativa interrupcao para recepcao
  
  //UARTFIFOLevelSet(UART0_BASE, UART_FIFO_TX1_8, UART_FIFO_TX1_8);
  //UARTFIFODisable(UART0_BASE);
  
  //IntMasterEnable();
  
  // Initialize the UART for console I/O.
  UARTStdioConfig(0, 115200, __SYSTEM_CLOCK);
  return;
} // UARTInit

void sendUart(char msg[])
{
      int n_char = strlen(msg);
      
      for(int i = 0; i < n_char; i++)
      {
        UARTCharPut(UART0_BASE, msg[i]);
      }
}

void sendSimulator(char msg[])
{
  sendUart(msg);        // conteudo da msg
  sendUart((char*)0xD);        // sinaliza o termino da msg
}

char received;
char receiveUart()
{
   while(!UARTCharsAvail(UART0_BASE))
   {

   }

   received = UARTCharGet(UART0_BASE);
   //sendUart(&received);

   //ignorar o pular linha
   if(received == '\n')
   {
      while(!UARTCharsAvail(UART0_BASE))
      {

      }
      received = UARTCharGet(UART0_BASE);
   }
   return received;
}


// char received;
// void UART0_Handler(void){
//   UARTIntClear(UART0_BASE, UART_INT_RX);
  
//   if(UARTCharsAvail(UART0_BASE))
//   {
//     received = UARTCharGet(UART0_BASE);
//     sendUart(&received);
//   }
//   //UARTStdioIntHandler();
// } // UART0_Handler
