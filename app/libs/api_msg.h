#ifndef _API_MSG_H
#define _API_MSG_H


#define NO_MSG              0

void msg_queue_init(void);
void msg_queue_clear(void);
void msg_enqueue(u16 msg);
u16 msg_dequeue(void);
uint msg_queue_check(u16 msg);                  //查询消息池里面有多少个msg消息

#endif // _API_MSG_H
