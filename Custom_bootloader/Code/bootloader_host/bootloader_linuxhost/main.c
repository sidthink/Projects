#include "uart.h"
#include "bl_commands.h"
#include "stdlib.h"

int main(){

    printf("+----------------------------------+\n");
    printf("+                                  +\n");
    printf("+------Serial Host Application-----+\n");
    printf("+                                  +\n");
    printf("+----------------------------------+\n");
    
    printf("\n");
    printf("\n");

    printf("Enter the COM port: ");
    char port[20];
    scanf("%s", port);
    printf("\n");
    //open serial port
    int serial_port = open(port, O_RDWR);   
    //checking for errors
    if(serial_port < 0){
        printf("Error %i from open: %s\n", errno, strerror(errno));
        return 0;
    }
    printf("Port opened, Connection established!\n");
    UART_configuration(serial_port, B115200, 100);

    printf("+----------------------------------+\n");
    printf("+                                  +\n");
    printf("+--------Supported Commands--------+\n");
    printf("+                                  +\n");
    printf("+  1.)     BL_GET_VER              +\n");
    printf("+----------------------------------+\n");
    while(1){
        printf("Enter the command no: ");
        int cmd;
        scanf("%d",&cmd);

        switch(cmd){
            case 0:
                    printf("Bye Bye!\n");
                    exit(0);
            case 1:
                    bl_get_version_func(serial_port);
                    break;
            default:
                    printf("Command not supported\n");
                    break;
        }
    }

    close(serial_port);

    return 1;

}






    /*
    //Serial port write
    unsigned char msg[] = { 'H', 'e', 'l', 'l', 'o', '\n','\r'};
    write(serial_port, msg, sizeof(msg));
    printf("Data sent\n");

    // Allocate memory for read buffer, set size according to your needs
    char read_buf [30];

    // Read bytes. The behaviour of read() (e.g. does it block?,
    // how long does it block for?) depends on the configuration
    // settings above, specifically VMIN and VTIME
    int n = read(serial_port, &read_buf, sizeof(read_buf));

    // n is the number of bytes read. n may be 0 if no bytes were received, and can also be negative to signal an error.


    //close port
    close(serial_port);

    printf("Data received from target: %s\n", read_buf);
    */