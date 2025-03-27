#include "DSP28x_Project.h" // Include the device-specific header file
#include "LCD2x16Display.h" // Include the header for the LCD 2x16 display
#include "gpio.h"           // Include the GPIO header for input/output operations

#define TRUE 1 // Define TRUE as 1 for clarity and readability
typedef enum { DECREASE, INCREASE} STATES;
static STATES current_state = DECREASE;
int counter = 0;
int dutyCycle = 0;
// Define the possible states of the system


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
