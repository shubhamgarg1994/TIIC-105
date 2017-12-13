#include <stdint.h>
#include <stdbool.h>
#include "inc/hw_memmap.h"
#include "inc/hw_types.h"
#include "driverlib/fpu.h"
#include "driverlib/sysctl.h"
#include "driverlib/pin_map.h"
#include "driverlib/uart.h"
#include "utils/uartstdio.h"
#include "driverlib/gpio.h"
#include "Sensor.h"
#include "GSM.h"
#include <stdio.h>
#include <string.h>

int main(void)
{
	unsigned char i=0,j=0;
	uint8_t ref;
	uint8_t id;
	char msg[30];
	char sensor_data[80];

	/*
	 * The System clock is run using a 16 Mhz crystal connected to the main oscillator pins of the microcontroller
	 * This generates a internal clock signal of 400 Mhz using the PLL
	 * The signal is prescaled by the system by 2
	 * Now we are defining a prescale of 5 in addition to make the clock frequency 40MHz
	 * The system clock frequency must be less than or equal to 80MHz
	 */
	SysCtlClockSet(SYSCTL_SYSDIV_5|SYSCTL_USE_PLL|SYSCTL_XTAL_16MHZ|SYSCTL_OSC_MAIN);

	/*
	 * Peripherals are enabled with this function.
	 *  At power-up, all peripherals are disabled.
	 *  System Clock to the peripheral must be enabled in order to use UART0
	 */

	SysCtlPeripheralEnable(SYSCTL_PERIPH_UART1);

	SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOC); // As GPIO Pins C4 and C5 are multiplexed with U1RX and U1TX



	//Configuring the GPIO A0 and A1 for UART functionality
	GPIOPinConfigure(GPIO_PC4_U1RX);

	GPIOPinConfigure(GPIO_PC5_U1TX);

	GPIOPinTypeUART(GPIO_PORTC_BASE, GPIO_PIN_4|GPIO_PIN_5);

	UARTConfigSetExpClk(UART1_BASE, SysCtlClockGet(),9600, UART_CONFIG_WLEN_8|UART_CONFIG_PAR_NONE|UART_CONFIG_STOP_ONE);

	/*
	 * Configures the UART to use the current system clock
	 * Sets the baud rate to 9600 and enables an 8bit no parity one stop bit communication(8-N-1)
	 */

	ADC_init();


	for (i=0;i<10000;i++)
	{
		for(j=0;j<10000;j++)
		SysCtlDelay(SysCtlClockGet());
	}
				Sensor_Read();
			sprintf(sensor_data,"CO2 = %f\nCH4 = %f\nO3 = %f\nH2 = %f\nCO = %f\n",CO2_Conc,Methane_Conc,Ozone_Conc,Hydrogen_Conc,CarbonMono_Conc);
	 GSM_init();
	 while(1)
	 {
		 while(SIM300WaitForMsg(&id)!=1);

		 for(i=0;i<100;i++)
		 SysCtlDelay(SysCtlClockGet()/10);

		 while(SIM300ReadMsg(id,msg)!=1);
		 while(SIM300DeleteMsg(id)!=1);

		 if(strcmp(msg,"Read Sensors") == 0)
		 while(SIM300SendMsg("+917600046837",sensor_data,&ref));//Change phone number to some valid value!
	 }
 }
