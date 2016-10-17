// #include <iostream.h>
#include <sys/select.h>
#include <stddef.h>
#include <unistd.h>
#include <termios.h>
#include <stdio.h>
#include <stdbool.h>
#include <sys/ioctl.h>


/*
 * Проверка - нажата ли клавиша на клавиатуре
 */
/*
int kbhit( void )
{
  struct timeval tv;
  fd_set read_fd;

  // Do not wait at all, not even a microsecond
  tv.tv_sec=0;
  tv.tv_usec=0;

  // Must be done first to initialize read_fd
  FD_ZERO( &read_fd );

  // Makes select() ask if input is ready: 0 is the file descriptor for stdin
  FD_SET( 0, &read_fd );

  // The first parameter is the number of the largest file descriptor to check + 1.
  if( select( 1, &read_fd, NULL, NULL, &tv) == -1 )
    return 0;  // An error occured

  //  read_fd now holds a bit map of files that are readable. We test the entry for the standard input (file 0).
  if( FD_ISSET(0,&read_fd) )
    // Character pending on stdin
    return 1;

  // no characters were pending
  return 0;
}
*/

int kbhit( void )
{
  static const int STDIN = 0;
  static bool initialized = false;

  if (! initialized)
  {
    // Use termios to turn off line buffering
    struct termios term;

    tcgetattr( STDIN, &term );
    term.c_lflag &= ~ICANON;
    tcsetattr( STDIN, TCSANOW, &term );
    setbuf( stdin, NULL );
    initialized = true;
  }

  int bytesWaiting;

  ioctl( STDIN, FIONREAD, &bytesWaiting );
  return bytesWaiting;
}


/******************************************************************************
 * Ожидание нажатия клавиши - без вывода эха.
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
