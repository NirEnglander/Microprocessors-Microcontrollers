#include "DSP28x_Project.h" // Include the device-specific header file
#include "LCD2x16Display.h" // Include the header for the LCD 2x16 display
#include "gpio.h"           // Include the GPIO header for input/output operations

#define TRUE 1 // Define TRUE as 1 for clarity and readability
typedef enum { DECREASE, INCREASE } STATES;
static STATES current_state = DECREASE;
int counter = 0;
int dutyCycle = 0;
// Define the possible states of the system



//keyboard init
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



void printNumber(int num)
{
    char buffer[12]; // Array to store the number as a string (including sign and null terminator)
    int isNegative = 0; // Flag for negative number

    // Handle negative numbers
    if (num < 0)
    {
        isNegative = 1;
        num = -num; // Convert to positive for further processing
    }

    int i = 0; // Index for storing digits in the array

    // Decompose the number into digits and store them in reverse order
    do
    {
        buffer[i++] = (num % 10) + '0'; // Compute the least significant digit and convert to ASCII
        num /= 10;                      // Remove the least significant digit
    } while (num > 0);

    // Add negative sign if necessary
    if (isNegative)
    {
        buffer[i++] = '-';
    }

    // Display the digits in reverse order (from most significant to least significant)
    while (i > 0)
    {
        PutcLCD(buffer[--i]); // Display each character
    }
}
void state_machine(int decrease, int increase, int reset, int period)
{
    switch (current_state)
    {
    case INCREASE:
        if (decrease == 1)
        {
            dutyCycle -= 10;
            if (dutyCycle < 0)
                dutyCycle = 0;

            current_state = DECREASE;
            ClearLCD();
            GoToLCD(0, 0);
            PrintLCD("Duty cycle=");
            printNumber(dutyCycle);
            PutcLCD('%');
            EPwm2Regs.CMPA.half.CMPA = (Uint16)(dutyCycle * 0.01 * period);
        }
        if (reset == 1)
        {
            current_state = DECREASE;
            dutyCycle = 0;
            EPwm2Regs.CMPA.half.CMPA = (Uint16)(dutyCycle * 0.01 * period);
            ClearLCD();
            GoToLCD(0, 0);
            PrintLCD("Duty cycle=0%");
        }
        if (increase == 1)
        {
            dutyCycle += 10;
            if (dutyCycle > 100)
                dutyCycle = 100;

            current_state = INCREASE;
            ClearLCD();
            GoToLCD(0, 0);
            PrintLCD("Duty cycle=");
            printNumber(dutyCycle);
            PutcLCD('%');
            EPwm2Regs.CMPA.half.CMPA = (Uint16)(dutyCycle * 0.01 * period);
        }
        break;

    case DECREASE:
        if (decrease == 1)
        {
            if (dutyCycle <= 0)
            {
                dutyCycle = 0;
                current_state = DECREASE;
                EPwm2Regs.CMPA.half.CMPA = (Uint16)(dutyCycle * 0.01 * period);
                ClearLCD();
                GoToLCD(0, 0);
                PrintLCD("Duty cycle=0%");
            }
            else
            {
                dutyCycle -= 10;
                current_state = DECREASE;
                ClearLCD();
                GoToLCD(0, 0);
                PrintLCD("Duty cycle=");
                printNumber(dutyCycle);
                PutcLCD('%');
                EPwm2Regs.CMPA.half.CMPA = (Uint16)(dutyCycle * 0.01 * period);
            }
        }
        if (reset == 1)
        {
            current_state = DECREASE;
            dutyCycle = 0;
            EPwm2Regs.CMPA.half.CMPA = (Uint16)(dutyCycle * 0.01 * period);
            ClearLCD();
            GoToLCD(0, 0);
            PrintLCD("Duty cycle=0%");
        }
        if (increase == 1)
        {
            dutyCycle += 10;
            if (dutyCycle > 100)
                dutyCycle = 100;

            ClearLCD();
            GoToLCD(0, 0);
            current_state = INCREASE;
            PrintLCD("Duty cycle=");
            printNumber(dutyCycle);
            PutcLCD('%');
            EPwm2Regs.CMPA.half.CMPA = (Uint16)(dutyCycle * 0.01 * period);
        }
        break;
    }
}
