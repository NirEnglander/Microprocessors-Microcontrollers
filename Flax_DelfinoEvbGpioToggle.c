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
int x=0;
int i=0;

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

	EPwm2Regs.CMPCTL.bit.SHDWAMODE = CC_SHADOW;    // Shadow mode for CMPA קובע איפה נשמר הערך החדש
	EPwm2Regs.CMPCTL.bit.LOADAMODE = CC_CTR_ZERO;  // Load on counter = 0 מתי הערך יעבור מהשדאו לCMPA


	// Set the action qualifier for PWM
	EPwm2Regs.AQCTLA.bit.ZRO = AQ_SET;  // Set(1) output on timer when Zero
	EPwm2Regs.AQCTLA.bit.CAU = AQ_CLEAR; // Clear(0) output on Compare A Up


	// Configure GPIO1 (or the correct pin for J12) for ePWM2A output
	GpioCtrlRegs.GPAMUX1.bit.GPIO2 = 1;  // Set GPIO2 to ePWM2A

	EDIS;
}

	inline static void KeboardWriteCode(char data)
	{
		GpioDataRegs.GPBCLEAR.all = (0x0FL << 8);
		// Clear 4 data bits GPIO40-GPIO43
		GpioDataRegs.GPBSET.all = ((long)data << 8); // Set the relevant data bits GPIO40-GPIO43
		DELAY_US(1000);
	}
	/*****************************************************************************/
	inline static char KeboardReadCode()
	{
		return((GpioDataRegs.GPBDAT.all >> 12) & 0x0FL);
	}
	// Read 4 data bits GPIO44-GPIO47
	/*****************************************************************************/
	inline void Beep(int MiliSec)
	{

		GpioDataRegs.GPASET.bit.GPIO27 = 1;
		DELAY_US(1000L * MiliSec);
		GpioDataRegs.GPACLEAR.bit.GPIO27 = 1;
		// Buzzer On
		// Buzzer Off
	}
	/*****************************************************************************/
	static char scan2ascii(char sc)
	{
		switch (sc)
		{
		case 0xE0: return('1');
		case 0xD0: return('2');
		case 0xB0: return('3');
		case 0x70: return('A');
		case 0xE1: return('4');
		case 0xD1: return('5');
		case 0xB1: return('6');
		case 0x71: return('B');
		case 0xE2: return('7');
		case 0xD2: return('8');
		case 0xB2: return('9');
		case 0x72: return('C');
		case 0xE3: return('*');
		case 0xD3: return('0');
		case 0xB3: return('#');
		case 0x73: return('D');
		}
		return(0);
	}
	/*****************************************************************************/
	char ReadKB(char wait)
	{
		static char code[] = { 0xE, 0xD, 0xB, 0x7 };
		char data;
		char i;
		KeboardWriteCode(0x0);
		DELAY_US(1000);
		while (KeboardReadCode() == 0x0F) // Check 4 data bits GPIO44-GPIO47
			if (!wait) return(0);
		Beep(20);
		for (i = 0; i < 4; i++)
		{
			KeboardWriteCode(code[i]);
			DELAY_US(1000);
			data = KeboardReadCode();
			if (data != 0x0F)
				break;
		}
		while (KeboardReadCode() != 0x0F); // Wait for button release
		DELAY_US(1000);
		KeboardWriteCode(0x0);
		return(scan2ascii((data << 4) | i));
	}
	/*****************************************************************************/

	/*****************************************************************************/
	void ConfigAndInstallKBInt()
	{
		EALLOW; // This is needed to write to EALLOW protected registers
		// Set input qualification period for GPIO44-GPIO47
		GpioCtrlRegs.GPACTRL.bit.QUALPRD3 = 1; // Qual period = SYSCLKOUT/2
		GpioCtrlRegs.GPBQSEL1.bit.GPIO44 = 2; // 6 samples
		GpioCtrlRegs.GPBQSEL1.bit.GPIO45 = 2; // 6 samples
		GpioCtrlRegs.GPBQSEL1.bit.GPIO46 = 2; // 6 samples-2
		GpioCtrlRegs.GPBQSEL1.bit.GPIO47 = 2; // 6 samples
		GpioIntRegs.GPIOXINT3SEL.all = 12;
		GpioIntRegs.GPIOXINT4SEL.all = 13;
		GpioIntRegs.GPIOXINT5SEL.all = 14;
		GpioIntRegs.GPIOXINT6SEL.all = 15;
		// Xint3 connected to GPIO44 32+12
		// Xint4 connected to GPIO45 32+13
		// Xint5 connected to GPIO46 32+14
		// Xint6 connected to GPIO47 32+15
		PieVectTable.XINT3 = &Xint3456_isr;
		PieVectTable.XINT4 = &Xint3456_isr;
		PieVectTable.XINT5 = &Xint3456_isr;
		PieVectTable.XINT6 = &Xint3456_isr;
		EDIS; // This is needed to disable write to EALLOW protected registers
		// Enable Xint 3,4,5,6 in the PIE: Group 12 interrupt 1-4
		// Enable int1 which is connected to WAKEINT:
		PieCtrlRegs.PIECTRL.bit.ENPIE = 1;
		// Enable the PIE block
		PieCtrlRegs.PIEIER12.bit.INTx1 = 1;
		PieCtrlRegs.PIEIER12.bit.INTx2 = 1;
		PieCtrlRegs.PIEIER12.bit.INTx3 = 1;
		PieCtrlRegs.PIEIER12.bit.INTx4 = 1;
		// Enable CPU int12
		IER |= M_INT12;
		// Configure XINT3-6
		XIntruptRegs.XINT3CR.bit.POLARITY = 0;
		XIntruptRegs.XINT4CR.bit.POLARITY = 0;
		XIntruptRegs.XINT5CR.bit.POLARITY = 0;
		XIntruptRegs.XINT6CR.bit.POLARITY = 0;
		// Enable XINT3-6
		XIntruptRegs.XINT3CR.bit.ENABLE = 1;
		XIntruptRegs.XINT4CR.bit.ENABLE = 1;
		XIntruptRegs.XINT5CR.bit.ENABLE = 1;
		XIntruptRegs.XINT6CR.bit.ENABLE = 1;
	}
	// Enable PIE Group 12 INT1
	// Enable PIE Group 12 INT2
	// Enable PIE Group 12 INT3
	// Enable PIE Group 12 INT4
	// Falling edge interrupt
	// Falling edge interrupt
	// Falling edge interrupt
	// Falling edge interrupt
	// Enable Xint3
	// Enable XINT4
	// Enable Xint5
	// Enable XINT6
	/*****************************************************************************/
	interrupt void Xint3456_isr(void)
	{
		x=1;
		i=0;

		Uint16 period = 5000;
		Beep(50);

		int reset = 0;
		int decrease = 0;
		int increase = 0;
		char key = ReadKB(1);
		if (key == '1')
			increase = 1;
		else if (key=='2')
				decrease = 1;

		else if (key == '#')
			reset = 1;

		state_machine(decrease,increase,reset,period);





		// Acknowledge this interrupt to receive more interrupts from group 12
		PieCtrlRegs.PIEACK.all = PIEACK_GROUP12;
	}
	interrupt void cpu_timer0_isr(void)
		{

		if(x==1)
		{
        if(i==10)
        {
        	x=0;
        	 i==0;
        }
		 GoToLCD(1, 0);
		 printNumber(i);
		  i++;
		}
		   PieCtrlRegs.PIEACK.all = PIEACK_GROUP1; // Acknowledge this interrupt to receive more interrupts from group 1
		}
