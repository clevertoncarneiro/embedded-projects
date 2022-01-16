/* Defines boolean and integer data types */
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
/* Defines the base address of the memories and peripherals */
#include "inc/hw_memmap.h"
/* Defines the common types and macros */
#include "inc/hw_types.h"
/* Defines and Macros for GPIO API */
#include "driverlib/gpio.h"
/* Prototypes for the system control driver */
#include "driverlib/sysctl.h"
/* Defines and Macros for UART API */
#include "driverlib/uart.h"

/*definições de macros*/

//para frequencia a 24 MHZ
#define CONSTANTE_DE_TEMPO_24                                 0.00000128205
//para frequencia a 120 MHZ
#define CONSTANTE_DE_TEMPO_120                                0.00000157880

#define CLOCK_24        1
#define CLOCK_120       2

void initUart()
{
  // Enable the UART0 module.
  SysCtlPeripheralEnable(SYSCTL_PERIPH_UART0);
  
  // Wait for the UART0 module to be ready.
  while(!SysCtlPeripheralReady(SYSCTL_PERIPH_UART0)){}
  
  // Initialize the UART. Set the baud rate, number of data bits, turn off
  // parity, number of stop bits, and stick mode. The UART is enabled by the
  // function call.
  UARTConfigSetExpClk(UART0_BASE,24000000UL, 9600,(UART_CONFIG_WLEN_8 | UART_CONFIG_STOP_ONE |UART_CONFIG_PAR_NONE));
}

void closeUart()
{
  // Disable the UART.
  UARTDisable(UART0_BASE);
}

void sendUart(char msg[])
{
  int n_char = strlen(msg);
  
  for(int i = 0; i < n_char; i++)
  {
    while(!UARTCharPutNonBlocking(UART0_BASE, msg[i]));
  }
}


int main(void)
{ 
  // Enable the GPIOA peripheral
  SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOA);
  
  // Wait for the GPIOA module to be ready.
  while(!SysCtlPeripheralReady(SYSCTL_PERIPH_GPIOA)){}
  
  // Initialize the GPIO pin configuration.
  // Set pin 7 as input, SW controlled.
  GPIOPinTypeGPIOInput(GPIO_PORTA_BASE,GPIO_PIN_7);
  
  initUart();
  
  // inicialização das variáveis do fluxograma
  int32_t i32Val = 0;
  static float alta = 0;
  static float baixa = 0;
  int iteracoes = 0;
  int32_t aux = 0;
  static float periodo = 0;
  static float frequencia = 0;
  static float ciclo_de_trabalho = 0;
  
  //
  int iClock = CLOCK_24;
  
  //strings a enviar
  static char sCiclo[] = "ciclo de trabalho";
  static char sPeriodo[] = "periodo";
  static char sFrequencia[] = "frequencia";
  static char sValor[10];
  static char sPulaLinha[] = "\n";
  
  i32Val = GPIOPinRead(GPIO_PORTA_BASE,(GPIO_PIN_7));
  aux = i32Val;
  
  while(1)
  {
    //zera tudo
    alta = 0;
    baixa = 0;
    iteracoes = 0;
    
    //sincroniza o sinal com a contagem
    while(aux == i32Val)
    {
      i32Val = GPIOPinRead(GPIO_PORTA_BASE,(GPIO_PIN_7));
    }
    
    //pega as amostras por determinada quantidade de amostras
    while(iteracoes < 200)
    {
      //aumenta os contadores dependendo da entrada
      //obs: a leitura e feita depois, por causa da leitura
      //feita acima na sincronizacao
      if(i32Val > 0)
        alta++;
      else
        baixa++;
      
      // Read some pins.
      i32Val = GPIOPinRead(GPIO_PORTA_BASE,(GPIO_PIN_7));
      
      //verifica se o sinal alterou, e entao conta como 1 iteracao a mais
      if(aux == i32Val)
      {
        aux = aux ^ GPIO_PIN_7;
        iteracoes++;
      }
    }//while
    
    
    if(iClock == CLOCK_24)
    {
      alta = alta/100 * CONSTANTE_DE_TEMPO_24;
      baixa = baixa/100 * CONSTANTE_DE_TEMPO_24;
      periodo = alta + baixa;
      frequencia = (1/periodo);
      ciclo_de_trabalho = (alta/periodo)*100;
    
    }// if 40
    
    
    else if(iClock == CLOCK_120)
    {
      alta = alta/100 * CONSTANTE_DE_TEMPO_120;
      baixa = baixa/100 * CONSTANTE_DE_TEMPO_120;
      periodo = alta + baixa;
      frequencia = (1/periodo);
      ciclo_de_trabalho = (alta/periodo)*100;
    
    }// if 120
    
    //imprime ciclo de trabalho
    sendUart(sCiclo);
    sendUart(sPulaLinha);
    sprintf (sValor, "%.6f", ciclo_de_trabalho);
    sendUart(sValor);
    sendUart(sPulaLinha);
    
    //imprime periodo
    sendUart(sPeriodo);
    sendUart(sPulaLinha);
    sprintf (sValor, "%.6f", periodo);
    sendUart(sValor);
    sendUart(sPulaLinha);
    
    //imprime frequencia
    sendUart(sFrequencia);
    sendUart(sPulaLinha);
    sprintf (sValor, "%.6f", frequencia);
    sendUart(sValor);
    sendUart(sPulaLinha);
    sendUart(sPulaLinha);
    sendUart(sPulaLinha);
   } // while infinito
  
  //closeUart();
  
}//main