/******************************************************************************
 *
 * Набор методов для работы с клавиатурой.
 *
 *----------------------------------------------------------------------------
 * ESC-последовательности от дополнительных клавиш:
 *
 * HOME:    0x1B 0x5B 0x31 0x7E
 * INSERT:  0x1B 0x5B 0x32 0x7E
 * DELETE:  0x1B 0x5B 0x33 0x7E
 * END:     0x1B 0x5B 0x34 0x7E
 * PAGEUP:  0x1B 0x5B 0x35 0x7E
 * PAGEDN:  0x1B 0x5B 0x36 0x7E
 *
 * UP:      0x1B 0x5B 0x41
 * DOWN:    0x1B 0x5B 0x42
 * RIGHT:   0x1B 0x5B 0x43
 * LEFT:    0x1B 0x5B 0x44
 *
 * F1:      0x1B 0x5B 0x5B 0x41
 * F2:      0x1B 0x5B 0x5B 0x42
 * F3:      0x1B 0x5B 0x5B 0x43
 * F4:      0x1B 0x5B 0x5B 0x44
 * F5:      0x1B 0x5B 0x5B 0x45
 * F6:      0x1B 0x5B 0x31 0x37
 * F7:      0x1B 0x5B 0x31 0x38
 * F8:      0x1B 0x5B 0x31 0x39
 * F9:      0x1B 0x5B 0x32 0x30
 * F10:     0x1B 0x5B 0x32 0x31
 * F11:     0x1B 0x5B 0x32 0x33
 * F12:     0x1B 0x5B 0x32 0x34
 *****************************************************************************/

#include <sys/select.h>
#include <stddef.h>
#include <unistd.h>
#include <termios.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdbool.h>
#include <signal.h>
#include <time.h>
#include <ctype.h>
#include <sys/ioctl.h>
#include <sys/time.h>
#include "keyboard.h"


/******************************************************************************
 * Перевод терминала в режим ввода без буферирования и без эха или
 * возврат терминала в стандартный режим работы.
 *
 * Вход: mode = true  - выключить буферизацию и эхо,
 *              false - вернуть стандартный режим работы.
 *****************************************************************************/

void echoOnOff( bool mode )
{
  static bool cur_state = false;
  static struct termios oldattr;

  if( mode )
  {
    struct termios newattr;

    if( cur_state )
      return;

    tcgetattr( STDIN_FILENO, &oldattr );                        // Запрашиваем текущие аттрибуты терминала и сохраняем их в oldattr
    newattr = oldattr;                                          //
    newattr.c_lflag &= ~( ICANON | ECHO );                      // Выключаем в атрибутах буферизацию и эхо
    newattr.c_cc[VTIME] = 0;
    newattr.c_cc[VMIN]  = 1;
    tcsetattr( STDIN_FILENO, TCSANOW, &newattr );               // и записываем изменённые атрибуты обратно в терминал
    cur_state = true;                                           // Запоминаем, что режим работы терминала изменён.
  }
  else
  {
    if( !cur_state )
      return;

    tcsetattr( STDIN_FILENO, TCSANOW, &oldattr );               // Восстанавливаем прежний режим работы терминала.
  }
}


/******************************************************************************
 * Проверка нажатости клавиши.
 *
 * Чтобы в окне терминала не выводились нажатые клавиши нужно
 * перед вызовом этой функции вызвать echoOnOff(ECHO_OFF),
 * а после - echoOnOff(ECHO_ON).
 *
 * Выход: int - количество нажатых клавиш в очереди.
 *****************************************************************************/

int kbhit( void )
{
  int bytesWaiting;

  ioctl( STDIN_FILENO, FIONREAD, &bytesWaiting );
  return bytesWaiting;
}


/******************************************************************************
 * Ожидание нажатия клавиши - без вывода эха.
 *
 * Выход: int - код нажатой клавиши.
 *****************************************************************************/

int getch( void )
{
  struct termios oldattr, newattr;
  int ch;

  tcgetattr( STDIN_FILENO, &oldattr );
  newattr = oldattr;
  newattr.c_lflag &= ~( ICANON | ECHO );
  tcsetattr( STDIN_FILENO, TCSANOW, &newattr );
  ch = getchar();
  tcsetattr( STDIN_FILENO, TCSANOW, &oldattr );
  return ch;
}


/******************************************************************************
 * Ожидание нажатия клавиши - в том числе функциональной.
 *
 * Выход: uint64_t - код нажатой клавиши
 *
 * Коды функциональных клавиш (для терминала Linux):
 *
 * KEY_UP       0x00000000001B5B41
 * KEY_DOWN     0x00000000001B5B42
 * KEY_RIGHT    0x00000000001B5B43
 * KEY_LEFT     0x00000000001B5B44
 * KEY_HOME     0x000000001B5B317E
 * KEY_INSERT   0x000000001B5B327E
 * KEY_DELETE   0x000000001B5B337E
 * KEY_END      0x000000001B5B347E
 * KEY_PAGEUP   0x000000001B5B357E
 * KEY_PAGEDN   0x000000001B5B367E
 * KEY_F1       0x000000001B5B5B41
 * KEY_F2       0x000000001B5B5B42
 * KEY_F3       0x000000001B5B5B43
 * KEY_F4       0x000000001B5B5B44
 * KEY_F5       0x000000001B5B5B45
 * KEY_F6       0x0000001B5B31377E
 * KEY_F7       0x0000001B5B31387E
 * KEY_F8       0x0000001B5B31397E
 * KEY_F9       0x0000001B5B32307E
 * KEY_F10      0x0000001B5B32317E
 * KEY_F11      0x0000001B5B32337E
 * KEY_F12      0x0000001B5B32347E
 *
 *****************************************************************************/

uint64_t getKeyCode( void )
{
  // Время ожидания остатка ESC-последовательности (в миллисекундах)
  #define WAIT_ESC_SEQ 100

  // Таблица, по которой будет выполняться поиск совпадения
  static const uint64_t funcKeyTable[] =
  {
    KEY_UP,
    KEY_DOWN,
    KEY_RIGHT,
    KEY_LEFT,
    KEY_HOME,
    KEY_INSERT,
    KEY_DELETE,
    KEY_END,
    KEY_PAGEUP,
    KEY_PAGEDN,
    KEY_F1,
    KEY_F2,
    KEY_F3,
    KEY_F4,
    KEY_F5,
    KEY_F6,
    KEY_F7,
    KEY_F8,
    KEY_F9,
    KEY_F10,
    KEY_F11,
    KEY_F12,
    0
  };

  struct timespec ts = { 0, 1000000 };                          // Время сна - 1000000ns = 1мс
  static uint8_t  keyQueue[256];                                // Кольцевой буфер для считанных кодов клавиш
  static uint8_t  idxIn  = 0;                                   // Индекс записываемых в очередь символов
  static uint8_t  idxOut = 0;                                   // Индекс считываемых из очереди символов
  uint64_t        keyCode = 0x1B5B;
  uint8_t         escLen = 2;
  uint8_t         lenQueue;
  uint8_t         escTime = 0;                                  // Счётчик миллисекунд, прошедших с момента получения ESC
  int             len;

  while( 1 )
  {
    ioctl( STDIN_FILENO, FIONREAD, &len );                      // Проверяем наличие непрочитанных из терминала данных

    for( ; len > 0 && (uint8_t)(idxIn+1) != idxOut; idxIn++, len-- )  // Если есть непрочитанные данные, то считываем их в кольцевой буфер
      read( STDIN_FILENO, keyQueue + idxIn, 1 );                // keyQueue - пока они не кончатся или пока не кончится место в буфере

    lenQueue = idxIn - idxOut;                                  // Вычисляем кол-во данных в кольцевом буфере

    if( !lenQueue )                                             // Если кольцевой буфер пуст, то
    {                                                           //
      nanosleep( &ts, NULL );                                   // спим 1мс
      continue;                                                 // и продолжаем ждать следующие данные
    }

    if( keyQueue[idxOut] != 0x1B )                              // Если текущий символ в кольцевом буфере не ESC, то выходим и
      return keyQueue[idxOut++];                                // возвращаем этот символ, предварительно удалив его из кольцевого буфера.

    if( ++escTime == WAIT_ESC_SEQ )                             // Инкрементируем счётчик времени: если прошло WAIT_ESC_SEQ мс с момента получения ESC,
      return keyQueue[idxOut++];                                // то выходим и возвращаем ESC, предварительно удалив его из кольцевого буфера.

    if( lenQueue == 1 )                                         // Если в кольцевом буфере только один байт ESC, то
    {                                                           //
      nanosleep( &ts, NULL );                                   // спим 1мс
      continue;                                                 // и продолжаем ждать следующие данные
    }

    if( keyQueue[(uint8_t)(idxOut+1)] != 0x5B )                 // Если следующий за ESC символ - не 0x5B, то выходим и
      return keyQueue[idxOut++];                                // возвращаем ESC, предварительно удалив ESC из кольцевого буфера.

    while( lenQueue > escLen )                                  //  При получении каждого следующего байта
    {                                                           //
      uint8_t i = 0;                                            //
                                                                //
      keyCode = ( keyCode << 8 ) |                              //
                keyQueue[(uint8_t)(idxOut + escLen)];           // дописываем принятый байт в хвост keyCode,
      escLen++;                                                 // инкрементируем счётчик принятых байт ESC-последовательности,
                                                                //
      while( funcKeyTable[i] )                                  // и ищем новый ESC-код в списке функциональных клавиш:
      {                                                         //
        if( funcKeyTable[i++] == keyCode )                      // если код нашёлся в списке, то
        {                                                       //
          idxOut += escLen;                                     // удаляем его из циклического буфера,
          return keyCode;                                       // выходим и возвращаем принятый код.
        }
      }

      if( escLen > 5 )                                          // Если принято уже больше 5 байт, а совпадения так и нет, то
        return keyQueue[idxOut++];                              // прерываем приём ESC-последовательности:
    }                                                           // возвращаем ESC, предварительно удалив ESC из кольцевого буфера.

    nanosleep( &ts, NULL );                                     // спим 1мс
  }

  return 0;                                                     // Осчастливливаем Eclipse

  #undef WAIT_ESC_SEQ
}


/******************************************************************************
 * Ввод строки - с выводом эха.
 *
 * uint32_t getString( char *buf, uint32_t size, const char *mask )
 *
 * Вход:  buf  - указатель на входной буфер,
 *        size - длина входного буфера,
 *        mask - строка, содержащая все допустимые символы,
 *        toup - true, если необходимо преобразовывать вводимые символы
 *                     в верхний регистр.
 *
 * Выход: int  - длина введённой строки,
 *        buf  - введённая строка.
 *****************************************************************************/

int32_t getString( char *buf, int32_t size, const char *mask, bool toup )
{
  int32_t i;
  int32_t len = 0;
  int32_t pos = 0;
  int32_t len_prev, pos_prev;

  while( 1 )
  {
    uint64_t keyCode = getKeyCode();                            // Ждём нажатия клавиши

    len_prev = len;                                             // Запоминаем текущую длину строки
    pos_prev = pos;                                             // Запоминаем текущую позицию

    switch( keyCode )
    {
      // ENTER - завершение ввода строки
      case KEY_ENTER:
      {
        buf[len] = 0;
        return len;
      }

      // Перемещение курсора в начало строки
      case KEY_HOME:
      {
        pos = 0;
        break;
      }

      // Перемещение курсора в конец строки
      case KEY_END:
      {
        pos = len;
        break;
      }

      // Перемещение курсора на одну позицию влево
      case KEY_LEFT:
      {
        if( pos > 0 )
          pos--;

        break;
      }

      // Перемещение курсора на одну позицию вправо
      case KEY_RIGHT:
      {
        if( pos < len )
          pos++;

        break;
      }

      // Удаление символа в текущей позиции
      case KEY_DELETE:
      {
        if( !len )                                              // Если строка пустая, то
          continue;                                             // ничего не делаем

        for( i=pos+1; i < len; i++ )                            // Удаляем символ в текущей позиции
          buf[i-1] = buf[i];

        len--;                                                  // Укорачиваем строку на один символ
        break;
      }

      // BACKSPACE - удаление предыдущего символа
      case KEY_BACKSPACE:
      {
        if( !pos )                                              // Если курсор стоит в самом начале строки, то
          continue;                                             // ничего не делаем.

        for( i=pos; i < len; i++ )                              // Удаляем предыдущий символ
          buf[i-1] = buf[i];

        pos--;                                                  // Переводим курсор на одну позицию левее
        len--;                                                  // Укорачиваем строку на один символ
        break;
      }

      // Все остальные функциональные клавиши игнорируем
      case KEY_ESC:
      case KEY_UP:
      case KEY_DOWN:
      case KEY_INSERT:
      case KEY_PAGEUP:
      case KEY_PAGEDN:
      case KEY_F1:
      case KEY_F2:
      case KEY_F3:
      case KEY_F4:
      case KEY_F5:
      case KEY_F6:
      case KEY_F7:
      case KEY_F8:
      case KEY_F9:
      case KEY_F10:
      case KEY_F11:
      case KEY_F12: break;

      // Обрабатываем все остальные символы
      default:
      {
        if( ( keyCode & 0xFF ) == 0x1B )                        // Игнорируем все остальные ESC-последовательности
          continue;

        if( toup )
          keyCode = toupper( keyCode );

        // Проверяем введённый символ на допустимость
        if( strchr( mask, keyCode ) == NULL )
          continue;

        if( len >= (size-1) )                                   // Проверяем наличие места во входном буфере:
          continue;                                             // ничего не делаем, если буфер заполнен до упора.

        if( pos == len )
        {                                                       // Если курсор стоит в конце строки, то
          buf[pos++] = keyCode;                                 // просто дописываем новый символ в конец строки
          len++;                                                // и инкрементируем длину строки.
          break;
        }
                                                                // Если же курсор стоит не в конце строки, то
        for( i=len; i > pos; i-- )                              // освобождаем место в строке под новый символ,
          buf[i] = buf[i-1];

        buf[pos++] = keyCode;                                   // вставляем новый символ в текущую позицию,
        len++;                                                  // инкрементируем длину строки.
      }
    }

    // Выводим изменённую строку

    for( i = pos_prev; i > 0; i-- )                             // Передвигаем курсор в начало строки
      printf( "\x08" );

    for( ; i < len; i++ )                                       // Печатаем новую строку
      printf( "%c", buf[i] );

    for( ; i < len_prev; i++ )                                  // Стираем хвостик, если строка стала короче
      printf( " " );

    for( ; i > pos; i-- )                                       // Возвращаем курсор в текущую позицию
      printf( "\x08" );

    fflush( stdout );
  }

  return 0;                                                     // Осчастливливаем Eclipse
}


/******************************************************************************
 * Ожидание нажатия клавиши Y или N.
 *
 * Выход: int - код нажатой клавиши.
 *****************************************************************************/

int getYN( void )
{
  int ch;

  while( 1 )
  {
    ch = toupper( getKeyCode() );

    if( ch == 'Y' || ch == 'N' )
      break;
  }

  return ch;
}
