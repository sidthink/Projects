//  print_radio_info();
 /* static const output_config_t output_power[] = {
  {  7, 0xFF },
  {  5, 0xED },
  {  3, 0xD5 },
  {  1, 0xC5 },
  {  0, 0xB6 },
  { -1, 0xB0 },
  { -3, 0xA1 },
  { -5, 0x91 },
  { -7, 0x88 },
  { -9, 0x72 },
  {-11, 0x62 },
  {-13, 0x58 },
  {-15, 0x42 },
  {-24, 0x00 },
};*/

#include "contiki.h"
#include "sys/node-id.h"
//#include "sys/log.h"
#include "net/mac/tsch/tsch.h"
#include "lib/memb.h"
#include "lib/list.h"
#include "net/nullnet/nullnet.h"
#include "math.h"
#include <string.h>
#include "sys/energest.h"
#include "os/sys/compower.h"
//#define DEBUG DEBUG_PRINT

#define MAX_BUFFER_SIZE 23

struct ringbuffer {
  uint8_t size, tail;
  float buffer[MAX_BUFFER_SIZE];
};

void ringbuffer_init(struct ringbuffer *r, uint8_t buf_size){
  r->size = buf_size;
  r->tail = 0;
} 

uint8_t ringbuffer_put(struct ringbuffer *r, float value){
  if( r->tail >= r->size)
    return 0;
  else{
    r->buffer[r->tail]= value;
    r->tail ++;
    return 1;
    
  }
}
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
    uint8_t input_size;
    uint8_t input_rssi_flag;
};

struct Ann{
  uint8_t nips, nops, nhid, nw, nb;
  float b[2];
  float w[175];
  float o[2];
  float h[7];
};

static uint16_t predictions;
#define MAX_NEIGHBORS 4
MEMB(neighbors_memb, struct neighbor,MAX_NEIGHBORS);
LIST(neighbors_list);
static struct node node = {24,0,1,0};

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

  if(n->nlayer > 25){
    list_remove(neighbors_list, n);
    memb_free(&neighbors_memb,n);
  }

  
   n->his_nghs = m->nghs;
  return 1;

}





/*---------------------------------------------------------------------------*/
PT_THREAD(pred_calc(struct pt *pt));
PROCESS(nw_form_process, "NLayer process");
PROCESS(prediction_process, "Prediction Process");
PROCESS(energest_process, " Energy Est Process");
PROCESS(prediction_calc_process, "Prediction_Calc");
AUTOSTART_PROCESSES(&nw_form_process);
//AUTOSTART_PROCESSES(&prediction_process, &prediction_calc_process,&energest_process);//, &energest_process);
static struct ringbuffer rbuffer;
const linkaddr_t mtd={{0x00,0x12,0x4b,0x00,0x18,0xd6,0xf3,0xf9}};
//const linkaddr_t mtd={{0x00,0x12,0x4b,0x00,0x18,0xd6,0xf8,0x51}};
 linkaddr_t Inode;//={{0x00,0x12,0x4b,0x00,0x18,0xd6,0xf8,0x80}};


/*---------------------------------------------------------------------------*/
static void input(void){
  const linkaddr_t *src= packetbuf_addr(PACKETBUF_ADDR_SENDER);
  struct neighbor *n ;
 if(!linkaddr_cmp(src, &mtd)){ 
    n = check_neighbor(src);
    if(n)
      update_neighbor_list(n);
    
    if(n == NULL){
      create_neighbor(src);}
/*
      int8_t i =1;
        for (n=list_head(neighbors_list); n!=NULL; n = list_item_next(n))
        {
        printf("Neighbour address [%d]:%02x%02x, Neural Layer:%d\n",i, n->addr.u8[6], n->addr.u8[7],n->nlayer );
        i++;
        }*/
  }

}

static void output(int mac){

}
uint16_t rx_packet_mtd;
uint16_t rx_packet_ngh;
static uint64_t prr_mtd, prr_ngh;
struct nmsg{
  uint16_t counter;
  int8_t rss;
};
static void neural_input(void){
  const linkaddr_t *src = packetbuf_addr(PACKETBUF_ADDR_SENDER);
  //printf("Src address :%02x%02x\n",  src->u8[6], src->u8[7]);

  if(linkaddr_cmp(src, &mtd)){
    
    rx_packet_mtd++;
    uint16_t tx_expected;
    tx_expected = *(uint16_t *)packetbuf_dataptr();
      if(tx_expected == 1) {
      prr_mtd = 0;
      rx_packet_mtd = 1;
    }
    prr_mtd = rx_packet_mtd * 1000;
    prr_mtd /= tx_expected;
    printf("Mtd,%llu.%llu\n", prr_mtd / 10, prr_mtd % 10);
    if(node.input_rssi_flag==0){
      float temp;
      int8_t rssi; 
      rssi = (int8_t)packetbuf_attr(PACKETBUF_ATTR_RSSI);
      temp = (float)(rssi*0.01);
      uint8_t ret = ringbuffer_put(&rbuffer, temp);
      if(ret==1){
       // printf("Data added from mtd\n");
        if(node.input_size >1)
          node.input_rssi_flag=1;
      }
   }
  }
  else{
        struct neighbor *n;
        n = check_neighbor(src);
        if(n->nlayer > -1){
            
            rx_packet_ngh++;
            uint16_t tx_expected;
            struct nmsg *nm;
            nm = (struct nmsg *)packetbuf_dataptr();
            tx_expected = nm->counter;
            if(tx_expected == 1) {
              prr_ngh = 0;
              rx_packet_mtd = 1;
            }
            prr_ngh = rx_packet_ngh * 1000;
            prr_ngh /= tx_expected;
            printf("Ngh,%llu.%llu\n", prr_ngh / 10, prr_ngh % 10);
           // struct nmsg *nm;
            nm = (struct nmsg *)packetbuf_dataptr();
           int8_t rss = nm->rss;
           float temp = (float)(rss *0.01);
           ringbuffer_put(&rbuffer, temp);
            
               // printf("Data added from neighbour\n");
           // else
              //  printf("Buffer is full\n");              
          }
      }
}

static void neural_output(int mac_status){

}

static float act(const float a)
{
    return 1.0f / (1.0f + expf(-a));
}

static void fprop( struct Ann t, const float* const in)
{
    // Calculate hidden layer neuron values.
    for(int i = 0; i < t.nhid; i++)
    {
        float sum = 0.0f;
        for(int j = 0; j < t.nips; j++){
           //printf("in[%d]:%d, t.w[%d]:%d\n",j,(int)(100*in[j]),i * t.nips + j,(int)(100000*t.w[i * t.nips + j]));
            sum += in[j] * t.w[i * t.nips + j];
        }
        //printf("Sum I->H: %d\n",(int)(100000* sum));
        t.h[i] = act(sum + t.b[0]);
        //printf("t.h[%d]:%d\n",i,(int)(100000 *t.h[i]));
    }
    //printf("Output: ");
    // Calculate output layer neuron values.
    for(int i = 0; i < t.nops; i++)
    {
        float sum = 0.0f;
        for(int j = 0; j < t.nhid; j++)
            sum += t.h[j] * t.w[t.nhid*t.nips + i * t.nhid + j];
       // printf("Sum H->O: %d\n",(int)(100000* sum));
        t.o[i] = act(sum + t.b[1]);
       // printf("%d ",(int)(100000 *t.o[i]));
    }
    //printf("\n");
}

void xtpredict( struct Ann t, const float* const in)
{
    fprop(t, in);
    //return &(t.o);
}


NETSTACK_SNIFFER(e1, input, output);
NETSTACK_SNIFFER(neural, neural_input, neural_output);

PROCESS_THREAD(nw_form_process, ev, data)
{
  
  static struct etimer et1,et2;
  netstack_sniffer_add(&e1);
  tsch_is_coordinator =1;
  NETSTACK_RADIO.set_value( RADIO_PARAM_TXPOWER,-7);
  NETSTACK_MAC.on(); 
  static struct neighbor *t,*n;
  PROCESS_BEGIN();
  printf("Starting up...\n");
  linkaddr_copy(&Inode, &linkaddr_node_addr);
  
  // Allow some delay for formation of network by TSCH MAC Layer //
  etimer_set(&et1, CLOCK_SECOND*240);
  etimer_set(&et2,CLOCK_SECOND*2);
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
     // process_start(&prediction_calc_process,NULL);
      process_start(&energest_process,NULL);
      PROCESS_EXIT();
    }
    for(t = list_head(neighbors_list);t!=NULL; t=list_item_next(t))
    {  
       struct message m;
        m.nlayer = node.nlayer;
        m.nghs = node.nghs;
       //printf("IAm on layer: %d\n",node.nlayer);
        neural_send(&m,sizeof(struct message),&(t->addr)); 
      //printf("Sending Datato :%02x%02x\n",t->addr.u8[6], t->addr.u8[7] );
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
  static struct etimer et;
  static struct pt pred_pt;
  
 
  
   PROCESS_BEGIN();
   tsch_is_coordinator = 1;
    NETSTACK_MAC.on();
    etimer_set(&et, CLOCK_SECOND*1); 
    while(1){
        PROCESS_WAIT_UNTIL(etimer_expired(&et));

        PROCESS_PT_SPAWN(&pred_pt, pred_calc(&pred_pt));

    }
  
 PROCESS_END();
}
PT_THREAD(pred_calc(struct pt *pt))
{
  PT_BEGIN(pt);

  static struct etimer et1,et2;
  
  struct neighbor *s;
  static struct Ann t = {0,0,0,0,0,{0.084156,0.342209},{0.637596,
0.087239,
0.293925,
-0.594525,
-0.085485,
-0.183497,
0.135362,
0.298296,
0.261404,
-0.071524,
0.440359,
0.047956,
0.390759,
0.281943,
-0.489824,
-0.722409,
-0.470928,
0.254666,
-0.602329,
0.396405,
0.082230,
0.282062,
0.095236,
-1.041863,
0.332757,
-2.115957,
0.694127,
0.641578,
-0.409867,
-0.957269,
1.087663,
0.133963,
0.904705,
-0.616705,
-1.195678,
0.000241,
-0.200530,
1.395435,
0.677977,
0.744209,
0.497359,
-0.371408,
0.757069,
-0.212785,
-0.594150,
-0.854336,
0.597389,
-0.288489,
0.610973,
0.474295,
-0.058218,
0.150813,
-0.224560,
0.213548,
0.427403,
-0.106182,
0.581222,
-0.456780,
0.034227,
0.697556,
-0.644365,
-0.966402,
-0.524525,
0.476578,
-0.301729,
0.170049,
-0.091855,
0.204916,
-0.867216,
-0.185560,
0.182465,
0.320173,
0.472204,
-0.047864,
-0.001343,
-0.503901,
0.367371,
-0.457674,
0.305765,
0.001748,
-0.156251,
0.053971,
0.407097,
-0.951681,
-0.215492,
0.236717,
0.966586,
-0.291432,
0.205651,
-0.453056,
0.276238,
-0.793032,
-1.031599,
0.414406,
-1.039470,
0.444464,
0.432511,
0.290909,
-0.166494,
0.453555,
-0.277683,
0.931856,
-0.325532,
-0.032907,
-0.448568,
0.066423,
0.196028,
0.664373,
0.634438,
0.481111,
-0.466782,
0.527223,
-0.426627,
-0.738084,
-0.627033,
-0.171188,
-0.076263,
0.575281,
0.375502,
-0.538438,
-0.215906,
-1.001673,
-0.080125,
-0.420809,
0.008711,
0.441735,
-0.318557,
0.064980,
0.251841,
-0.717576,
-1.046816,
0.113877,
0.703291,
-0.353830,
0.539530,
0.451361,
0.250448,
-0.495745,
0.487655,
-0.278344,
-0.202017,
0.092807,
-0.311491,
0.533916,
-0.687393,
0.021521,
0.476247,
-0.267791,
-0.279754,
-0.608667,
-0.176694,
-0.227095,
-0.483201,
-0.563844,
-0.052422,
0.337644,
0.070042,
1.194434,
0.024187,
0.313782,
-0.229060,
-1.341615,
1.384192,
-2.223683,
-1.859800,
0.858266,
-2.092505,
-1.541960,
0.245901,
-3.360588,
-0.293990,
-0.828935,
-1.657270,
-0.480992,
-1.022639 }, {0,0},{0,0,0,0,0,0,0}};//Global_Iter=580,saved=9

  for ( s=list_head(neighbors_list);s!=NULL;s=list_item_next(s)){
    if(s->nlayer == node.nlayer -1 ){
      node.input_size += s->his_nghs + 2;
    }
  }
  
  
 // printf("Node input size: %d\n", node.input_size);
  rbuffer.size=23;
  rbuffer.tail=0;
  t.nips =23;
  t.nops = 2;
  t.nhid = 7;
  t.nb =2;
  t.nw = t.nhid*(t.nips + t.nops);
  etimer_set(&et1, CLOCK_SECOND*10);

  printf(".....Waiting....\n");
  PT_WAIT_UNTIL(pt,etimer_expired(&et1));
  //etimer_set(&et2, CLOCK_SECOND*2);
  etimer_set(&et2, CLOCK_SECOND*0.1);
  while(1){
    PT_WAIT_UNTIL(pt, etimer_expired(&et2));
  //float temp1[23]= {-0.58, -0.60,-0.72,-0.56,-0.58,-0.60,-0.72,-0.56,-0.58,-0.60,-0.72,-0.58,-0.60,-0.71,-0.56,-0.58,-0.60,-0.72,-0.56,-0.58,-0.61,-0.72,-0.56};
  //for( uint8_t i =0; i< 23; i++)
   //rbuffer.buffer[i]= temp1[i];
  //rbuffer.tail =23;
   if( rbuffer.tail >= rbuffer.size){
  
     
     
       xtpredict(t,rbuffer.buffer);
       predictions++;


     
      //printf("Buffer size :%d\n", rbuffer.size);
    //for( uint8_t i=0; i< rbuffer.size;i++)
      //  printf("Buffer[%d]:%d\n", i,(int)(100*rbuffer.buffer[i]));
     rbuffer.tail=0;
     //tempbuf.tail =0;
      node.input_rssi_flag=0;
      linkaddr_set_node_addr(&Inode);
     

    }
      
        
    etimer_restart(&et2);
 }

 
  PT_END(pt);
}
/*---------------------------------------------------------------------------*/
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
/*
    printf("\nEnergest:\n");
    printf(" CPU          %4lus LPM      %4lus DEEP LPM %4lus  Total time %lus\n",
           to_seconds(energest_type_time(ENERGEST_TYPE_CPU)),
           to_seconds(energest_type_time(ENERGEST_TYPE_LPM)),
           to_seconds(energest_type_time(ENERGEST_TYPE_DEEP_LPM)),
           to_seconds(ENERGEST_GET_TOTAL_TIME()));
    printf(" Radio LISTEN %4lus TRANSMIT %4lus OFF      %4lus\n",
           to_seconds(energest_type_time(ENERGEST_TYPE_LISTEN)),
           to_seconds(energest_type_time(ENERGEST_TYPE_TRANSMIT)),
           to_seconds(ENERGEST_GET_TOTAL_TIME()
                      - energest_type_time(ENERGEST_TYPE_TRANSMIT)
                      - energest_type_time(ENERGEST_TYPE_LISTEN)));
  */  
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

PROCESS_THREAD(prediction_calc_process, ev, data)
{
  static struct etimer et;
  PROCESS_BEGIN();
  etimer_set(&et, CLOCK_SECOND*60);
  while(1){
    PROCESS_WAIT_UNTIL(etimer_expired(&et));
    printf("%lu,%d\n", clock_seconds(), predictions);
    etimer_restart(&et);
  }
  PROCESS_END();
}
