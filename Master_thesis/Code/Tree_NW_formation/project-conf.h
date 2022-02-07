/*
 * Copyright (c) 2015, SICS Swedish ICT.
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
 */

/**
 * \author Simon Duquennoy <simonduq@sics.se>
 */

#ifndef PROJECT_CONF_H_
#define PROJECT_CONF_H_

/* Set to enable TSCH security */
#ifndef WITH_SECURITY
#define WITH_SECURITY 0
#endif /* WITH_SECURITY */

#undef UIP_CONF_TCP
#define UIP_CONF_TCP 0

#undef UIP_CONF_UDP
#define UIP_CONF_UDP 0

/* USB serial takes space, free more space elsewhere */
//#define SICSLOWPAN_CONF_FRAG 0
//#define UIP_CONF_BUFFER_SIZE 160

/*******************************************************/
/******************* Configure TSCH ********************/
/*******************************************************/

/* IEEE802.15.4 PANID */
#define IEEE802154_CONF_PANID 0x81a5

/* Do not start TSCH at init, wait for NETSTACK_MAC.on() */
#define TSCH_CONF_AUTOSTART 0


/* 6TiSCH minimal schedule length.
 * Larger values result in less frequent active slots: reduces capacity and saves energy. */
#define TSCH_SCHEDULE_CONF_DEFAULT_LENGTH 3

#undef NBR_TABLE_CONF_MAX_NEIGHBORS
#define NBR_TABLE_CONF_MAX_NEIGHBORS 4

#undef QUEUEBUF_CONF_NUM
#define QUEUEBUF_CONF_NUM 8

#undef TSCH_SCHEDULE_CONF_MAX_LINKS
#define TSCH_SCHEDULE_CONF_MAX_LINKS 50

#undef NETSTACK_MAX_ROUTE_ENTRIES
#define NETSTACK_MAX_ROUTE_ENTRIES 10

#define FRAME802154_CONF_VERSION FRAME802154_IEEE802154_2015
#if WITH_SECURITY

/* Enable security */
#define LLSEC802154_CONF_ENABLED 0

#endif /* WITH_SECURITY */

/*******************************************************/
/************* Platform dependent configuration ********/
/*******************************************************/

/* USB serial takes space, free more space elsewhere */
#define SICSLOWPAN_CONF_FRAG 0
#define UIP_CONF_BUFFER_SIZE 0

//#define UIP_CONF_IPV6_CHECKS 1

//#define SICSLOWPAN_CONF_COMPRESSION SICSLOWPAN_COMPRESSION_6LORH

/* Enables energest */
#define ENERGEST_CONF_ON 1

#define PROCESS_CONF_NO_PROCESS_NAMES 1
#undef LPM_CONF_ENABLE
#define LPM_CONF_ENABLE 1



/*******************************************************/
/************* Other system configuration **************/
/*******************************************************/

/* Logging */
#define LOG_CONF_LEVEL_RPL                         0// LOG_LEVEL_INFO
#define LOG_CONF_LEVEL_TCPIP                        0//LOG_LEVEL_WARN
#define LOG_CONF_LEVEL_IPV6                         0// LOG_LEVEL_WARN
#define LOG_CONF_LEVEL_6LOWPAN                      0// LOG_LEVEL_WARN
#define LOF_CONF_LEVEL_NULLNET                     0 // LOG_LEVEL_INFO
#define LOG_CONF_LEVEL_MAC                      0 // LOG_LEVEL_INFO
#define LOG_CONF_LEVEL_FRAMER                     0// LOG_LEVEL_INFO
#define TSCH_LOG_CONF_PER_SLOT                     0

#endif /* PROJECT_CONF_H_ */
