typedef struct {
  int     size;
	int		  head;
	int 	  tail;
	uint8_t	*buf;
} fifo_t;

void fifoInit(fifo_t *fifo, uint8_t *buf, int size) {
  fifo->size = size;
	fifo->head = 0;
	fifo->tail = 0;
	fifo->buf  = buf;
}

bool fifoPut(fifo_t *fifo, uint8_t c) {
	int next;
	
	next = fifo->head + 1;
  if (next == fifo->size) next = 0;

	if (next == fifo->tail) {
		return 0; /* full */
	}
	
	fifo->buf[fifo->head] = c;
	fifo->head = next;
	
	return 1;
}

bool fifoGet(fifo_t *fifo, uint8_t *pc) {
	int next;
	
	if (fifo->head == fifo->tail) {
		return 0; /* empty */
	}
	
	next = fifo->tail + 1;
  if (next == fifo->size) next = 0;
	
	*pc = fifo->buf[fifo->tail];
	fifo->tail = next;

	return 1;
}

int fifoAvailable(fifo_t *fifo) {
  if (fifo->head >= fifo->tail) return (fifo->head - fifo->tail);
  else return (fifo->size + fifo->head - fifo->tail);
}

int fifoFree(fifo_t *fifo) {
	return (fifo->size - 1 - fifoAvailable(fifo));
}

