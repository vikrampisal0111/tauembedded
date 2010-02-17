#include "simple.h"

#include "uip.h"
void simple_init(void) {
    uip_listen(HTONS(1234));
}

void simple_app(void) {
    if(uip_newdata() || uip_rexmit()) {
        uip_send("ok\n", 3);
    }
}


