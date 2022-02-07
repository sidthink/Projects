CONTIKI_PROJECT = pancoordinator_g pannodes_g pancoordinator_l pannodes_l
all: $(CONTIKI_PROJECT)

PLATFORMS_EXCLUDE = sky nrf52dk native

CONTIKI=../..

 # force Orchestra from command line
#MAKE_WITH_ORCHESTRA ?= 0
# force Security from command line
##MAKE_WITH_SECURITY ?= 0
 # print #routes periodically, used for regression tests

# Orchestra link-based rule? (Works only if Orchestra & storing mode routing is enabled)
#MAKE_WITH_LINK_BASED_ORCHESTRA ?= 0

MAKE_ROUTING = MAKE_ROUTING_NULLROUTING
MAKE_NET = MAKE_NET_NULLNET
MAKE_MAC = MAKE_MAC_TSCH

#include $(CONTIKI)/Makefile.dir-variables
#MODULES += $(CONTIKI_NG_SERVICES_DIR)/shell
MODULES += os/services/TREE
LIB += lm

include $(CONTIKI)/Makefile.include
