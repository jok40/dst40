#ifndef KEYBOARD_H_
#define KEYBOARD_H_

#include <stdint.h>

void echoOnOff( bool );
int kbhit( void );
int getch( void );
int getYN( void );
int32_t getString( char *, int32_t, const char *, bool );
uint64_t getKeyCode( void );


// Модификаторы для метода echoOnOff

#define ECHO_OFF true
#define ECHO_ON  false


// Таблица кодов функциональных клавиш

#define  KEY_BACKSPACE  0x0000000000000008
#define  KEY_ENTER      0x000000000000000A
#define  KEY_ESC        0x000000000000001B

#define  KEY_UP         0x00000000001B5B41
#define  KEY_DOWN       0x00000000001B5B42
#define  KEY_RIGHT      0x00000000001B5B43
#define  KEY_LEFT       0x00000000001B5B44

#define  KEY_HOME       0x000000001B5B317E
#define  KEY_INSERT     0x000000001B5B327E
#define  KEY_DELETE     0x000000001B5B337E
#define  KEY_END        0x000000001B5B347E
#define  KEY_PAGEUP     0x000000001B5B357E
#define  KEY_PAGEDN     0x000000001B5B367E

#define  KEY_F1         0x000000001B5B5B41
#define  KEY_F2         0x000000001B5B5B42
#define  KEY_F3         0x000000001B5B5B43
#define  KEY_F4         0x000000001B5B5B44
#define  KEY_F5         0x000000001B5B5B45

#define  KEY_F6         0x0000001B5B31377E
#define  KEY_F7         0x0000001B5B31387E
#define  KEY_F8         0x0000001B5B31397E

#define  KEY_F9         0x0000001B5B32307E
#define  KEY_F10        0x0000001B5B32317E
#define  KEY_F11        0x0000001B5B32337E
#define  KEY_F12        0x0000001B5B32347E


#endif /* KEYBOARD_H_ */
