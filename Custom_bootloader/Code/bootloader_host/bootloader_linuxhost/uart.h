#ifndef __UART_H
#define __UART_H
//C headers
#include <stdio.h>
#include <string.h>
//linux headers
#include <fcntl.h> // contains file controls like 0_RDWR
#include <errno.h> // error integer and strerror() function
#include <unistd.h> //write(), read(), close() func
#include<termios.h>


void UART_configuration(int, int, int);


#endif