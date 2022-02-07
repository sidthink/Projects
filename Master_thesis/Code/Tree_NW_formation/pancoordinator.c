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
    //uint8_t input_rss_flag;
};
struct neighbor_msg{
  linkaddr_t addr;
  uint8_t nlayer;
};
struct message{
    uint8_t nlayer;
    uint8_t nghs; // no. of neighbours 
    uint8_t flag;
   // uint8_t input_rss_flag;
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

#define MAX_NEIGHBORS 4
MEMB(neighbors_memb, struct neighbor,MAX_NEIGHBORS);
LIST(neighbors_list);
static struct node node = {6,0,1,0};

int create_neighbor(const linkaddr_t *addr, struct process *pr){
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

//static uint8_t sizeflag = 0;
int update_neighbor_list(struct neighbor * ngh){
  struct neighbor *n;
  struct message *m;
  n = ngh;
  m = packetbuf_dataptr();
  if(m->nlayer >0 &&ngh->flag ==0){
    n->nlayer = m->nlayer;
    ngh->flag=1;
  }

  
   n->his_nghs = m->nghs;
  return 1;

}





/*---------------------------------------------------------------------------*/
PT_THREAD(pred_calc(struct pt *pt));
PROCESS(node_process1, "NLayer process");
PROCESS(prediction_process, "Prediction Process");
PROCESS(energest_process, " Energy Est Process");
AUTOSTART_PROCESSES(&prediction_process, &energest_process);
static struct ringbuffer rbuffer;
const linkaddr_t mtd={{0x00,0x12,0x4b,0x00,0x18,0xd6,0xf8,0x51}};
 linkaddr_t Inode={{0x00,0x12,0x4b,0x00,0x18,0xd6,0xf8,0x80}};


/*---------------------------------------------------------------------------*/
static void input(void){
  const linkaddr_t *src= packetbuf_addr(PACKETBUF_ADDR_SENDER);
  struct neighbor *n ;
 if(!linkaddr_cmp(src, &mtd)){ 
    n = check_neighbor(src);
    if(n)
      update_neighbor_list(n);
    
    if(n == NULL)
      create_neighbor(src,&node_process1);
  }
int i =1;
 for (n=list_head(neighbors_list); n!=NULL; n = list_item_next(n))
 {
   printf("Neighbour address [%d]:%02x%02x, Neural Layer:%d\n",i, n->addr.u8[6], n->addr.u8[7],n->nlayer );
   i++;
 }



}

static void output(int mac){

}

static void neural_input(void){
  const linkaddr_t *src = packetbuf_addr(PACKETBUF_ADDR_SENDER);
  printf("Src address :%02x%02x\n",  src->u8[6], src->u8[7]);

  if(linkaddr_cmp(src, &mtd)){
    if(node.input_rssi_flag==0){
      float temp;
      int8_t rssi;
      rssi = (int8_t)packetbuf_attr(PACKETBUF_ATTR_RSSI);
      temp = (float)(rssi*0.01);
      uint8_t ret = ringbuffer_put(&rbuffer, temp);
      if(ret==1){
        printf("Data added from mtd\n");
        if(node.input_size >1)
          node.input_rssi_flag=1;
      }
      //else
       // printf("Buffer is full\n");
   }
  }
  else{
        struct neighbor *n;
        n = check_neighbor(src);
        if(n->nlayer == node.nlayer -1){
           int8_t rss = *(int8_t *)packetbuf_dataptr();
           //neural_send(&rss, sizeof(int8_t), &output_ngh_addr);
            float temp = (float)(rss *0.01);
            uint8_t ret =ringbuffer_put(&rbuffer, temp);
            if(ret==1){}
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
        for(int j = 0; j < t.nips; j++)
            sum += in[j] * t.w[i * t.nips + j];
        t.h[i] = act(sum + t.b[0]);
        //printf("t.h[%d]:%d\n",i,(int)(100000 *t.h[i]));
    }
    // Calculate output layer neuron values.
    for(int i = 0; i < t.nops; i++)
    {
        float sum = 0.0f;
        for(int j = 0; j < t.nhid; j++)
            sum += t.h[j] * t.w[t.nhid*t.nips + i * t.nhid + j];
        t.o[i] = act(sum + t.b[1]);
        printf("Output[%d]:%d\n",i,(int)(100000 *t.o[i]));
    }
}

void xtpredict( struct Ann t, const float* const in)
{
    fprop(t, in);
    //return &(t.o);
}

void xtprint(const float* arr, const int size)
{
    for(int i = 0; i < size; i++)
        printf("%d ", (int)(100000* arr[i]));
    printf("\n");
}

NETSTACK_SNIFFER(e1, input, output);
NETSTACK_SNIFFER(neural, neural_input, neural_output);





PROCESS_THREAD(node_process1, ev, data)
{
  
  static struct etimer et1,et2;
  netstack_sniffer_add(&e1);
  tsch_is_coordinator =1;
  NETSTACK_RADIO.set_value( RADIO_PARAM_TXPOWER,-24);
  NETSTACK_MAC.on(); 
  static struct neighbor *t;
  PROCESS_BEGIN();
  printf("Starting up...\n");
  
  // Allow some delay for formation of network by TSCH MAC Layer //
  etimer_set(&et1, CLOCK_SECOND*95);
  etimer_set(&et2,CLOCK_SECOND*2);
  while(1){
    PROCESS_WAIT_EVENT_UNTIL(ev==PROCESS_EVENT_TIMER);
    if(etimer_expired(&et1)){
      printf("Stopping Nlayer process....\n");
      process_start(&prediction_process, NULL);
      PROCESS_EXIT();
    }
    for(t = list_head(neighbors_list);t!=NULL; t=list_item_next(t))
    {  
       struct message m;
        m.nlayer = node.nlayer;
        m.nghs = node.nghs;
        m.flag = 0;
        //m.input_rss_flag = node.input_rssi_flag;
       printf("IAm on layer: %d\n",node.nlayer);
        neural_send(&m,sizeof(struct message),&(t->addr)); 
      printf("Sending Datato :%02x%02x\n",t->addr.u8[6], t->addr.u8[7] );
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
  //static struct Ann t;
  //float bias[2]={-0.007537,-0.169036};
  static struct Ann t = {0,0,0,0,0,{-0.007537,-0.169036},{0.33609,
-0.07852,
-0.464002,
0.469069,
0.206722,
0.378299,
-0.506752,
0.230533,
-0.494572,
-0.360018,
-0.034595,
-0.449023,
0.147654,
0.425064,
0.123474,
-0.144686,
-0.394094,
0.195914,
0.283293,
-0.070292,
-0.131235,
0.370159,
-0.499291,
-0.309164,
0.008874,
0.076293,
-0.10916,
-0.360682,
-0.250327,
0.166031,
-0.514265,
-0.36538,
-0.370698
-0.449781,
-0.384651,
0.348828,
0.445617,
-0.344544,
0.086039,
-0.536341,
-0.204558,
-0.409903,
-0.465222,
-0.530075,
-0.461388,
0.163244,
-0.105709,
-0.306886,
-0.060655,
-0.295492,
0.147743,
0.322804,
-0.424226,
0.159126,
-0.424938,
0.172015,
-0.1959,
0.019535,
0.346223,
0.122659,
-0.261688,
0.375925,
0.29979,
-0.092023,
0.467896,
0.465736,
-0.204072,
0.466557,
-0.347414,
0.412704,
0.4489,
-0.024722,
-0.435806,
-0.466426,
-0.000951,
-0.40731,
0.253854,
0.406665,
-0.15896,
-0.298486,
-0.385544,
-0.495827,
-0.469041,
-0.264117,
0.168383,
-0.37449,
0.396704,
-0.498996,
0.145617,
0.249382,
0.105141,
0.391662,
0.110032,
-0.106419,
-0.216603,
0.098581,
-0.126909,
0.088109,
0.055508,
0.031704,
0.042894,
0.052883,
-0.437107,
0.124374,
0.114843,
0.085484,
0.31373,
-0.100357,
0.034385,
-0.321502,
0.132126,
0.184253,
-0.288456,
0.200126,
0.431956,
0.270942,
0.216553,
0.229144,
0.299595,
-0.079571,
0.020538,
-0.036741,
-0.167683,
-0.307368,
-0.565464,
0.175759,
-0.634925,
-0.179705,
-0.188562,
-0.009814,
-0.545573,
-0.527546,
-0.420086,
-0.483323,
0.119828,
0.250626,
0.169179,
-0.047931,
-0.148464,
-0.100998,
0.344006,
-0.436415,
-0.346753,
-0.389276,
0.295177,
-0.357235,
-0.425713,
0.224261,
-0.422983,
-0.445094,
-0.186652,
0.279139,
0.187329,
0.336442,
-0.326116,
0.269047,
0.196522,
-0.293428,
-0.22768,
-0.284381,
0.356831,
-0.512613,
-0.971252,
-0.617289,
-0.377096,
-0.582098,
-0.873285,
-0.600788,
-0.411356,
-1.373832,
-0.098269,
-0.073735,
-0.251502,
-0.809402,
0.735983
}, {0,0},{0,0,0,0,0,0,0}};

  for ( s=list_head(neighbors_list);s!=NULL;s=list_item_next(s)){
    if(s->nlayer == node.nlayer -1 ){
      node.input_size += s->his_nghs + 2;
    }
  }
  
  
  printf("Node input size: %d\n", node.input_size);
  ringbuffer_init(&rbuffer, node.input_size);
  rbuffer.size=23;
  rbuffer.tail=0;
 
  t.nips =23;
  t.nops = 2;
  t.nhid = 7;
  t.nb =2;
  t.nw = t.nhid*(t.nips + t.nops);
  
  /*for(uint8_t i=0; i<t.nb;i++){
    t.b[i]= trained_ann[i];
    //printf("Value of t.b[%d]:%d\n",i,(int)(1000000*t.b[i]));
  }
  for(uint8_t i=0; i<t.nw;i++){
    t.w[i]= trained_ann[t.nb+i];
    //printf("Value of t.w[%d]:%d\n",i,(int)(1000000*t.w[i]));
     }*/
 
  
  etimer_set(&et1, CLOCK_SECOND*10);

  printf(".....Waiting....\n");
  PT_WAIT_UNTIL(pt,etimer_expired(&et1));
  //etimer_set(&et2, CLOCK_SECOND*2);
 
  while(1){
    etimer_set(&et2, CLOCK_SECOND*0.5);
    PT_WAIT_UNTIL(pt, etimer_expired(&et2));
  //float temp1[1]= {-0.58};//, -0.072, -0.069};//, -0.062, -0.061 ,-0.067, -0.069, -0.07, -0.069 ,-0.063, -0.064, -0.07, -0.068, -0.074, -0.071, -0.063, -0.085, -0.075, -0.069, -0.065, -0.072, -0.079, -0.079};
  //for( uint8_t i =0; i< 3; i++)
   //rbuffer.buffer[i]= temp1[i];
 // rbuffer.tail =1;
   if( rbuffer.tail >= rbuffer.size){
  
     
     
       xtpredict(t,rbuffer.buffer);


     
      //printf("Buffer size :%d\n", rbuffer.size);
    // for( uint8_t i=0; i< rbuffer.size;i++)
     //   printf("Buffer[%d]:%d\n", i,(int)(1000*rbuffer.buffer[i]));
     rbuffer.tail=0;
     //tempbuf.tail =0;
      node.input_rssi_flag=0;
      linkaddr_set_node_addr(&Inode);
     

    }
      
        
    //etimer_restart(&et2);
 }

 
  PT_END(pt);
}

static inline unsigned long
to_seconds(uint64_t time)
{
  return (unsigned long)(time / ENERGEST_SECOND);
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

    /*
     * Update all energest times. Should always be called before energest
     * times are read.
     */
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
                      - energest_type_time(ENERGEST_TYPE_LISTEN)));*/
    
     printf(" ENERGEST CPU %llu LPM %llu DEEPLPM %llu Totaltime %llu LISTEN %llu TRANSMIT %llu SEC %d ID:%u\n",
         (energest_type_time(ENERGEST_TYPE_CPU)),
         (energest_type_time(ENERGEST_TYPE_LPM)),
         (energest_type_time(ENERGEST_TYPE_DEEP_LPM)),
         (ENERGEST_GET_TOTAL_TIME()),
         (energest_type_time(ENERGEST_TYPE_LISTEN)),
         (energest_type_time(ENERGEST_TYPE_TRANSMIT)),
        ENERGEST_SECOND, node_id);
  }

  PROCESS_END();
}