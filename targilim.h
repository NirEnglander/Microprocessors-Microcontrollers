#ifndef targilim_h
#define targilim_h
void DisplayNumber(int num);
void state_machine(int decrease, int increase, int reset, int period);
inline static void KeboardWriteCode(char data);
inline static char KeboardReadCode();
inline void Beep(int MiliSec);
static char scan2ascii(char sc);
char ReadKB(char wait);
void ConfigAndInstallKBInt();
#endif
