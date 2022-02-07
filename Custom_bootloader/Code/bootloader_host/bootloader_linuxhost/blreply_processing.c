#include "blreply_processing.h"
#include<unistd.h>

int bootloader_reply(int serial_port, unsigned char cmd_code){

    unsigned char ack[2] = {0};
    read(serial_port, ack, 2);

    if(ack[0]== 0xAA){
        
    }
}



