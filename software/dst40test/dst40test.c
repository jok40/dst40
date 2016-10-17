/******************************************************************************
 *
 * Программа для тестирования работы модуля DST40.
 *
 * Программа заточена под четырёхъядерный вариант схемы FPGA.
 *
 * Алгоритм:
 *
 * 1. Генерация случайных challenge и key.
 * 2. Расчёт response.
 * 3. Загрузка challenge, response и key в модуль DST40.
 * 4. Старт DST40.
 * 5. Ожидание завершения работы модуля DST40.
 * 6. Если модуль не нашёл ключ - выводим сообщение об ошибке и переходим на 1.
 * 7. Если модуль нашёл ключ, то считываем его и сравниваем с нашим.
 * 8. Если ключи не совпали, то выводим сообщение об ошибке и переходим на 1.
 * 9. Выводим время, прошедшее с момента старта программы.
 * 10. Опрашиваем клавиатуру.
 * 11. Если нажали ESC, то выходим из программы.
 * 12. Переходим на 1.
 *
 *----------------------------------------------------------------------------
 *
 * Адресная карта модуля DST40 (четырёхядерный вариант):
 *
 * 0 - challenge                ( 40 бит,  Чтение/Запись )  Первый запрос
 * 1 - response                 ( 24 бита, Чтение/Запись )  Первый ответ
 * 2 - start_key                ( 40 бит,  Чтение/Запись )  Ключ, с которого начинать поиск
 * 3 - run                      (  1 бит,  Чтение/Запись )  Флаг запуска поиска
 * 4 - флаги:
 *     бит 0 - key_found_w      (  1 бит,  Только чтение )  Флаг "ключ найден"
 *     бит 8 - key_not_found_w  (  1 бит,  Только чтение )  Флаг "ключ не найден"
 * 5 - key                      ( 38 бит,  Только чтение )  Найденный ключ (младшие биты)
 * 6 - kernels                  (  4 бита, Только чтение )  Биты ядер, нашедших ключ
 *
 *****************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <termios.h>
#include <time.h>
#include <sys/mman.h>
#include "hwlib.h"
#include "socal/socal.h"
#include "socal/hps.h"
#include "socal/alt_gpio.h"
#include <stdbool.h>
#include <signal.h>
#include <sys/time.h>
#include <wchar.h>
#include <locale.h>
#include <math.h>

uint64_t  dst40hash( uint64_t, uint64_t );

int kbhit( void );
int getch( void );



/******************************************************************************
 * Функция генерации случайного 40-битного числа
 *****************************************************************************/

uint64_t getRand40( void )
{
  uint64_t rnd40;

  rnd40 = ( (uint64_t)(rand() % 256) << 32 ) +
          ( (uint64_t)(rand() % 256) << 24 ) +
          ( (uint64_t)(rand() % 256) << 16 ) +
          ( (uint64_t)(rand() % 256) << 8  ) +
          ( (uint64_t)(rand() % 256)       );

  return rnd40;
}



/******************************************************************************
 * Преобразование старших бит ключа в маску бита ядра.
 *
 * Вход:  key - ключ,
 * Выход: Битовая маска ядра.
 *****************************************************************************/

uint64_t getMaskKernel( uint64_t key )
{
  uint64_t kernel = key >> 38;
  uint64_t mask = 1;

  for( ; kernel > 0; kernel-- )
    mask <<= 1;

  return mask;
}


int main( int argc, char** argv )
{
	void* h2f_base;
	int   fd;

	time_t time_start, time_now, time_prev;

  uint64_t challenge;
  uint64_t response;
  uint64_t key;

  uint64_t testCount = 0;
  uint64_t errCount = 0;

  // Флаги текущего состояния FPGA

  union
  {
    uint64_t Val;

    struct
    {
      uint8_t  key_found;
      uint8_t  key_not_found;
      uint32_t reserved1;
      uint8_t  reserved2;
    };
  } flags;

  // Инициализируем генератор случайных чисел.

  srand( time(NULL) );

  // Маппим регистры модуля DST40 в память

  if( ( fd = open( "/dev/mem", ( O_RDWR | O_SYNC ) ) ) == -1 )
  {
    perror( "\r\nERROR: could not open \"/dev/mem\"" );
    return( 1 );
  }

  h2f_base = mmap( NULL, 1024, ( PROT_READ | PROT_WRITE ), MAP_SHARED, fd, 0xC0000000 );

  if( h2f_base == MAP_FAILED )
  {
    perror( "\r\nERROR: mmap() failed" );
    close( fd );
    return(1);
  }

  printf( "\r\nPress ESC for exit\r\n\r\n" );
  fflush( stdout );

  // Настраиваем счётчики времени
  time_start = time( NULL );
  time_prev  = time_start;

  // Цикл тестирования работы FPGA
  while( 1 )
  {
    // Проверяем нажатие ESC на клавиатуре
    if( kbhit() && getch() == 27 )
    {
      printf( "\r\n\r\nKey ESC has been pressed.\r\n" );
      break;
    }

    // Генерим случайные запрос и ключ
    challenge = getRand40();
    key       = getRand40();

    // Вычисляем ответ
    response  = dst40hash( challenge, key );

    // Останавливаем FPGA
    alt_write_dword( h2f_base + 24, 0 );

    // Загружаем исходные данные в FPGA
    alt_write_dword( h2f_base +  0, challenge );
    alt_write_dword( h2f_base +  8, response );
    alt_write_dword( h2f_base + 16, key );

    // Разрешаем FPGA искать ключ
    alt_write_dword( h2f_base + 24, 1 );

    // Читаем флаги в цикле, пока какой-нибудь флаг не взведётся -
    // это приводит к полной загрузке одного ядра процессора.
    do
    {
      flags.Val = alt_read_dword( h2f_base + 32 );
    }
    while( !flags.Val );

    // Проверяем - нашёлся ли ключ
    if( flags.key_found )
    {
      // Считываем найденный ключ из FPGA
      uint64_t result = alt_read_dword( h2f_base + 40 );

      // Считываем биты ядер, нашедших ключ
      uint64_t kernels = alt_read_dword( h2f_base + 48 );

      // Вычисляем маску ядра, которое должно было найти ключ
      uint64_t mask = getMaskKernel( key );

      // Проверяем совпадение найденного ключа с исходным и совпадение номера ядра с требуемым
      if( result != (key & 0x3FFFFFFFFF) || (kernels & mask) == 0 )
      {
        printf( "\r\nError: Count=%lld, CHALLENGE=%010llX, RESPONSE=%06llX, KEY=%010llX, KEY_FPGA=%010llX, KERNELS=%lld\r\n", testCount, challenge, response, key, result, kernels );
        errCount++;
      }
    }
    else if( flags.key_not_found )
    {
      printf( "\r\nError: Key not found\r\n" );

      // Инкрементируем счётчик ошибок
      errCount++;
    }
    else
    {
      printf( "\r\nError: unknown interrupt\r\n" );

      // Инкрементируем счётчик ошибок
      errCount++;
      break;
    }

    testCount++;

    time_now = time( NULL ) - time_start;

    if( time_now != time_prev )
    {
      time_prev = time_now;
      printf( "\rTime: %ld | Tests: %lld | Errors: %lld", time_now, testCount, errCount );
      // wprintf( L"\rВремя: %ld. Выполнено проверок: %lld. Обнаружено ошибок: %lld", time_now, testCount, errCount );
      fflush( stdout );
    }
  }

  // Размапливаем регистры модуля DST40

  if( munmap( h2f_base, 1024 ) != 0 )
  {
    printf( "ERROR: munmap() failed...\n" );
    close( fd );
    return( 1 );
  }

  close( fd );

  return 0;
}
