#ifndef __SIMPLE_H__
#define __SIMPLE_H__

#include "uipopt.h"
#include "psock.h"

/* Next, we define the uip_tcp_appstate_t datatype. This is the state
   of our application, and the memory required for this state is
   allocated together with each TCP connection. One application state
   for each TCP connection. */
typedef struct simple_state {
  struct psock p;
  char inputbuffer[10];
  char name[40];
} uip_tcp_appstate_t;


#ifndef UIP_APPCALL
#define UIP_APPCALL simple_app
#endif /* UIP_APPCALL */

void simple_init(void);
void simple_app(void);

#endif
