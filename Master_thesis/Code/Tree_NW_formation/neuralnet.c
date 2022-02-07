/*
 * Copyright (c) 2017, RISE SICS.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. Neither the name of the Institute nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE INSTITUTE AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE INSTITUTE OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 *
 * This file is part of the Contiki operating system.
 *
 */

/**
 * \file
 *         NullNet, a minimal network layer.
 * \author
 *         Simon Duquennoy <simon.duquennoy@ri.se>
 *
 */

/**
 * \addtogroup nullnet
 * @{
 */

#include "contiki.h"
#include "net/packetbuf.h"
#include "net/netstack.h"
#include "lib/memb.h"
#include "lib/list.h"
#include "neuralnet.h"

/* Log configuration */
#include "sys/log.h"
#define LOG_MODULE "NeuralNet"
#define LOG_LEVEL LOG_LEVEL_NULLNET

//uint8_t *nullnet_buf;
//uint16_t nullnet_len;

static uint8_t *packetbuf_ptr; 

static struct neural_callbacks *callback;



//static neuralnet_input_callback current_callback = NULL;
/*--------------------------------------------------------------------*/
static void
init(void)
{
  LOG_INFO("init\n");
  callback = NULL;
}
/*--------------------------------------------------------------------*/

static void
input(void)
{
    //const struct neural_callbacks *u;

    packetbuf_ptr = packetbuf_dataptr();

    if(packetbuf_datalen() == 0) {
    LOG_WARN("input: empty packet\n");
    return;
    }
    printf("Data received\n");
    callback->input_callbacks();

}
/*
  if(callback != NULL) {
    LOG_INFO("received %u bytes from ", packetbuf_datalen());
    LOG_INFO_LLADDR(packetbuf_addr(PACKETBUF_ADDR_SENDER));
    LOG_INFO_("\n");

    if(callback){
        callback->input_callback();
        //u->recv();
        
    }
   // current_callback(packetbuf_dataptr(), packetbuf_datalen(),
     // packetbuf_addr(PACKETBUF_ADDR_SENDER), packetbuf_addr(PACKETBUF_ADDR_RECEIVER));
  }
}
--------------------------------------------------------------------
void
neuralnet_set_input_callback(neuralnet_input_callback callback)
{
 current_callback = callback;
} */

void neural_open(struct neural_conn *n, struct neural_callbacks *c)
{
  n->u=c;
}

static void
packet_sent(void *ptr, int status, int transmissions)
{
  
}
/*--------------------------------------------------------------------*/
static uint8_t
output(const linkaddr_t *dest)
{
  //packetbuf_clear();
  //packetbuf_copyfrom(msg, sizeof(struct message));
  if(dest != NULL) {
    packetbuf_set_addr(PACKETBUF_ADDR_RECEIVER, dest);
  } else {
    packetbuf_set_addr(PACKETBUF_ADDR_RECEIVER, &linkaddr_null);
  }
  packetbuf_set_addr(PACKETBUF_ADDR_SENDER, &linkaddr_node_addr);
  LOG_INFO("sending %u bytes to ", packetbuf_datalen());
  LOG_INFO_LLADDR(packetbuf_addr(PACKETBUF_ADDR_RECEIVER));
  LOG_INFO_("\n");
  NETSTACK_MAC.send(&packet_sent, NULL);
  return 1;
}
/*--------------------------------------------------------------------*/
const struct network_driver neuralnet_driver = {
  "neuralnet",
  init,
  input,
  output
};
/*--------------------------------------------------------------------*/
/** @} */
