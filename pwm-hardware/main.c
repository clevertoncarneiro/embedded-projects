/* Defines boolean and integer data types */
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include "inc/hw_memmap.h"
#include "inc/hw_types.h"
#include "inc/hw_timer.h"
#include "inc/hw_ints.h"
#include "inc/hw_gpio.h"
#include "driverlib/timer.h"
#include "driverlib/interrupt.h"
#include "driverlib/gpio.h"
#include "driverlib/sysctl.h"
#include "driverlib/pin_map.h"
#include "driverlib/uart.h"
#include "utils/uartstdio.h"

#include "system_TM4C1294.h" 
/*definições de macros*/
#define __SYSTEM_CLOCK    (120000000UL)

//variáveis globais
unsigned char ucFlag = 0;
unsigned long int uliTempoAtual = 0;
unsigned long int uliTempoAnterior = 0;
unsigned char ucLevel = 0;
signed int iteracoes = 0;
static float alta = 0;
static float baixa = 0;


// inicialização das variáveis do fluxograma
int32_t i32Val = 0;
int32_t aux = 0;
static float periodo = 0;
static float frequencia = 0;
static float ciclo_de_trabalho = 0;


//strings a enviar
static char sCiclo[] = "ciclo de trabalho";
static char sPeriodo[] = "periodo";
static char sFrequencia[] = "frequencia";
static char sValor[10];
static char sPulaLinha[] = "\n";

uint32_t clock_atual;

extern void UARTStdioIntHandler(void);

void UART0_Handler(void){
  UARTStdioIntHandler();
} // UART0_Handler

void TIMER0B_Handler()
{
	TimerIntClear(TIMER0_BASE,TIMER_TIMB_TIMEOUT);
	TimerDisable(TIMER0_BASE, TIMER_BOTH);
	iteracoes = -1;
	
	return;
}

void TIMER0A_Handler()
{
	TimerIntClear(TIMER0_BASE,TIMER_CAPA_EVENT);
	if(ucFlag)
	{
		uliTempoAnterior = uliTempoAtual;
		uliTempoAtual = TimerValueGet(TIMER0_BASE, TIMER_A);
		
		if(uliTempoAnterior > uliTempoAtual)
		{
			ucFlag = 0;
		}
		
		else
		{
			if(ucLevel)
			{
				alta += (uliTempoAtual - uliTempoAnterior);
			}
		
			else
			{
				baixa += (uliTempoAtual - uliTempoAnterior);
			}

			iteracoes++;
			if(iteracoes >= 2)
			{
				TimerDisable(TIMER0_BASE, TIMER_BOTH);
			}
		}
	}

	else
	{
		ucFlag = 1;
		uliTempoAtual = TimerValueGet(TIMER0_BASE, TIMER_A);
	}
	ucLevel = !ucLevel;
	return;
}

void initGPIO()
{
	// Enable the GPIOA peripheral
	SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOD);

	// Wait for the GPIOA module to be ready.
	while(!SysCtlPeripheralReady(SYSCTL_PERIPH_GPIOD)){}

	// Initialize the GPIO pin configuration.
	// Set pin 7 as input, SW controlled.
	GPIOPinTypeGPIOInput(GPIO_PORTD_BASE,GPIO_PIN_0);
	GPIOPinTypeTimer(GPIO_PORTD_BASE, GPIO_PIN_0); 
	GPIOPinConfigure(0x00030003);
	return;
}

void initTimer()
{
	SysCtlPeripheralEnable(SYSCTL_PERIPH_TIMER0);
	while(!SysCtlPeripheralReady(SYSCTL_PERIPH_TIMER0));
	//TimerDisable(TIMER0_BASE, TIMER_BOTH);
	TimerConfigure(TIMER0_BASE, (TIMER_CFG_SPLIT_PAIR | TIMER_CFG_A_CAP_TIME_UP | TIMER_CFG_B_ONE_SHOT));
	TimerControlEvent(TIMER0_BASE, TIMER_A, TIMER_EVENT_BOTH_EDGES);
	TimerPrescaleSet(TIMER0_BASE, TIMER_A, 255);
	TimerPrescaleSet(TIMER0_BASE, TIMER_B, 255);
	TimerLoadSet(TIMER0_BASE, TIMER_A, 0);
	TimerLoadSet(TIMER0_BASE, TIMER_B, 0xFFFF);
	TimerIntEnable(TIMER0_BASE, TIMER_CAPA_EVENT | TIMER_TIMB_TIMEOUT);
	IntRegister(INT_TIMER0A_TM4C129, TIMER0A_Handler);
	IntRegister(INT_TIMER0B_TM4C129, TIMER0B_Handler);
	IntEnable(INT_TIMER0A_TM4C129);
	IntEnable(INT_TIMER0B_TM4C129);
	IntMasterEnable();
	return;
}

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

  // Initialize the UART for console I/O.
  UARTStdioConfig(0, 9600, clock_atual);
  return;
} // UARTInit

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
	// config pra um clock de 40MHz, return: clock que foi efetivamente setado
	clock_atual = SysCtlClockFreqSet((SYSCTL_XTAL_25MHZ | SYSCTL_OSC_MAIN | SYSCTL_USE_PLL | SYSCTL_CFG_VCO_480), __SYSTEM_CLOCK);
	initGPIO();
	initTimer();
	UARTInit();

	ucLevel = GPIOPinRead(GPIO_PORTD_BASE,(GPIO_PIN_0));
	TimerEnable(TIMER0_BASE, TIMER_BOTH);

	while(1)
	{

		if(iteracoes >= 2 || iteracoes == -1)
		{
			if(iteracoes >= 2)
			{
				alta = alta/clock_atual;
				baixa = baixa/clock_atual;
				periodo = alta + baixa;
				frequencia = (1/periodo);
				ciclo_de_trabalho = (alta/periodo)*100;
			}
			
			else if(iteracoes == -1)
			{
				alta = alta/clock_atual;
				baixa = baixa/clock_atual;
				periodo = 0;
				frequencia = 0;
				if(ucLevel)
				{
					ciclo_de_trabalho = 100;
				}
				else
				{
					ciclo_de_trabalho = 0;
				}	
				
			}
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

			iteracoes = 0;
			alta = 0;
			baixa = 0;
			ucFlag = 0;
			uliTempoAtual = 0;
			uliTempoAnterior = 0;

			TimerLoadSet(TIMER0_BASE, TIMER_A, 0);
			TimerLoadSet(TIMER0_BASE, TIMER_B, 0xFFFF);
			ucLevel = GPIOPinRead(GPIO_PORTD_BASE,(GPIO_PIN_0));
			TimerEnable(TIMER0_BASE, TIMER_BOTH);
		}
	} // while infinito
	
}//main