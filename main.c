#include "DSP28x_Project.h"
#include "LCD2x16Display.h"
#include "targilim.h"
#include "gpio.h"
#define TB_CLK_DIV1    0x0000  // or 0x0001 depending on your setup
#define KB_BUFFER_SIZE 64
void DelfinoEvbGpioSelect(void);
void FlaxInitEPwm2(Uint16 Period);
interrupt void cpu_timer0_isr(void);
interrupt void Xint3456_isr(void);
void ConfigAndInstallKBInt();
Uint16 period = 50000;
int x = 0;
int i = 0;

void main()
{

	InitSysCtrl();
	DelfinoEvbGpioSelect();
	InitPieCtrl();
	InitPieVectTable();
	ConfigAndInstallKBInt();
	FlaxInitEPwm2(period);
	EALLOW;
	PieVectTable.TINT0 = &cpu_timer0_isr;
	EDIS;



	InitCpuTimers();   // For this example, only initialize the Cpu Timers
#if (CPU_FRQ_150MHZ)
	// Configure CPU-Timer 0 to interrupt every 500 milliseconds:
	// 150MHz CPU Freq, 50 millisecond Period (in uSeconds)
	ConfigCpuTimer(&CpuTimer0, 150, 1000000);
#endif

	CpuTimer0Regs.TCR.all = 0x4001; // Use write-only instruction to set TSS bit = 0
	EALLOW;
	GpioCtrlRegs.GPBMUX1.bit.GPIO32 = 0;
	GpioCtrlRegs.GPBDIR.bit.GPIO32 = 1;
	EDIS;


	IER |= M_INT1;
	PieCtrlRegs.PIEIER1.bit.INTx7 = 1;
	EINT;   // Enable Global interrupt INTM
	ERTM;   // Enable Global realtime interrupt DBGM

	// Configure GPIO32 as a GPIO output pin


	BackLightLCD(1);
	InItLCD();
	ClearLCD();

}
void FlaxInitEPwm2(Uint16 Period)
{
#define EPWM2_CMPA_INIT 0
#define EPWM2_CMPB_INIT 0

#define TB_HSCLK_DIV1    0x0000  // No clock division
#define FREESOFT         0x00    // No software control

	EALLOW;

	// Configure ePWM2
	EPwm2Regs.TBPRD = Period;          // Set PWM period   איתחולים
	EPwm2Regs.TBPHS.all = 0;           // Phase = 0
	EPwm2Regs.TBCTR = 0x0000;          // Reset counter
	EPwm2Regs.TBCTL.bit.CTRMODE = TB_COUNT_UP;  // Count up mode
	EPwm2Regs.TBCTL.bit.PHSEN = TB_DISABLE;     // Disable phase loading
	EPwm2Regs.TBCTL.bit.PRDLD = TB_SHADOW;      // Load period in shadow
	EPwm2Regs.TBCTL.bit.SYNCOSEL = TB_SYNC_IN;  // Sync on input
	EPwm2Regs.TBCTL.bit.HSPCLKDIV = TB_HSCLK_DIV1;  // High-speed clock divider  //לעבוד עם תדר גבוה מ16 ביט
	EPwm2Regs.TBCTL.bit.CLKDIV = TB_CLK_DIV1;  // Low-speed clock divider
	EPwm2Regs.TBCTL.bit.FREE_SOFT = FREESOFT; // Free-running mode control איתחולים

	// Configure the compare values for PWM (CMPA/B)
	EPwm2Regs.CMPA.half.CMPA = EPWM2_CMPA_INIT;  // Initialize CMPA

	EPwm2Regs.CMPCTL.bit.SHDWAMODE = CC_SHADOW;    // Shadow mode for CMPA 
	EPwm2Regs.CMPCTL.bit.LOADAMODE = CC_CTR_ZERO;  // Load on counter = 0


	// Set the action qualifier for PWM
	EPwm2Regs.AQCTLA.bit.ZRO = AQ_SET;  // Set(1) output on timer when Zero
	EPwm2Regs.AQCTLA.bit.CAU = AQ_CLEAR; // Clear(0) output on Compare A Up


	// Configure GPIO1 (or the correct pin for J12) for ePWM2A output
	GpioCtrlRegs.GPAMUX1.bit.GPIO2 = 1;  // Set GPIO2 to ePWM2A

	EDIS;
}



interrupt void Xint3456_isr(void)
{
	x = 1;
	i = 0;

	Uint16 period = 5000;
	Beep(50);

	int reset = 0;
	int decrease = 0;
	int increase = 0;
	char key = ReadKB(1);
	if (key == '1')
		increase = 1;
	else if (key == '2')
		decrease = 1;

	else if (key == '#')
		reset = 1;

	state_machine(decrease, increase, reset, period);





	// Acknowledge this interrupt to receive more interrupts from group 12
	PieCtrlRegs.PIEACK.all = PIEACK_GROUP12;
}
interrupt void cpu_timer0_isr(void)
{

	if (x == 1)
	{
		if (i == 10)
		{
			x = 0;
			i == 0;
		}
		GoToLCD(1, 0);
		printNumber(i);
		i++;
	}
	PieCtrlRegs.PIEACK.all = PIEACK_GROUP1; // Acknowledge this interrupt to receive more interrupts from group 1
}
