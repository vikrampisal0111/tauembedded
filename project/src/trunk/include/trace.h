#ifndef __TRACE_H__
#define __TRACE_H__

#if UART0_DEBUG == 1 || UART1_DEBUG == 1
        #include <stdio.h>
        #define TRACE(X ,reg...)   \
                do {	           \
                printf(X, ##reg);  \
                printf("\n");      \
                } while(0)

        #define trace(X ,reg...)     \
                do {	             \
                printf(X, ##reg);    \
                } while(0)

        #define ASSERT(expr)               \
                do{                        \
                        if (expr) { ; }    \
                        else  {            \
                        while (1){	   \
                        printf("\r\nAssert failed: " #expr " (file %s line %d)", \
                                 __FILE__, (int) __LINE__ );                     \
                        }                  \
                        }                  \
                } while(0)

        #define EP()  TRACE("%s Line %d", __FILE__, (int) __LINE__) ;
#else
        #define TRACE_INIT() 	  do{}while(0)
        #define ASSERT(X) 	  do{}while(0)
        #define TRACE(X, reg...)  do{}while(0)
        #define trace(X, reg...)  do{}while(0)
        #define EP()  	          do{}while(0)
#endif

#endif /* __TRACE_H_  */
