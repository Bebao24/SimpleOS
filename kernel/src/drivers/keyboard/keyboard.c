#include "keyboard.h"
#include <system.h>
#include <irq.h>
#include <pic.h>
#include <console.h>
#include <heap.h>
#include <memory.h>

key_info_t* g_KeyInfo;

static const char ASCIITable[] = {
     0 ,  0 , '1', '2',
    '3', '4', '5', '6',
    '7', '8', '9', '0',
    '-', '=',  0 ,  0 ,
    'q', 'w', 'e', 'r',
    't', 'y', 'u', 'i',
    'o', 'p', '[', ']',
     0 ,  0 , 'a', 's',
    'd', 'f', 'g', 'h',
    'j', 'k', 'l', ';',
    '\'','`',  0 , '\\',
    'z', 'x', 'c', 'v',
    'b', 'n', 'm', ',',
    '.', '/',  0 , '*',
     0 , ' '
};

void InitializeKeyboard()
{
    IRQ_RegisterHandler(1, IRQ_KeyboardHandler);
    g_KeyInfo = malloc(sizeof(key_info_t));
    memset(g_KeyInfo, 0, sizeof(key_info_t));
}

void IRQ_KeyboardHandler(cpu_registers_t* cpu)
{
    (void)cpu;

    uint8_t scancode = x64_inb(0x60);

    // Handle special keys
    switch (scancode)
    {
        case LeftShift:
            g_KeyInfo->uppercase = true;
            return;
        case LeftShift + 0x80: // Release
            g_KeyInfo->uppercase = false;
            return;
        case RightShift:
            g_KeyInfo->uppercase = true;
            return;
        case RightShift + 0x80:
            g_KeyInfo->uppercase = false;
            return;
        case Enter:
            g_KeyInfo->key = '\r';
            return;
        case BackSpace:
            g_KeyInfo->key = '\b';
            return;
    }

    char ascii = TranslateScancode(scancode, g_KeyInfo->uppercase);

    if (ascii)
    {
        g_KeyInfo->key = ascii;
    }
}

char TranslateScancode(uint8_t scancode, bool uppercase)
{
    if (scancode > 58) return 0;

    if (uppercase)
    {
        char ascii = ASCIITable[scancode];
        
        if (ascii <= 'z' && ascii >= 'a')
        {
            return ASCIITable[scancode] - 32;
        }

        switch (ascii)
        {
            case '1':
                return '!';
            case '2':
                return '@';
            case '3':
                return '#';
            case '4':
                return '$';
            case '5':
                return '%';
            case '6':
                return '^';
            case '7':
                return '&';
            case '8':
                return '*';
            case '9':
                return '(';
            case '0':
                return ')';

            case ';':
                return ':';
            case '[':
                return '{';
            case ']':
                return '}';
            case '\'':
                return '\"';

            case '-':
                return '_';
            case '=':
                return '+';
            case '\\':
                return '|';
            case ',':
                return '<';
            case '.':
                return '>';
            case '/':
                return '?';
            case '`':
                return '~';
        }
    }

    return ASCIITable[scancode];
}

char getKey()
{
    while (!g_KeyInfo->key)
    {
        asm volatile("hlt");
    }

    char key = g_KeyInfo->key;
    g_KeyInfo->key = 0;

    return key;
}



