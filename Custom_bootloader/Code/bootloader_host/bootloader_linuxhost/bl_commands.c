#include "bl_commands.h"
#include <stdio.h>
#include <unistd.h>

void bl_get_version_func(int serial_port){

    unsigned char tx_buffer[10];
    unsigned char rx_buffer[10];
    int crc32;

    tx_buffer[0] = 0x01;
    tx_buffer[1] = 0x51;
    crc32 = get_crc(tx_buffer, 2);
    tx_buffer[2] = crc32 & 0x000000FF;
    tx_buffer[3] = crc32 & 0x0000FF00;
    tx_buffer[4] = crc32 & 0x00FF0000;
    tx_buffer[5] = crc32 & 0xFF000000;
    write(serial_port, tx_buffer, 1);
    write(serial_port, &tx_buffer[1], 5);

    int reply = read(serial_port,rx_buffer, 1);


}
int get_crc(char *buff, int len)

{
    int i;

    int Crc = 0XFFFFFFFF;

    for(int n = 0 ; n < len ; n++ )
    {
        int data = buff[n];
        Crc = Crc ^ data;
        for(i=0; i<32; i++)
        {

        if (Crc & 0x80000000)
            Crc = (Crc << 1) ^ 0x04C11DB7; // Polynomial used in STM32
        else
            Crc = (Crc << 1);
        }

    }

  return(Crc);
}
