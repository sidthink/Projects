#include "radioconf.h"
#include<stdio.h>

void print_radio_info(void)
{
  radio_value_t ch, ch_min, ch_max, txp, panid;   

  printf("\nRadio information:\n");
  //printf("  * %s\n", RF_STRING);

  

  if(NETSTACK_RADIO.get_value(RADIO_PARAM_CHANNEL, &ch) == RADIO_RESULT_OK) {
    printf("  * Channel %d ", ch);
  }

  if(NETSTACK_RADIO.get_value(RADIO_CONST_CHANNEL_MIN, &ch_min) ==
    RADIO_RESULT_OK) {
    printf("(%d-", ch_min);
  }

  if(NETSTACK_RADIO.get_value(RADIO_CONST_CHANNEL_MAX, &ch_max) ==
    RADIO_RESULT_OK) {
    printf("%d)\n", ch_max);
  }
     

  if(NETSTACK_RADIO.get_value(RADIO_PARAM_TXPOWER, &txp) == RADIO_RESULT_OK) {
    printf("  * TX power %d dBm [0x%04x]\n", txp, (uint16_t)txp);
  }

  if(NETSTACK_RADIO.get_value(RADIO_PARAM_PAN_ID, &panid) == RADIO_RESULT_OK) {
    printf("  * PAN ID %d \n", panid);
  }
}