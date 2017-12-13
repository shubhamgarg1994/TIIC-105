#ifndef GSM_H_
#define GSM_H_

#include "driverlib/UART.h"
#include "utils/UARTstdio.h"
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdlib.h>

char sim300_buffer[128];


void UART_Transmit_String(char *buf)
{
	//Waits until the UART is Ready
				while(UARTBusy(UART1_BASE));

			while(*buf != '\0')
				UARTCharPut(UART1_BASE,*buf++);
}

void UART_Transmit_Number(uint32_t num)
{
	unsigned char Number[4];
	unsigned i=0,rem=0;
	if(num ==0)
		UARTCharPut(UART1_BASE,0x30);
	else
	{
	while(num !=0)
	{
	rem = num %10;
	Number[i]= rem + 0x30;
	num =num /10;
	i++;
	}

	while(i!=0)
	{
	UARTCharPut(UART1_BASE,Number[i-1]);
			i--;
	}
	}
}

int8_t SIM300Cmd(char *cmd)
{

	UART_Transmit_String(cmd);
	UARTCharPut(UART1_BASE,0x0D);

	 uint8_t len=strlen(cmd);

	 len++;   //Add 1 for trailing CR added to all commands

	 uint16_t p=0;
	 while(p < len)
	 {
		 sim300_buffer[p] = UARTgetc();
		 p++;
	 }
	 return 1;
}

 int8_t SIM300WaitForResponse()
 {
	      uint8_t i=0;
	      while(sim300_buffer[i]==0x0D && i!=0)
	      {
	       sim300_buffer[i]=UARTgetc();
	       i++;
	      }
	       return i+1;
 }

 int8_t SIM300WaitForMsg(uint8_t *id)
 {
    //Wait for a unsolicited response for 250ms
    uint8_t len=SIM300WaitForResponse();

    if(len==0)
       return 0;

    sim300_buffer[len-1]='\0';

    //Check if the response is +CMTI (Incoming msg indicator)
    if(strncmp(sim300_buffer+2,"+CMTI:",6)==0)
    {
       char str_id[4];

       char *start;

       start=strchr(sim300_buffer,',');
       start++;

       strcpy(str_id,start);

       *id=atoi(str_id);

       return 1;
    }
    else
       return 0;
 }

 int8_t SIM300ReadMsg(uint8_t i, char *msg)
 {
    //String for storing the command to be sent
    char cmd[16];

    //Build command string
    sprintf(cmd,"AT+CMGR=%d",i);

    //Send Command
    SIM300Cmd(cmd);

    uint8_t len=SIM300WaitForResponse();

    if(len==0)
       return 0;

    sim300_buffer[len-1]='\0';

    //Check of SIM NOT Ready error
    if(strcmp(sim300_buffer+2,"+CMS ERROR: 517")==0)
       return 0;


    //MSG Slot Empty
    if(strcmp(sim300_buffer+2,"OK")==0)
       return 0;

    //Now read the actual msg text
    len=SIM300WaitForResponse();

    if(len==0)
       return 0;

    sim300_buffer[len-1]='\0';
    strcpy(msg,sim300_buffer+1);//+1 for removing trailing LF of prev line

    return 1;
 }

 int8_t SIM300SendMsg(const char *num, char *msg, uint8_t *msg_ref)
 {
    char cmd[25];
    uint8_t p=0;
    char buffer[100];

    sprintf(cmd,"AT+CMGS= %s",num);

    cmd[8]=0x22; //"

    uint8_t n=strlen(cmd);

    cmd[n]=0x22; //"
    cmd[n+1]='\0';

    //Send Command
    SIM300Cmd(cmd);

    SysCtlDelay(SysCtlClockGet()/10);

    UART_Transmit_String(msg);

    UARTCharPut(UART1_BASE,0x1A);

 	//Wait for echo
    while(p < (strlen(msg)+5))
    {
    	buffer[p] = UARTgetc();
    	p++;
    }

    //Remove Echo
    UARTgets(sim300_buffer,strlen(msg)+5);

    uint8_t len=SIM300WaitForResponse();

    if(len==0)
       return 0;

    sim300_buffer[len-1]='\0';

    if(strncmp(sim300_buffer+2,"CMGS:",5)==0)
    {
       *msg_ref=atoi(sim300_buffer+8);
        return 1;
    }

    else
    return 0;
 }

 int8_t   SIM300DeleteMsg(uint8_t i)
 {
    //String for storing the command to be sent
    char cmd[16];

    //Build command string
    sprintf(cmd,"AT+CMGD=%d",i);

    //Send Command
    SIM300Cmd(cmd);

    uint8_t len=SIM300WaitForResponse();

    if(len==0)
       return 0;

    sim300_buffer[len-1]='\0';

    //Check if the response is OK
    if(strcmp(sim300_buffer+2,"OK")==0)
       return 1;
    else
       return 0;
 }

 int8_t SIM300CheckResponse(const char *response,const char *check,uint8_t len)
 {
	uint8_t i;
 	len-=2;

 	//Check leading CR LF
 	if(response[0]!=0x0D | response[1]!=0x0A)
 		return 0;

 	//Check trailing CR LF
 	if(response[len]!=0x0D | response[len+1]!=0x0A)
 		return 0;


 	//Compare the response
 	for(i=2;i<len;i++)
 	{
 		if(response[i]!=check[i-2])
 			return 0;
 	}

 	return 1;
 }

void GSM_init()
{
	SIM300Cmd("AT");
	UARTgets(sim300_buffer,6);	//Read serial Data
	while(!(SIM300CheckResponse(sim300_buffer,"OK",6)));
}

#endif /* GSM_H_ */

