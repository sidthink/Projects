#include "uart.h"




void UART_configuration(int serial_port, int baud_rate, int wait_time){
    /*termnios struct contains the UART config parameters like parity, stopbits, data size, etc
    struct termios {
	tcflag_t c_iflag;		// input mode flags 
	tcflag_t c_oflag;		// output mode flags 
	tcflag_t c_cflag;		// control mode flags 
	tcflag_t c_lflag;		// local mode flags 
	cc_t c_line;			// line discipline 
	cc_t c_cc[NCCS];		// control characters 
    ; */
    struct termios tty;

    if(tcgetattr(serial_port, &tty)!= 0){
        printf("Error %i from open: %s\n", errno, strerror(errno));
    }
    //UART configuration
    tty.c_cflag &= ~PARENB;         // no parity
    tty.c_cflag &= ~CSTOPB;          // one stop bit
    tty.c_cflag &= ~CSIZE;          // clear data size
    tty.c_cflag |= CS8;             // data packet size (8 bytes)
    tty.c_cflag &= ~CRTSCTS;        // clear RTS/CTS HW flow control
    tty.c_cflag |= CREAD | CLOCAL; // Turn on READ & ignore ctrl lines (CLOCAL = 1)
    tty.c_lflag &= ~ICANON; // canonical mode processes input line-by-line instead of one byte
    tty.c_lflag &= ~ECHO; // Disable echo
    tty.c_lflag &= ~ECHOE; // Disable erasure
    tty.c_lflag &= ~ECHONL; // Disable new-line echo
    tty.c_lflag &= ~ISIG; // Disable interpretation of INTR, QUIT and SUSP
    tty.c_iflag &= ~(IXON | IXOFF | IXANY); // Turn off s/w flow ctrl
    tty.c_iflag &= ~(IGNBRK|BRKINT|PARMRK|ISTRIP|INLCR|IGNCR|ICRNL); // Disable any special handling of received bytes
    tty.c_oflag &= ~OPOST; // Prevent special interpretation of output bytes (e.g. newline chars)
    tty.c_oflag &= ~ONLCR; // Prevent conversion of newline to carriage return/line feed
    // tty.c_oflag &= ~OXTABS; // Prevent conversion of tabs to spaces (NOT PRESENT IN LINUX)
    // tty.c_oflag &= ~ONOEOT; // Prevent removal of C-d chars (0x004) in output (NOT PRESENT IN LINUX)
    tty.c_cc[VTIME] = wait_time;    // Wait for up to 1s (10 deciseconds), returning as soon as any data is received.
    tty.c_cc[VMIN] = 0;

    // Set in/out baud rate to be 115200
    cfsetispeed(&tty, baud_rate);
    cfsetospeed(&tty, baud_rate);

    // Save tty settings, also checking for error
    if (tcsetattr(serial_port, TCSANOW, &tty) != 0) {
        printf("Error %i from tcsetattr: %s\n", errno, strerror(errno));
    }
    }