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

#define MAX_BUFFER_SIZE 67

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

struct xy_packet{
    uint8_t type;
    float data;
};
struct rss_packet{
    uint8_t type;
    int8_t data;
};


// type =1 for xy packet
//type =2 for rss_packet
struct node{
    uint8_t nlayer;
    uint8_t nghs; // no. of neighbours 
    uint8_t input_size;
    uint8_t input_rssi_flag;
};

struct packet_msg{
    uint8_t type;
    int8_t rss;
    float xy;
};
struct Ann{
  uint8_t nips, nops, nhid, nw, nb;
  float b[2];
  float w[175];
  float o[2];
  float h[2];
};


#define MAX_NEIGHBORS 4
MEMB(neighbors_memb, struct neighbor,MAX_NEIGHBORS);
LIST(neighbors_list);
static struct node node = {0,0,1,0};
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
  if(n->nlayer >25){
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
static struct ringbuffer rbuffer;//={0,0,0};
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
    
  if(m->nlayer > node.nlayer){
      node.nlayer= m->nlayer - 1;
      nflag =1;
  }
  }
 /*int i =1;
 for (n=list_head(neighbors_list); n!=NULL; n = list_item_next(n))
 {
   printf("Neighbour address [%d]:%02x%02x, Neural Layer:%d\n",i, n->addr.u8[6], n->addr.u8[7],n->nlayer );
   i++;
 }
*/


}

static void output(int mac){

}

void neural_send_toallngh(void *from, uint16_t len ){
  struct neighbor *n;
  for (n=list_head(neighbors_list);n!=NULL;n=list_item_next(n)){
      neural_send(from, len, &n->addr);
  }

}
static void neural_input(void){
  const linkaddr_t *src = packetbuf_addr(PACKETBUF_ADDR_SENDER);
  
  if(linkaddr_cmp(src, &mtd)){
    int8_t rss;
    rss = (int8_t)packetbuf_attr(PACKETBUF_ATTR_RSSI);
    struct rss_packet s;
    s.type =2;
    s.data = rss;
    neural_send_toallngh(&s, sizeof(struct rss_packet));
   // printf("Output packet sent\n");
    if(node.input_rssi_flag==0){
      float temp;
      temp = (float)(rss*0.01);
      uint8_t ret = ringbuffer_put(&rbuffer, temp);
      if(ret==1){
          if(node.input_size>1)
                node.input_rssi_flag=1;
      }
     // else
      //  printf("Buffer is full\n");
    }
  }
  else{
       struct neighbor *n;
       n = check_neighbor(src);
       if(n->nlayer >-1){
           struct packet_msg *pm;
           pm = (struct packet_msg *)packetbuf_dataptr();
           float temp;
           //uint8_t ret;
               if(pm->type==1){
                   temp=0.01* pm->xy;
                   ringbuffer_put(&rbuffer,temp);
               }
               else if(pm->type==2){
                    temp= (float)0.01* pm->rss;
                    ringbuffer_put(&rbuffer,temp);
                    struct rss_packet r;
                    r.type=2;
                    r.data = pm->rss;
                    neural_send_toallngh(&r, sizeof(struct rss_packet));
               }
           

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
  // printf("Output: ");
    // Calculate output layer neuron values.
    for(int i = 0; i < t.nops; i++)
    {
        float sum = 0.0f;
        for(int j = 0; j < t.nhid; j++)
            sum += t.h[j] * t.w[t.nhid*t.nips + i * t.nhid + j];
        t.o[i] = act(sum + t.b[1]);
       // printf("%d ",(int)(100000 *t.o[i]));
        struct xy_packet xy;
        xy.type=1;
        xy.data = t.o[i];
        neural_send_toallngh(&xy,sizeof(struct xy_packet));
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
  static struct etimer et1, et2;
  
  static struct neighbor *t ,*n; 
  NETSTACK_RADIO.set_value( RADIO_PARAM_TXPOWER,-24);
  tsch_is_coordinator =0;
  NETSTACK_MAC.on();
 
  netstack_sniffer_add(&e1);
  PROCESS_BEGIN();
  // Allow some delay for formation of network by TSCH MAC Layer //
  etimer_set(&et1, CLOCK_SECOND*100);
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
  static struct etimer et1, et2;
  struct neighbor  *s;

  PROCESS_BEGIN();
 static struct Ann t = {0,0,0,0,0,{0.302150,-0.389495},{-0.684120,
-0.901073,
-0.126499,
-0.624212,
-0.492144,
-1.599341,
-0.059590,
-0.152187,
-0.427146,
-0.020268,
-0.479511,
-0.381019,
-0.606145,
-0.065579,
-0.148622,
-0.530183,
0.156128,
-0.812358,
-0.504091,
-0.687534,
-0.546522,
-0.098574,
-0.027739,
0.196571,
0.356504,
0.378156,
0.492966,
-0.108659,
0.106986,
-0.139204,
-0.366971,
-0.372924,
0.048633,
0.334975,
-0.179465,
0.359930,
-0.378749,
-0.244722,
0.212474,
0.139328,
0.480430,
-0.426889,
-0.403637,
-0.299912,
0.472614,
0.257868,
0.042738,
0.410274,
0.357001,
0.286829,
0.533289,
-0.278911,
-0.055915,
0.277243,
0.364976,
-0.254279,
0.104216,
0.302742,
0.082225,
-0.342864,
-0.389147,
0.163061,
-0.266239,
0.108129,
-0.054957,
0.003923,
-0.087013,
-0.385212,
-0.068081,
-0.390918,
-0.030678,
-0.062934,
-0.211020,
-0.598284,
0.173634,
0.484542,
0.249676,
-0.048719,
-0.158405,
0.182532,
0.228275,
0.080732,
-0.185590,
0.105561,
-0.157554,
0.095613,
0.209846,
-0.293102,
0.109786,
-0.211436,
-0.228117,
-0.998814,
0.266845,
-1.023936,
0.528373,
-1.321770,
0.400602,
-1.090456,
0.117905,
-0.922588,
-0.090666,
-0.812267,
-0.369764,
-1.025121,
0.249338,
-0.616936,
-0.088301,
-0.934858,
0.572587,
-1.249579,
-0.366387,
-0.451300,
0.527471,
-0.576940,
-0.135730,
-0.925246,
-0.028507,
-0.574851,
0.122788,
-0.677080,
0.463304,
-0.672637,
-0.321190,
-0.838126,
-0.340773,
-1.366375,
0.201303,
-0.507627,
-0.091238,
-1.254677,
-0.074987,
-0.749866,
0.293967,
-1.013661,
-2.998331,
0.162616,
-0.691292,
-5.037154}, {0,0},{0,0}};


  for ( s=list_head(neighbors_list);s!=NULL;s=list_item_next(s)){
    if(s->nlayer == node.nlayer -1 ){
      node.input_size += s->his_nghs + 2;
    }
  }
  
  
  //printf("Node input size: %d\n", node.input_size);
  //ringbuffer_init(&rbuffer, node.input_size);
  rbuffer.size=67;
  rbuffer.tail=0;
 
  t.nips =67;
  t.nops = 2;
  t.nhid = 2;
  t.nb =2;
  t.nw = t.nhid*(t.nips + t.nops);
  
 
  
  etimer_set(&et1, CLOCK_SECOND*10);

  printf(".....Waiting....\n");
  PROCESS_WAIT_UNTIL(etimer_expired(&et1));
  //etimer_set(&et2, CLOCK_SECOND*2);
   etimer_set(&et2, CLOCK_SECOND*0.1);
  while(1){
    PROCESS_WAIT_UNTIL(etimer_expired(&et2));
   // float temp1[1]= {-0.067};//, -0.072, -0.069};//, -0.062, -0.061 ,-0.067, -0.069, -0.07, -0.069 ,-0.063, -0.064, -0.07, -0.068, -0.074, -0.071, -0.063, -0.085, -0.075, -0.069, -0.065, -0.072, -0.079, -0.079};
   //for( uint8_t i =0; i< 3; i++)
   //  rbuffer.buffer[i]= temp1[i];
   // rbuffer.tail =1;
   if( rbuffer.tail >= rbuffer.size){
  
     
     
       xtpredict(t,rbuffer.buffer);

    linkaddr_set_node_addr(&nodeId);
     
      //printf("Buffer size :%d\n", rbuffer.size);
    // for( uint8_t i=0; i< rbuffer.size;i++)
     //   printf("Buffer[%d]:%d\n", i,(int)(1000*rbuffer.buffer[i]));
     rbuffer.tail=0;
     //tempbuf.tail =0;
      node.input_rssi_flag=0;
      
      etimer_restart(&et2);
    }
  }
  PROCESS_END();
  
}
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