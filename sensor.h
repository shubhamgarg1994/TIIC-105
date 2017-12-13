#ifndef SENSOR_H_
#define SENSOR_H_

#include <math.h>
#include "driverlib/adc.h"
#include "driverlib/sysctl.h"
#include "driverlib/gpio.h"
#include "GSM.h"

#define No_Of_Samples 10

/*
 * Related to Carbon dioxide Sensor
 */
#define         DC_GAIN                      5.5   //define the DC gain of amplifier
#define         ZERO_POINT_VOLTAGE           (0.220) //define the output of the sensor in volts when the concentration of CO2 is 400PPM
#define         REACTION_VOLTAGE             (0.020) //define the voltage drop of the sensor when move the sensor from air into 1000ppm CO2
float           CO2Curve[3]  =  {2.602,ZERO_POINT_VOLTAGE,(REACTION_VOLTAGE/(2.602-3))};
                                                     //two points are taken from the curve.
                                                     //with these two points, a line is formed which is
                                                     //"approximately equivalent" to the original curve.
                                                     //data format:{ x, y, slope}; point1: (lg400, 0.324), point2: (lg4000, 0.280)
                                                     //slope = ( reaction voltage ) / (log400 –log1000)

/*
 * Related to Ozone Sensor
 */
float O3Curve[3]  =  {0.69,0.90,-0.93};  //-c,1/m,Ro
                                                     //two points are taken from the curve.
                                                     //with these two points, a line is formed which is
                                                     //"approximately equivalent" to the original curve.
#define O_Load_R   160

/*
 * Related to Methane Sensor
 */
float MethaneCurve[3]  =  {3.3, 0,  -0.38};
#define M_Load_R  56

/*
 * Related to CarbonMono Sensor
 */
float CarbonMonoCurve[3]  =  {1.0,0.46,-1.01};
#define C_Load_R   200

/*
 * Related to Hydrogen Sensor
 */
float HydrogenCurve[3]  =  {0,-0.16,-0.34};
#define H_Load_R   18


float CO2_Conc = 0;
float Ozone_Conc= 0;
float Methane_Conc =0;
float Hydrogen_Conc =0;
float CarbonMono_Conc = 0;

uint32_t ulADC0Value[5] = {0};

void Read_MG811_sensor(uint32_t value)               //Carbon dioxide sensor
{
 float voltage=0;
 voltage = (value *3.3)/4095;
 CO2_Conc = pow(10,((voltage/DC_GAIN)-CO2Curve[1])/CO2Curve[2]+CO2Curve[0]);
}

void Read_MQ131_sensor(uint32_t value)              //Ozone Sensor
{
float voltage=0;
voltage = (value *3.3)/4095;
Ozone_Conc = pow(((5-voltage)*O_Load_R*pow(10,O3Curve[0]))/(voltage*O3Curve[2]),O3Curve[1]);
}

void Read_MQ8_sensor(uint32_t value)                //Hydrogen Sensor
{
	float voltage=0;
	voltage = (value *3.3)/4095;
	Hydrogen_Conc = pow(((5-voltage)*H_Load_R*pow(10,HydrogenCurve[0]))/(voltage*HydrogenCurve[2]),HydrogenCurve[1]);
}


void Read_MQ135_sensor (uint32_t value)             //Basically it measures air pollutants but I am using it for measuring CarbonMono
{
	float voltage=0;
	voltage = (value *3.3)/4095;
	CarbonMono_Conc = pow(((5-voltage)*C_Load_R*pow(10,CarbonMonoCurve[0]))/(voltage*CarbonMonoCurve[2]),CarbonMonoCurve[1]);
}

void Read_MQ214_sensor(uint32_t value)              // Methane Gas sensor
{
	float voltage=0;
	voltage = (value *3.3)/4095;
	Methane_Conc = pow(((5-voltage)*M_Load_R*pow(10,MethaneCurve[0]))/(voltage*MethaneCurve[2]),MethaneCurve[1]);
}

/*
AIN0 6 PE3 I Analog Analog-to-digital converter input 0. // MG811
AIN1 7 PE2 I Analog Analog-to-digital converter input 1. // MQ131
AIN2 8 PE1 I Analog Analog-to-digital converter input 2. // MQ8
AIN3 9 PE0 I Analog Analog-to-digital converter input 3. // MQ135
AIN4 64 PD3                                              // MQ214
*/

void ADC_init()
{
	SysCtlPeripheralEnable(SYSCTL_PERIPH_ADC0);
	SysCtlPeripheralEnable (SYSCTL_PERIPH_GPIOE);   //enable GPIO-E peripheral
	SysCtlPeripheralEnable (SYSCTL_PERIPH_GPIOD);   //enable GPIO-D peripheral

	GPIOPinTypeADC(GPIO_PORTE_BASE, GPIO_PIN_0 |GPIO_PIN_1 |GPIO_PIN_2 | GPIO_PIN_3);
	GPIOPinTypeADC(GPIO_PORTD_BASE,GPIO_PIN_3);

	ADCSequenceDisable(ADC0_BASE, 0);

	ADCSequenceConfigure(ADC0_BASE,0,ADC_TRIGGER_PROCESSOR,0 );

	//GPIOADCTriggerEnable
	ADCSequenceStepConfigure(ADC0_BASE,0, 0, ADC_CTL_CH0);
	ADCSequenceStepConfigure(ADC0_BASE,0, 1, ADC_CTL_CH1);
	ADCSequenceStepConfigure(ADC0_BASE,0, 2, ADC_CTL_CH2);
	ADCSequenceStepConfigure(ADC0_BASE,0, 3, ADC_CTL_CH3);
	ADCSequenceStepConfigure(ADC0_BASE,0, 4, ADC_CTL_CH4 | ADC_CTL_IE | ADC_CTL_END);
	ADCSequenceEnable(ADC0_BASE,0);
	ADCIntClear(ADC0_BASE,0);
}

void Sensor_Read()
{
	uint32_t sum[5]={0};
	uint8_t i=0;

	for(i=0;i<No_Of_Samples;i++)
	{
	ADCProcessorTrigger(ADC0_BASE,0);
	while(!ADCIntStatus(ADC0_BASE,0, false))
	{

	}
	ADCIntClear(ADC0_BASE,0);
	ADCSequenceDataGet(ADC0_BASE,0,ulADC0Value);
	sum[0] = sum[0] + ulADC0Value[0];
	sum[1] = sum[1] + ulADC0Value[1];
	sum[2] = sum[2] + ulADC0Value[2];
	sum[3] = sum[3] + ulADC0Value[3];
	sum[4] = sum[4] + ulADC0Value[4];

	SysCtlDelay(SysCtlClockGet()/20);
	}

	Read_MG811_sensor(sum[0]/No_Of_Samples);
	Read_MQ131_sensor(sum[1]/No_Of_Samples);
	Read_MQ8_sensor(sum[2]/No_Of_Samples);
	Read_MQ135_sensor(sum[3]/No_Of_Samples);
	Read_MQ214_sensor(sum[4]/No_Of_Samples);
}

#endif /* SENSOR_H_ */
