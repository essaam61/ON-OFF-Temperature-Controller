/* FreeRTOS includes. */
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include <stdint.h>
#include <stdbool.h>
#include "sysctl.h"
#include "tm4c123gh6pm.h"
#include "task.h"
#include "basic_io.h"
#include "inc/hw_gpio.h"
#include "timers.h"
#include "driverlib/uart.h"
#include "semphr.h"
#include "inc/hw_types.h"
#include "inc/hw_memmap.h"
#include "driverlib/sysctl.h"
#include "driverlib/gpio.h"
#include "LCD.h"
#include "ADC.h"
#include "TM4C123.h" /* include register defintion file of TM4C123GH6PM */

const portTickType xTicksToWait = 100 / portTICK_RATE_MS;
volatile unsigned long long setpoint;
long long UART0_Receiver(void) //This function receives setpoint from user through putty
{
    long long data;
      while((UART0->FR & (1<<4)) != 0); //wait until Rx buffer is not full
    data = UART0->DR ;      /* before giving it another byte */
    return (unsigned long long) data; 
}
void UART0_Transmitter(unsigned long long data) //This function dispalys setpoint on the putty  
{
    while((UART0->FR & (1<<5)) != 0); // wait until Tx buffer not full 
    UART0->DR = data;                  //before giving it another byte 
}

void toString (char tim, char text []){
	//initialize text [0,0]
	for (int j =0; j<2; j++){
		text[j]='0';
	}
	//put numbers in char array
        int i = 2;
	while (tim != 0){
		i--;		
		text[i]=((tim%10)+'0');
		tim/=10;
	}
	text[2]='\0';//add null terminator
}
void printchar(char c){
while((UART0_FR_R&(1<<5))!=0);
UART0_DR_R=c;
}


void print(char *string){
  while(*string){
  printchar(*(string++));
  }
}
static void UARTINIT( void )
{	
SYSCTL_RCGCUART_R=0x03;	//Activate	UART0
//PORTA Intialization for enabling UART0
SYSCTL_RCGCGPIO_R = 0x01; // activate clock for port B&F & A
	GPIO_PORTA_AFSEL_R|=0x03;
	GPIO_PORTA_PUR_R = 0x08;
	GPIO_PORTA_PCTL_R =0x11;
	GPIO_PORTA_DEN_R |= 0x4F;   // digital I/O on PB1-0
  GPIO_PORTA_AMSEL_R &= ~0x03; // No analog on PB1-0
	GPIO_PORTA_DIR_R = 0x44;
	//UART	Initialization to print Up and Down on putty
  UART0_CTL_R &= ~0x01;		//Disable	Uart0
  UART0_IBRD_R=325; 	//baud rate =9600 bit/sec   /50MHz
  UART0_FBRD_R=33;
  UART0_LCRH_R=0x70;
	UART0_CTL_R =0x301;  //Enabling	Tx	&UART
	
}
	
	
 void PORTF_INIT(){
	//PORTF Buttons Initialization
SYSCTL_RCGCGPIO_R |= 0x20;
while((SYSCTL_PRGPIO_R&0x00000020) == 0){};
GPIO_PORTF_LOCK_R = 0x4C4F434B;
GPIO_PORTF_CR_R = 0x1F;
GPIO_PORTF_DIR_R = 0X0E;
GPIO_PORTF_PUR_R = 0x11;
GPIO_PORTF_DEN_R = 0x0E;
 }
 void PORTA_INIT(){
	//PORTA Button Initialization for Buzzer
SYSCTL_RCGCGPIO_R |= 0x01;
while((SYSCTL_PRGPIO_R&0x00000001) == 0){};
GPIO_PORTA_DIR_R = 0X80;
GPIO_PORTA_DEN_R = 0x80; // PA7 SET DIGITAL MODE
 }


xQueueHandle xUARTQueue;
xQueueHandle xLCDQueue;
xQueueHandle xBuzzerQueue;

// Task 1 main controller function
void Task1(void *pvParameters){

	typedef struct Message{
		char Txt1[4];
		char Txt2[4];
	}AMessage;
	
	AMessage msg;
#define LED 0x01;
#define ON 1
#define OFF 0
	char *cpy;
	char *on;
	char off=0;

	unsigned AdcValue;
	volatile unsigned long long Temperature;
	float mV;
	unsigned const char AlarmValue=50;
ADCinit();
on=1 ;
off=0;
while(1){
	
	xQueueReceive(xUARTQueue,&setpoint,xTicksToWait );
	AdcValue=ADCread();  //read temperature from sensor 
	mV=(mV-500.0)/10.0;
	Temperature=(long long)mV;
	
	if (Temperature < setpoint) //if temperature greater than setpoint turn on blue led and this indicates that heater is on
		{
	GPIO_PORTF_DATA_R=0X04;      //BLUE LED ON,HEATER ON
	}
	if (Temperature >setpoint)  //if temperature less than setpoint turn on RED led
	{
	GPIO_PORTF_DATA_R=0X02;      //RED LED ON,HEATER OFF
	}
	
	toString(Temperature,msg.Txt1);  //It is function converts from byte to string,as the lcd receives string only 
	toString(setpoint,msg.Txt2);
	
	xQueueSend(xLCDQueue,&msg,0); //sends the Temperature & setpoint  to lcd

	if(Temperature> AlarmValue)  //if temp > alarmvalue Buzzer is on
   xQueueSend(xBuzzerQueue,&on,0);  //sends 1 to buzzer queue to turn on buzzer 
	else
	 xQueueSend(xBuzzerQueue,&off,0);  //sends 0 to buzzer queue to turn off buzzer 
}
}


// Task 2  UART communication with putty

void Task2(void *pvParameters){
	
	unsigned AdcValue;
	unsigned  int Total;
	

	while(1){

	  print("\n\r\n\rEnter Temperature Setpoint (Degrees): ");
		setpoint=0;
		Total=0;
		
		while(1){

		setpoint=UART0_Receiver();  //This function receives setpoint from user through putty
		UART0_Transmitter(setpoint); //This function dispalys setpoint on the putty  
	  if(setpoint== '\r') break;  // if Enter button is pressed -> break
        setpoint = setpoint- '0';
   			Total = 10*Total + setpoint;
				
		}
	
		xQueueSend(xUARTQueue,&Total,pdMS_TO_TICKS(10)); //sends the new setpoint to the queue
		print("\n\rTemperature setpoint changed...");
		//break;
		
	}
	
}

// Task 3 LCD print on lcd the measured temperature and setpoint 

void Task3(void *pvParameters)
{
	typedef struct Message
		{
			char Txt1[4];
			char Txt2[4];
		}Amessage;
		
		Amessage msg;
	LCD screen;
	screen=LCD_create();
	LCD_setup(&screen);
	LCD_init(&screen);
		
		while(1)
		{ 
			xQueueReceive(xLCDQueue, &msg,0); //Receives temperature and setpoint from Task 1
			LCD_clear(&screen);   // clean the screen
			LCD_setPosition(&screen,1,1);  //set the position(first line)
			LCD_sendString(&screen, "Measured: "); 
			LCD_sendString(&screen,msg.Txt1); //show temperature in LCD
			LCD_sendByte(&screen,1,(char)223);
			LCD_sendString(&screen, "C"); //Celsius
			LCD_setPosition(&screen,2,1);//set the position(second line)
			LCD_sendString(&screen, "Setpoint: "); 
			LCD_sendString(&screen,msg.Txt2); //show setpoint in LCD
			LCD_sendByte(&screen,1,(char)223);
			LCD_sendString(&screen, "C"); //Celsius
			vTaskDelay(pdMS_TO_TICKS(1000)); 
		}
}

// Task 4 buzzer control

void Task4(void *pvParameters){	
	
	unsigned char BuzzerState;
	BuzzerState=0;
	
	while(1){
		
xQueueReceive(xBuzzerQueue,&BuzzerState,0);	//receive buzzerstate from task 1
		if(BuzzerState==1)
			GPIO_PORTA_DATA_R=0x80;  // TURN ON BUZZER
		else
			GPIO_PORTA_DATA_R=0x0; // TURN OFF BUZZER
	}
	
}

int main(){

	UARTINIT(); 
	PORTF_INIT();
	PORTA_INIT();

	xUARTQueue=xQueueCreate(1, 1);
	xLCDQueue=xQueueCreate(1, 8);
	xBuzzerQueue= xQueueCreate(1, 1);

	// create Task 1
	xTaskCreate((TaskFunction_t)Task1,"MAIN Controller", configMINIMAL_STACK_SIZE,NULL,10,NULL);
	// create Task 2
	xTaskCreate((TaskFunction_t)Task2,"UART Controller", configMINIMAL_STACK_SIZE,NULL,10,NULL);
	// create Task 3
	xTaskCreate((TaskFunction_t)Task3,"LCD Controller", configMINIMAL_STACK_SIZE,NULL,10,NULL);
	// create Task 4
	xTaskCreate((TaskFunction_t)Task4,"BUZZER Controller", configMINIMAL_STACK_SIZE,NULL,10,NULL);
	
	vTaskStartScheduler();
}
	
	