#ifndef _TIMER_H__
#define _TIMER_H__

typedef unsigned long 	COOKIE_t;
void tmr_set(COOKIE_t cookie, int dly_sec, void (*callback)(COOKIE_t));
void tmr_cancel(COOKIE_t cookie);
int is_tmr_init(void);
void tmr_init(void);
void tmr_check(void);
void tmr_task(void);
#endif
