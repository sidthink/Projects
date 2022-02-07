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
 * \ingroup net-layer
 * \addtogroup neuralnet
A network layer that does nothing. Useful for lower-layer testing and
for non-IPv6 scenarios.
 * @{
 */

#ifndef NEURALNET_H_
#define NEURALNET_H_

#include "contiki.h"
#include "net/linkaddr.h"
#include "net/packetbuf.h"
#include "net/queuebuf.h"

/**
 * Buffer used by the output function
*/
//extern uint8_t *neuralnet_buf;
//extern uint16_t neuralnet_len;

/**
 * Function prototype for NullNet input callback
*/
//typedef void (* neuralnet_input_callback)(const void *data, uint16_t len,
 // const linkaddr_t *src, const linkaddr_t *dest);



//void neural_open(const struct neural_callbacks *callbacks);
/**
 * Set input callback for NullNet
 *
 * \param callback The input callback
*/
//void neuralnet_set_input_callback(neuralnet_input_callback callback);

struct neural_conn{
  struct neural_callbacks *u;
};

struct neural_callbacks {
  /** Called when a packet has been received by the broadcast module. */
  void (* input_callbacks)(void);
};

void neural_open(struct neural_conn * n, struct neural_callbacks *c);
extern const struct network_driver neuralnet_driver;

//int neural_send(const linkaddr_t *dest);

#endif /* NEURALNET_H_ */
/** @} */
