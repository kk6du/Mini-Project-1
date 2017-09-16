#include <stdint.h>
#include "PLL.h"
#include "LCD.h"
#include "os.h"
#include "joystick.h"
#include "FIFO.h"

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

uint16_t origin[2]; // the original ADC value of x,y if the joystick is not touched
int16_t x = 63;  // horizontal position of the crosshair, initially 63
int16_t y = 63;  // vertical position of the crosshair, initially 63
int16_t prevx,prevy;
uint8_t select;  // joystick push

void CrossHair_Init(void){
	BSP_LCD_FillScreen(BGCOLOR);
	BSP_Joystick_Input(&origin[0],&origin[1],&select);
}

void Producer(void){	
	uint16_t rawX,rawY; // raw adc value
	uint8_t select;
	rxDataType data;
	BSP_Joystick_Input(&rawX,&rawY,&select);
	// here is your code
}

void Consumer(void){
	rxDataType data;
	// here is your code
}

int main(void){
  PLL_Init(Bus80MHz);       // set system clock to 80 MHz
  BSP_LCD_Init();           // initialize LCD
  BSP_Joystick_Init();
  CrossHair_Init();
  RxFifo_Init();
	
  OS_AddPeriodicThread(TASK,PERIOD,PRIORITY);

  while(1){
     Consumer();
  }
}
 
