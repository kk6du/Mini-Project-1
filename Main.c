#include <stdint.h>
#include "PLL.h"
#include "LCD.h"
#include "os.h"
#include "joystick.h"
#include "FIFO.h"
#include "tm4c123gh6pm.h"

void DisableInterrupts(void); // Disable interrupts
void EnableInterrupts(void);  // Enable interrupts
long StartCritical (void);    // previous I bit, disable interrupts
void EndCritical(long sr);    // restore I bit to previous value
void WaitForInterrupt(void);  // low power mode

//constants
#define BGCOLOR     LCD_BLACK
#define AXISCOLOR   LCD_ORANGE
#define MAGCOLOR    LCD_YELLOW
#define EWMACOLOR   LCD_CYAN
#define SOUNDCOLOR  LCD_CYAN
#define LIGHTCOLOR  LCD_LIGHTGREEN
#define TOPTXTCOLOR LCD_WHITE
#define TOPNUMCOLOR LCD_ORANGE
#define CROSSSIZE 5
#define PE0  (*((volatile unsigned long *)0x40024004))

uint16_t origin[2]; // the original ADC value of x,y if the joystick is not touched
int16_t x = 63;  // horizontal position of the crosshair, initially 63
int16_t y = 63;  // vertical position of the crosshair, initially 63
int16_t prevx = 63;
int16_t prevy = 63;
uint8_t select;  // joystick push

//---------------------User debugging-----------------------
//#define TEST_TIMER
#define PE0  (*((volatile unsigned long *)0x40024004))
#define TEST_PERIOD 800000  //defined by user
#define PERIOD 2000000

unsigned long Count;   // number of times thread loops

void PortE_Init(void){
  SYSCTL_RCGCGPIO_R |= 0x10;       // activate port E
  while((SYSCTL_RCGCGPIO_R & 0x10) == 0){}
  GPIO_PORTE_DIR_R |= 0x0F;    // make PE3-0 output heartbeats
  GPIO_PORTE_AFSEL_R &= ~0x0F;   // disable alt funct on PE3-0
  GPIO_PORTE_DEN_R |= 0x0F;     // enable digital I/O on PE3-0
  GPIO_PORTE_PCTL_R = ~0x0000FFFF;
  GPIO_PORTE_AMSEL_R &= ~0x0F;      // disable analog functionality on PF
}
//--------------------------------------------------------------
void CrossHair_Init(void){
	BSP_LCD_FillScreen(BGCOLOR);
	BSP_Joystick_Input(&origin[0],&origin[1],&select);
}

void Producer(void){
   #ifdef TEST_TIMER
	PE0 ^= 0x01;       // heartbeat
	Count++;
   #else
	uint16_t rawX,rawY; // raw adc value
	uint8_t select;
	rxDataType data;
	BSP_Joystick_Input(&rawX,&rawY,&select);
	// here is your code
	if (RxFifo_Put(data) == RXFIFOSUCCESS)
	{
		x = ((double)(rawX - origin[0]) * (double) ((double)(0x7F)/(double)(0x1000)));
		y = ((double)(rawY - origin[1]) * (double) ((double)(0x7F)/(double)(0x1000)));
		//origin plus delta -> delta = curr - origin then  do prev + delta
		x = x/4 + prevx;
		y = (-y/4) + prevy;
		if (x < 0) //if x is less than 0
		{
			x = 0;
		}
		if (x > 0x80) //if x is greater than 128
		{
			x = 0x80;
		}
		if (y < 0)
		{
			y = 0;
		}
		if (y > 0x74)
		{
			y = 0x74;
		}
	}
	data.x = x;
	data.y = y;
	RxFifo_Put(data);
   #endif
}

void Consumer(void){
	rxDataType data;
	// here is your code

	if (RxFifo_Get(&data) == RXFIFOSUCCESS)
	{
		RxFifo_Get(&data);
		BSP_LCD_DrawFastHLine(prevx-4, prevy, 8, LCD_BLACK);
	  BSP_LCD_DrawFastVLine(prevx, prevy-4, 8, LCD_BLACK);
		BSP_LCD_DrawFastHLine(data.x-4, data.y, 8, LCD_RED);
		BSP_LCD_DrawFastVLine(data.x, data.y-4, 8, LCD_RED);
		prevx = data.x;
		prevy = data.y;
	}
	BSP_LCD_Message (1, 0, 0, "X: ", prevx);
	BSP_LCD_Message (1, 0, 8, "Y: ", prevy);
}

int main(void){
  PLL_Init(Bus80MHz);       // set system clock to 80 MHz
  #ifdef TEST_TIMER
	PortE_Init();       // profile user threads
	Count = 0;
	OS_AddPeriodicThread(&Producer,TEST_PERIOD,1);
	while(1){}
  #else
  	BSP_LCD_Init();        // initialize LCD
	  BSP_Joystick_Init();   // initialize Joystick
  	CrossHair_Init();
  	RxFifo_Init();
	OS_AddPeriodicThread(&Producer,PERIOD,1);
	while(1){
	   Consumer();
  	}
  #endif
		//in LCD display take device * 6 + parameter + 1
}
