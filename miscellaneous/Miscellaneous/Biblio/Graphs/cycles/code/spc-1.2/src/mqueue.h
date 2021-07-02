/* -----   queues definitions ----- */
/* needs standard type "node" to be properly defined */

node *q_head,                    /* head of the queue */
     *q_tail,                    /* tail of the queue */
     *q_entry;                   /* entry to the middle of queue */


#define INIT_QUEUE(source)\
{\
q_head = source;\
q_tail = dummyNode;\
source -> next   = dummyNode;\
source -> prev   = NNULL;\
dummyNode -> next = NNULL;\
dummyNode -> prev = source;\
}

#define NONEMPTY_QUEUE           ( q_head != q_tail )

#define NODE_OUT_OF_QUEUE(node)\
  (( node -> prev == NNULL ) && ( node -> next == NNULL ))

#define NODE_IN_QUEUE(node)\
  (( node -> prev != NNULL ) || ( node -> next != NNULL ))

#define EXTRACT_FIRST(node)\
{\
node = q_head;\
q_head = q_head -> next;\
q_head -> prev = NNULL;\
node -> prev = node -> next = NNULL;\
}

/* assume queue contains at least two elements */

#define DELETE_FROM_QUEUE(v)\
{\
   if ( v != q_tail )\
     v -> next -> prev = v -> prev;\
   else {\
     q_tail = q_tail -> prev;\
     q_tail -> next = NNULL;\
   }\
   if ( v != q_head )\
     v -> prev -> next = v -> next;\
   else {\
     q_head = q_head -> next;\
     q_head -> prev = NNULL;\
   }\
   v -> prev = v -> next = NNULL;\
}


/* asume queue is non-empty */
#define INSERT_TO_QUEUE(node)\
{\
  q_tail -> next = node;\
  node -> prev = q_tail;\
  q_tail = node;\
  q_tail -> next = NNULL;\
}


