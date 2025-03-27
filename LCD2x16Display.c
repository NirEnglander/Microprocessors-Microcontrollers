/**********************************************************************
 * Tittle: Delfino Evaluation Board LCD 2x16 Display *
 * Filename: LCD2x16Display.c *
 * Date: 20-11-2014 *
 * Last Modify: 20-11-2014 *
 * File Version: 1.0 *
 * *
 * Author: Flaxer Eli *
 * Company: Flaxer.net *
 * *
 * */
#include "DSP28x_Project.h" // Device Header file and Examples Include File

/*****************************************************************************/
static inline void LcdControlBit(int bit)
{
if (bit)
GpioDataRegs.GPBSET.bit.GPIO57 = 1;
else
GpioDataRegs.GPBCLEAR.bit.GPIO57 = 1;
}
/*****************************************************************************/
static inline void LcdEnablelBit(int bit)
{
if (bit)
GpioDataRegs.GPBSET.bit.GPIO56 = 1;
else
GpioDataRegs.GPBCLEAR.bit.GPIO56 = 1;
}
/*****************************************************************************/
static inline void LcdBlinklBit(int bit)
{
if (bit)
GpioDataRegs.GPBSET.bit.GPIO58 = 1;
else
GpioDataRegs.GPBCLEAR.bit.GPIO58 = 1;
}
/*****************************************************************************/
static inline void LcdWriteData(char data)
{
GpioDataRegs.GPBCLEAR.all = (0xFFL << 16); // Clear all data bits GPIO48-GPIO55
GpioDataRegs.GPBSET.all = ((long)data << 16); // Set the relevant data bits GPIO48-GPIO55
}
/*****************************************************************************/
void BackLightLCD(int x)
{
LcdBlinklBit(x);
}
/*****************************************************************************/
void PutcLCD(const char c)
{
LcdControlBit(1); // Data
DELAY_US(1);
LcdEnablelBit(1);
LcdWriteData(c);
DELAY_US(1);
LcdEnablelBit(0);
DELAY_US(100);
}
/*****************************************************************************/
void PrintLCD(const char *str)
{
int i;
for(i=0; str[i]; i++)
{
PutcLCD(str[i]);
}
}
/*****************************************************************************/
void ClearLCD(void)
{
LcdControlBit(0); // Control
DELAY_US(1);
LcdEnablelBit(1);
LcdWriteData(0x01); // Clear code
DELAY_US(1);
LcdEnablelBit(0);
DELAY_US(10000);
}
/*****************************************************************************/
void InItLCD(void)
{
static const char LcdInitStr[] = {0x38, 0x0E, 0x06, 0x01};
int i;
LcdControlBit(0); // Control
for (i=0; i<4; i++)
{
LcdEnablelBit(1);
LcdWriteData(LcdInitStr[i]);
DELAY_US(10000);
LcdEnablelBit(0);
DELAY_US(5000);
}
}
/*****************************************************************************/
void HomeLCD()
{
LcdControlBit(0); // Control
DELAY_US(1);
LcdEnablelBit(1);
LcdWriteData(0x02); // Home Code
DELAY_US(1);
LcdEnablelBit(0);
DELAY_US(1000);
}
/*===============================================*/
void OnOffLCD(char D, char C, char B)
{
LcdControlBit(0); // Control
DELAY_US(1);
LcdEnablelBit(1);
LcdWriteData(0x08 | (D<<2)|(C<<1)|(B<<0)); // DCB Code
DELAY_US(1);
LcdEnablelBit(0);
DELAY_US(1000);
}
/*===============================================*/
void GoToLCD(char y, char x)
{
LcdControlBit(0); // Control
DELAY_US(1);
LcdEnablelBit(1);
LcdWriteData((1<<7)|(0x40*y + x)); // Set DDRAM Address
DELAY_US(1);
LcdEnablelBit(0);
DELAY_US(50);
}
/*===============================================*/
void PrintLineLCD(char line, const char *str)
{
static const char EmptyLine[]=" ";
GoToLCD(line, 0);
PrintLCD(EmptyLine);
GoToLCD(line, 0);
PrintLCD(str);
}
