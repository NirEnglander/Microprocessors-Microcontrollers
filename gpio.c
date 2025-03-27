#include "DSP28x_Project.h"
#include "LCD2x16Display.h"
#include "targilim.h"


void DelfinoEvbGpioSelect(void)
{
    EALLOW;

	GpioCtrlRegs.GPAMUX1.all = 0x00000000;  	// All GPIO
	GpioCtrlRegs.GPAMUX2.all = 0x00000000;  	// All GPIO

    GpioCtrlRegs.GPADIR.all = 0x0000000F;   	// Outputs 4 Leds
    GpioCtrlRegs.GPADIR.bit.GPIO27 = 1;   		// Buzzer
    GpioCtrlRegs.GPBDIR.all = 0x07FF0F00;   	// Outputs LCD 8 Bus 3 Control
    //GpioCtrlRegs.GPBDIR.all = 0x07FF??00;   	// Extended Bus Direction GPIO40-GPIO47 KB
    GpioCtrlRegs.GPCDIR.all = 0x0000FFFF;   	// Outputs 8 Leds 4 TP 4 TestLed
    //GpioCtrlRegs.GPCDIR.all = 0x000?FFFF;   	// Extended Bus Direction GPIO80-GPIO83 Button

    //GpioCtrlRegs.GPBPUD.all = 0x0000FF00;   	// Extended Bus Pull-Up Resistors

    EDIS;
}
void GpioCSetClear(int k,int x)
{
	if (x)
		GpioDataRegs.GPCSET.all = (1L<<k);
	else
		GpioDataRegs.GPCCLEAR.all = (1L<<k);
}
