/******************************************************************************
 *
 * Программа для поиска ключа DST40.
 *
 * Программа заточена под четырёхъядерный вариант модуля.
 *
 *----------------------------------------------------------------------------
 *
 * Аппаратная часть модуля DST40 соединена с HPS через мост HPS-to-FPGA.
 *
 * Адресная карта модуля DST40 (четырёхъядерный вариант):
 *
 * 0x00 - challenge                ( 40 бит,  Чтение/Запись )  Первый запрос
 * 0x08 - response                 ( 24 бита, Чтение/Запись )  Первый ответ
 * 0x10 - start_key                ( 40 бит,  Чтение/Запись )  Ключ, с которого начинать поиск
 * 0x18 - run                      (  1 бит,  Чтение/Запись )  Флаг запуска поиска
 * 0x20 - флаги:
 *        бит 0 - key_found_w      (  1 бит,  Только чтение )  Флаг "ключ найден"
 *        бит 8 - key_not_found_w  (  1 бит,  Только чтение )  Флаг "ключ не найден"
 * 0x28 - key                      ( 38 бит,  Только чтение )  Найденный ключ (младшие биты)
 * 0x30 - kernels                  (  4 бита, Только чтение )  Флаги ядер, нашедших ключ
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
#include "keyboard.h"


//#############################################################################
// ОПРЕДЕЛЕНИЯ

// Адреса регистров в схеме DST40

#define DST40_CHALLENGE   (_h2f_base+0)
#define DST40_RESPONSE    (_h2f_base+8)
#define DST40_START_KEY   (_h2f_base+16)
#define DST40_RUN         (_h2f_base+24)
#define DST40_FLAGS       (_h2f_base+32)
#define DST40_KEY         (_h2f_base+40)
#define DST40_KERNELS     (_h2f_base+48)



//#############################################################################
// ГЛОБАЛЬНЫЕ ПЕРЕМЕННЫЕ

int   _dst40_regs_file = 0;
int   _irq_ctrl_file = 0;
void* _h2f_base = 0;



/******************************************************************************
 * Callback-обработчик кода Ctrl+C - для корректного завершения
 * работы программы.
 *****************************************************************************/

void exitToLinux( int sig )
{
  if( sig != SIGINT )
    return;

  if( _h2f_base )
  {
    alt_write_dword( DST40_RUN, 0 );                            // Останавливаем FPGA

    if( munmap( _h2f_base, 1024 ) != 0 )                        // Размапливаем регистры модуля DST40
      printf( "\nERROR: munmap() failed...\n" );
  }

  if( _dst40_regs_file > 0 )                                    // Закрываем файл маппера,
    close( _dst40_regs_file );                                  // если он был открыт

  if( _irq_ctrl_file > 0 )                                      // Закрываем файл обработчика прерываний,
    close( _irq_ctrl_file );                                    // если он был открыт

  printf( "\n" );                                               // Переводим строку - чтобы приглашение вывелось в следующей строке
  echoOnOff( ECHO_ON );                                         // Переводим терминал в канонический режим работы
  exit( 0 );
}



/******************************************************************************
 * MAIN
 *
 * key        = 0000260000
 * challenge1 = 0000000001
 * response1  = 5CA1BA
 * challenge2 = 0000000002
 * response2  = 07F2C0
 *------------------------
 * key        = 7991F53219
 * challenge1 = 0000000001
 * response1  = CD6504
 * challenge2 = 0000000002
 * response2  = DF2F1D
 *
 *****************************************************************************/

int main( int argc, char** argv )
{
  bool  irq_enable = false;                                     // Признак наличия в системе драйвера обработчика прерываний IRQ0
	char  buf[20];

	time_t time_start, time_now;

  uint64_t c1, r1, c2, r2, start_key;
  uint64_t key1 = -1;
  uint64_t key2 = -1;
  uint64_t kernels1 = 0;
  uint64_t kernels2 = 0;
  uint8_t  data_set = 0;                                        // Идентификатор текущего набора данных (0 или FF)

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

  // Выключаем вывод нажатых клавиш в терминал
  echoOnOff( ECHO_OFF );

  // Устанавливаем свой обработчик нажатий Ctrl+C
  signal( SIGINT, exitToLinux );

  printf( "\n\nWARNING: Don't forget to load FPGA\n\nPress Ctrl+C for exit\n" );

  //------------------------------------------------------------//
  // Запрос входных данных

  while( 1 )
  {
    c1 = 0;
    r1 = 0;
    c2 = 0;
    r2 = 0;
    start_key = 0;

    printf( "\nType in first Challenge  (40-bit HEX-number): " );
    fflush( stdout );
    if( getString( buf, sizeof(buf), "0123456789ABCDEF", true ) )
      sscanf( buf, "%llX", &c1 );

    printf( "\nType in first Response   (24-bit HEX-number): " );
    fflush( stdout );
    if( getString( buf, sizeof(buf), "0123456789ABCDEF", true ) )
      sscanf( buf, "%llX", &r1 );

    printf( "\nType in second Challenge (40-bit HEX-number): " );
    fflush( stdout );
    if( getString( buf, sizeof(buf), "0123456789ABCDEF", true ) )
      sscanf( buf, "%llX", &c2 );

    printf( "\nType in second Response  (24-bit HEX-number): " );
    fflush( stdout );
    if( getString( buf, sizeof(buf), "0123456789ABCDEF", true ) )
      sscanf( buf, "%llX", &r2 );

    printf( "\nType in Start Key        (40-bit HEX-number): " );
    fflush( stdout );
    if( getString( buf, sizeof(buf), "0123456789ABCDEF", true ) )
      sscanf( buf, "%llX", &start_key );

    // Выводим результат ввода
    printf( "\n\nChallenge1 = %010llX", c1 );
    printf( "\nResponse1  = %06llX", r1 );
    printf( "\nChallenge2 = %010llX", c2 );
    printf( "\nResponse2  = %06llX", r2 );
    printf( "\nStart key  = %010llX", start_key );
    printf( "\n\nContinue? (Y/N) " );
    fflush( stdout );

    // Ждём нажатия Y или N - выходим из цикла, если нажали 'Y'
    if( getYN() == 'Y' )
      break;

    // Выводим "N" в терминал
    printf( "N\n\n" );
  }

  // Выводим "Y" в терминал
  printf( "Y\n\n" );

  //------------------------------------------------------------//
  // Поиск ключа                                                //

  // Открываем файл драйвера IRQ

  if( ( _irq_ctrl_file = open( "/dev/irq-ctrl", O_RDONLY ) ) == -1 )
    printf( "\nWARNING: IRQ-CTRL driver not found: Cyclic poll flags will be used\n" );
  else
    irq_enable = true;

  // Маппим регистры модуля DST40 в память

  if( ( _dst40_regs_file = open( "/dev/mem", ( O_RDWR | O_SYNC ) ) ) == -1 )
  {
    perror( "\nERROR: could not open \"/dev/mem\"\n" );
    exitToLinux( SIGINT );
  }

  _h2f_base = mmap( NULL, 1024, ( PROT_READ | PROT_WRITE ), MAP_SHARED, _dst40_regs_file, 0xC0000000 );

  if( _h2f_base == MAP_FAILED )
  {
    _h2f_base = 0;
    perror( "\nERROR: mmap() failed\n" );
    exitToLinux( SIGINT );
  }

  printf( "\n\nKey search has been started\n\n" );

  // Запоминаем время старта поиска
  time_start = time( NULL );

  // Начинаем поиск со стартового ключа
  key1 = start_key;

  // Останавливаем FPGA
  alt_write_dword( DST40_RUN, 0 );

  while( 1 )
  {
    uint64_t curr_key;

    // Загружаем исходные данные в FPGA
    if( !data_set )
    {
      alt_write_dword( DST40_CHALLENGE, c1   );
      alt_write_dword( DST40_RESPONSE,  r1   );
      alt_write_dword( DST40_START_KEY, key1 );
      curr_key = key1;
    }
    else
    {
      alt_write_dword( DST40_CHALLENGE, c2   );
      alt_write_dword( DST40_RESPONSE,  r2   );
      alt_write_dword( DST40_START_KEY, key2 );
      curr_key = key2;
    }

    // Разрешаем FPGA искать ключ
    alt_write_dword( DST40_RUN, 1 );

    // Выводим информацию о текущем ключе, времени и прогрессе в терминал
    time_now = time( NULL ) - time_start;
    printf( "\rCurrent KEY: %010llX [%lds] [%lld%%] ", curr_key, time_now, ((curr_key * 100) / 0x4000000000) );
    fflush( stdout );

    if( irq_enable )
    {
      // Засыпаем до момента прерывания
      read( _irq_ctrl_file, buf, 1 );

      // Считываем флаги из FPGA
      flags.Val = alt_read_dword( DST40_FLAGS );
    }
    else
    {
      // Читаем флаги в цикле, пока какой-нибудь флаг не взведётся -
      // это приводит к полной загрузке одного ядра процессора.
      do
      {
        flags.Val = alt_read_dword( DST40_FLAGS );
      }
      while( !flags.Val );
    }

    // Считываем текущий ключ и биты ядер из FPGA
    if( !data_set )
    {
      key2     = alt_read_dword( DST40_KEY );
      kernels2 = alt_read_dword( DST40_KERNELS );
    }
    else
    {
      key1     = alt_read_dword( DST40_KEY );
      kernels1 = alt_read_dword( DST40_KERNELS );
    }

    // Останавливаем FPGA
    alt_write_dword( DST40_RUN, 0 );

    // Выходим из цикла, если все ключи перебраны
    if( flags.key_not_found )
    {
      time_now = time( NULL ) - time_start;
      printf( "\rCurrent KEY: 3FFFFFFFFF [%lds] [100%%] ", time_now );
      printf( "\n\nKey not found\n\n" );
      exitToLinux( SIGINT );
    }

    // Если уже выполнены две проверки одного и того-же ключа
    // с разными парами запрос/ответ и оба раза ключ обнаружен
    // одним и тем-же ядром, то считаем ключ найденным и выходим.
    if( key1 == key2 && ( kernels1 & kernels2 ) != 0 )
    {
      uint64_t full_key;

      switch( kernels1 & kernels2 )
      {
        case 2:  full_key = 1;  break;
        case 4:  full_key = 2;  break;
        case 8:  full_key = 3;  break;
        default: full_key = 0;  break;
      }

      full_key = ( full_key << 38) | key1;

      printf( "\n\nKEY FOUND: %010llX\n\n", full_key );
      exitToLinux( SIGINT );
    }

    // Переключаемся на другой набор исходных данных
    data_set ^= 0xFF;
  }

  // Осчастливливаем Eclipse
  return 0;
}
