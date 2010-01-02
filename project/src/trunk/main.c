#include <stdio.h>
#include "lcd.h"
#include "uart0.h"

static char a = 'a';

int main(void)
{
    lcdInit();
    uart0Init();

    lcdPrintString("Ugiya !");
    uart0SendByte((uint8_t)a);
    printf("Testing\n");
}
