#include "contiki.h"
#include "sys/node-id.h"
#include "sys/log.h"
#include "net/mac/tsch/tsch.h"
#include "net/mac/tsch/tsch-queue.h"
#include "lib/memb.h"
#include "lib/list.h"
#include "net/nullnet/nullnet.h"
#include "math.h"
#include "sys/energest.h"
#include "os/sys/compower.h"
#define DEBUG DEBUG_PRINT

struct neighbor{
    struct neighbor *next;
    linkaddr_t addr;
    uint8_t nlayer;
    uint8_t his_nghs; // no. of neighbors below of the neighbor
    uint8_t flag;
};

struct message{
    uint8_t nlayer;
    uint8_t nghs; // no. of neighbours  
};
struct node{
    uint8_t nlayer;
    uint8_t nghs; // no. of neighbours 
    //uint8_t input_size;
    uint8_t input_rssi_flag;
};

#define MAX_NEIGHBORS 4
MEMB(neighbors_memb, struct neighbor,MAX_NEIGHBORS);
LIST(neighbors_list);
static struct node node = {0,0,0};
linkaddr_t output_ngh_addr;

int create_neighbor(const linkaddr_t *addr){
  struct neighbor *n = NULL;
   struct message *m;
    m= packetbuf_dataptr(); 
    n = memb_alloc(&neighbors_memb);
    if(n == NULL) {
      return 0;
      }

      /* Initialize the fields. */
      linkaddr_copy(&n->addr, addr);
      n->nlayer = m->nlayer;
      node.nghs ++;
      
      /* Place the neighbor on the neighbor list. */
      list_add(neighbors_list, n);
  
      return 1;
}
struct neighbor *check_neighbor(const linkaddr_t *addr){
  struct neighbor *n;
  for(n=list_head(neighbors_list);n!=NULL; n= list_item_next(n)){
    if(linkaddr_cmp(&n->addr, addr))
      return n;
    
  }
  return n;
}

int update_neighbor_list(struct neighbor * ngh){
  struct neighbor *n;
  struct message *m;
  n = ngh;
  m = packetbuf_dataptr();
  if(m->nlayer >0){
    n->nlayer = m->nlayer;
    ngh->flag=1;
  }
  
  n->his_nghs = m->nghs;
  if(n->nlayer > 25){
    list_remove(neighbors_list, n);
    memb_free(&neighbors_memb,n);
  }

  if( n->nlayer == node.nlayer+1 && node.nlayer>0)
      linkaddr_copy(&output_ngh_addr, &n->addr);
  else if(node.nlayer ==0 && n->nlayer > 0)
        linkaddr_copy(&output_ngh_addr, &n->addr);
  
  
  
  return 1;

}


/*---------------------------------------------------------------------------*/
PROCESS(nw_form_process, "NLayer process");
PROCESS(prediction_process, "Prediction Process");
PROCESS(energest_process, " Energy Est Process");
AUTOSTART_PROCESSES(&nw_form_process);

static uint8_t nflag=0;
const linkaddr_t mtd={{0x00,0x12,0x4b,0x00,0x18,0xd6,0xf3,0xf9}};
//const linkaddr_t mtd={{0x00,0x12,0x4b,0x00,0x18,0xd6,0xf8,0x51}};
linkaddr_t nodeId;

/*---------------------------------------------------------------------------*/
static void input(void){
  const linkaddr_t *src= packetbuf_addr(PACKETBUF_ADDR_SENDER);
   struct neighbor *n;
    struct message *m;
  if(!linkaddr_cmp(src, &mtd)){ 
    m = packetbuf_dataptr();
    n = check_neighbor(src);
    if(n)
      update_neighbor_list(n);
    
    if(n == NULL)
      create_neighbor(src);
    
  if(m->nlayer > node.nlayer ){
      node.nlayer= m->nlayer - 1;
      nflag=1;
  }
  }
  /*
 int8_t i =1;
 for (n=list_head(neighbors_list); n!=NULL; n = list_item_next(n))
 {
   printf("Neighbour address [%d]:%02x%02x, Neural Layer:%d\n",i, n->addr.u8[6], n->addr.u8[7],n->nlayer );
   i++;
 }*/



}

static void output(int mac){

}
struct nmsg{
  uint16_t counter;
  int8_t rss;
};
static uint16_t counter;
static void neural_input(void){
  const linkaddr_t *src = packetbuf_addr(PACKETBUF_ADDR_SENDER);
  counter++;
  if(linkaddr_cmp(src, &mtd)){
    int8_t rss;
    rss = (int8_t)packetbuf_attr(PACKETBUF_ATTR_RSSI);
    struct nmsg nm;
    nm.counter=counter;
    nm.rss = rss;
    neural_send(&nm, sizeof(struct nmsg), &output_ngh_addr);
    //printf( "Packet sent to: %02x%02x\n", output_ngh_addr.u8[6],output_ngh_addr.u8[7] );
   // printf("Output packet sent\n");
  }
  else{
    struct neighbor *n;
    n = check_neighbor(src);
    if(n->nlayer >-1){
        struct nmsg *nms,n_msg;
        nms = (struct nmsg *)packetbuf_dataptr();
        n_msg.rss = nms->rss;
        n_msg.counter=counter;
        neural_send(&n_msg, sizeof(struct nmsg), &output_ngh_addr);
        //printf( "Packet sent to: %02x%02x\n", output_ngh_addr.u8[6],output_ngh_addr.u8[7] );     
        }
  }
  
}

static void neural_output(int mac_status){

}



NETSTACK_SNIFFER(e1, input, output);
NETSTACK_SNIFFER(neural, neural_input, neural_output);


PROCESS_THREAD(nw_form_process, ev, data)
{
  static struct etimer et1, et2;
  
  static struct neighbor *t,*n; 
  NETSTACK_RADIO.set_value( RADIO_PARAM_TXPOWER,-7);
  tsch_is_coordinator =0;
  NETSTACK_MAC.on();
 
  netstack_sniffer_add(&e1);
  PROCESS_BEGIN();
  // Allow some delay for formation of network by TSCH MAC Layer //
  etimer_set(&et1, CLOCK_SECOND*300);
  etimer_set(&et2,CLOCK_SECOND*2);
  linkaddr_copy(&nodeId, &linkaddr_node_addr);
  while(1){
    PROCESS_WAIT_EVENT_UNTIL(ev==PROCESS_EVENT_TIMER);
     if(etimer_expired(&et1)){
      printf("Stopping Nlayer process....\n");
      int8_t i =1;
      for (n=list_head(neighbors_list); n!=NULL; n = list_item_next(n))
      {
        printf("Neighbour address [%d]:%02x%02x, Neural Layer:%d\n",i, n->addr.u8[6], n->addr.u8[7],n->nlayer );
        i++;
      }
      process_start(&prediction_process, NULL);
      process_start(&energest_process,NULL);
      PROCESS_EXIT();
    }
   for(t = list_head(neighbors_list);t!=NULL; t=list_item_next(t))
    {  
       struct message m;
        m.nlayer = node.nlayer;
        m.nghs = node.nghs;
        //printf("I'm on layer: %d\n",node.nlayer);
        neural_send(&m,sizeof(struct message),&(t->addr)); 
        //printf("Output Ngh:%02x%02x\n",output_ngh_addr.u8[6], output_ngh_addr.u8[7] );
    }
    
    etimer_restart(&et2);
  }
  PROCESS_END();
}
/*---------------------------------------------------------------------------*/

PROCESS_THREAD(prediction_process, ev, data)
{
  netstack_sniffer_remove(&e1);
  netstack_sniffer_add(&neural);
  static struct etimer et1, et2;
  //struct neighbor  *s;

  PROCESS_BEGIN();
/*
  for ( s=list_head(neighbors_list);s!=NULL;s=list_item_next(s)){
    if(s->nlayer == node.nlayer -1 ){
      node.input_size += s->his_nghs + 2;
    }
  }
  */
  
 // printf("Node input size: %d\n", node.input_size);
 
  
  etimer_set(&et1, CLOCK_SECOND*5);

  printf(".....Waiting....\n");
  PROCESS_WAIT_UNTIL(etimer_expired(&et1));
  //etimer_set(&et2, CLOCK_SECOND*2);
 
  while(1){
    etimer_set(&et2, CLOCK_SECOND);
    PROCESS_WAIT_UNTIL(etimer_expired(&et2));
   
    
   
  }
  PROCESS_END();
  
}

/*
 * This Process will periodically print energest values for the last minute.
 *
 */
PROCESS_THREAD(energest_process, ev, data)
{
  static struct etimer periodic_timer;

  PROCESS_BEGIN();

  etimer_set(&periodic_timer, CLOCK_SECOND * 10);
  while(1) {
    PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&periodic_timer));
    etimer_reset(&periodic_timer);

    
    energest_flush(); 
     printf("CPU,%llu LPM,%llu DEEPLPM,%llu Totaltime,%llu LISTEN,%llu TRANSMIT,%llu\n",
         (energest_type_time(ENERGEST_TYPE_CPU)),
         (energest_type_time(ENERGEST_TYPE_LPM)),
         (energest_type_time(ENERGEST_TYPE_DEEP_LPM)),
         (ENERGEST_GET_TOTAL_TIME()),
         (energest_type_time(ENERGEST_TYPE_LISTEN)),
         (energest_type_time(ENERGEST_TYPE_TRANSMIT)));
  }

  PROCESS_END();
}
