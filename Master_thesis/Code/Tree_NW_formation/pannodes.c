#include "contiki.h"
#include "sys/node-id.h"
#include "sys/log.h"
#include "net/mac/tsch/tsch.h"
#include "net/mac/tsch/tsch-queue.h"
#include "lib/memb.h"
#include "lib/list.h"
#include "net/nullnet/nullnet.h"
#include "math.h"
#define DEBUG DEBUG_PRINT

#define MAX_BUFFER_SIZE 25

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
    //uint8_t input_rss_flag;
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
  float h[2];
};


#define MAX_NEIGHBORS 4
MEMB(neighbors_memb, struct neighbor,MAX_NEIGHBORS);
LIST(neighbors_list);
static struct node node = {0,0,1,0};
linkaddr_t output_ngh_addr;

int create_neighbor(const linkaddr_t *addr, struct process *pr){
  struct neighbor *n = NULL;
   struct message *m;
    m= packetbuf_dataptr();
  if(m->nlayer < 10){
   
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
  }
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
  if(m->nlayer >0 &&ngh->flag ==0){
    n->nlayer = m->nlayer;
    ngh->flag=1;
  }
  
  n->his_nghs = m->nghs;
  if(n->nlayer == 0 || n->nlayer>10){
    list_remove(neighbors_list, n);
    memb_free(&neighbors_memb,n);
  }

  if( n->nlayer == node.nlayer+1)
      linkaddr_copy(&output_ngh_addr, &n->addr);

  return 1;

}


/*---------------------------------------------------------------------------*/
PROCESS(node_process1, "NLayer process");
PROCESS(prediction_process, "Prediction Process");
AUTOSTART_PROCESSES(&node_process1);

static uint8_t nflag=0;
static struct ringbuffer rbuffer;//={0,0,0};
const linkaddr_t mtd={{0x00,0x12,0x4b,0x00,0x18,0xd6,0xf8,0x51}};
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
      create_neighbor(src,&node_process1);
    
  if(m->nlayer > node.nlayer && nflag==0){
      node.nlayer= m->nlayer - 1;
      nflag =1;
  }
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
  
  if(linkaddr_cmp(src, &mtd)){
    int8_t rss;
    rss = (int8_t)packetbuf_attr(PACKETBUF_ATTR_RSSI);
    neural_send(&rss, sizeof(int8_t), &output_ngh_addr);
   // printf("Output packet sent\n");
    if(node.input_rssi_flag==0){
      float temp;
      temp = (float)(rss*0.01);
      uint8_t ret = ringbuffer_put(&rbuffer, temp);
      if(ret==1)
        node.input_rssi_flag=1;
      else
        printf("Buffer is full\n");
    }
  }
  else{
       struct neighbor *n;
       n = check_neighbor(src);
       if(n->nlayer == node.nlayer -1){
           int8_t rss = *(int8_t *)packetbuf_dataptr();
           neural_send(&rss, sizeof(int8_t), &output_ngh_addr);
              float temp = (float)(rss *0.001);
              uint8_t ret =ringbuffer_put(&rbuffer, temp);
              if(ret ==0)
                printf("Buffer is full\n");
                  
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
        neural_send(&t.o[i], sizeof(float),&output_ngh_addr);
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
  static struct etimer et1, et2;
  
  static struct neighbor *t; 
  NETSTACK_RADIO.set_value( RADIO_PARAM_TXPOWER,-24);
  tsch_is_coordinator =0;
  NETSTACK_MAC.on();
 
  netstack_sniffer_add(&e1);
  PROCESS_BEGIN();
  // Allow some delay for formation of network by TSCH MAC Layer //
  etimer_set(&et1, CLOCK_SECOND*100);
  etimer_set(&et2,CLOCK_SECOND*10);
  linkaddr_copy(&nodeId, &linkaddr_node_addr);
  while(1){
    PROCESS_WAIT_EVENT_UNTIL(ev==PROCESS_EVENT_MSG || ev==PROCESS_EVENT_TIMER);
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
  static struct etimer et1, et2;
  struct neighbor  *s;

  PROCESS_BEGIN();

   static struct Ann t;
  float *trained_ann;
  float trained_ann1[8] ={0.150980,-0.413570,-1.942550,1.406715,-6.850386,-6.135756,-1.617947,-4.996579};                         
  float trained_ann4[14]={0.228282,0.188072,1.226243,-0.215877,0.719876,-0.149299,-2.481379,-0.076731,-2.903281,-0.872582,-4.594995,-6.649438,-3.948446,-2.287645};       
  float trained_ann7[20] = {0.095956,-0.288781,-3.645298,-2.624871,-1.362379,-0.637902,-3.868870,-3.999546,-0.910780,-1.798960,-0.713678,-0.380873,0.267602,-1.070891,-2.073524,-1.002129,-4.321589,-4.139883,-3.591597,0.667894};

  for ( s=list_head(neighbors_list);s!=NULL;s=list_item_next(s)){
    if(s->nlayer == node.nlayer -1 ){
      node.input_size += s->his_nghs + 2;
    }
  }
  
  
  printf("Node input size: %d\n", node.input_size);
  ringbuffer_init(&rbuffer, node.input_size);
  rbuffer.size=node.input_size;
  rbuffer.tail=0;
 
  t.nips =node.input_size;
  t.nops = 2;
  t.nhid = 2;
  t.nb =2;
  t.nw = t.nhid*(t.nips + t.nops);
  switch(node.input_size){
    case 1:
          trained_ann = trained_ann1;
          break;
    case 4:
          trained_ann = trained_ann4;
          break;
   case 7:
          trained_ann = trained_ann7;
          break;
   /*  case 10:
          trained_ann = trained_ann10;
          break;
    case 13:
          trained_ann = trained_ann13;
          break;*/
    default: 
          trained_ann = NULL;

  }
  for(uint8_t i=0; i<t.nb;i++){
    t.b[i]= trained_ann[i];
    //printf("Value of t.b[%d]:%d\n",i,(int)(1000000*t.b[i]));
  }
  for(uint8_t i=0; i<t.nw;i++){
    t.w[i]= trained_ann[t.nb+i];
    //printf("Value of t.w[%d]:%d\n",i,(int)(1000000*t.w[i]));
     }
 
  
  etimer_set(&et1, CLOCK_SECOND*10);

  printf(".....Waiting....\n");
  PROCESS_WAIT_UNTIL(etimer_expired(&et1));
  //etimer_set(&et2, CLOCK_SECOND*2);
 
  while(1){
    etimer_set(&et2, CLOCK_SECOND*0.5);
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
      for ( s=list_head(neighbors_list);s!=NULL;s=list_item_next(s)){
          if(s->nlayer == node.nlayer +1 )
               linkaddr_copy(&output_ngh_addr, &s->addr);
    }
      
   }
  }
  PROCESS_END();
  
}
