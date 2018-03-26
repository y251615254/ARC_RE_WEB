#include "httpd.h"

#define MAX_TIMER	10
#define MAX_TIMER_IR	10
//#define TIMER_CHK_INT	(CLOCKHZ/2)
/*
 * Timer task [tmr_task()]
 * 	data structure & routines which support timer
 */
struct timerDS {
	struct timerDS	*next;
	unsigned long	expire;
	void            (*handler)(COOKIE_t);
	COOKIE_t        cookie;
};

static struct timerDS null_Timer = {
	NULL, 0, NULL, (COOKIE_t)0
};

static struct timerDS timer_pool[MAX_TIMER];
static struct timerDS timer_pool_IR[MAX_TIMER_IR];
static int IR_head, IR_end;
static struct timerDS *timer_list;
static struct timerDS *free_timer_list;
static int Initialized = 0;

void tmr_init() {
	int	i;

	if (Initialized) return;

	Initialized = 1;
	/* initialize free timer list */
	memset(timer_pool, 0, sizeof(timer_pool));
	memset(timer_pool_IR, 0, sizeof(timer_pool));
	IR_head = IR_end =0;
	free_timer_list = timer_pool;
	for (i=0; i < (MAX_TIMER - 1); i++)
		timer_pool[i].next = &timer_pool[i+1];

	timer_pool[MAX_TIMER - 1].next = NULL;

	/* initialize allocated timer list */
	timer_list = &null_Timer;
	// free_timer_list = NULL;
#if HTRACE >= 3
	E_PRINTF("timer task initialized!\n");
#endif
}

#ifdef _ETCPIP
void tmr_set_IR(COOKIE_t cookie, int dly_sec, void (*callback)(COOKIE_t)) {
	struct timerDS *new_timer, *pre_timer, *timerp;
	unsigned long   when;

	//LOCK_TIMER();
	if (!Initialized) tmr_init();

	when = SYS_CLOCK() +((unsigned long)CLOCKHZ)*((unsigned long)dly_sec);

	new_timer = &timer_pool_IR[IR_head];
	new_timer->cookie = cookie;
	new_timer->expire = when;
	new_timer->handler = callback;

	IR_head++;
	if (IR_head == MAX_TIMER_IR) IR_head=0;
	return;
}
#endif

void tmr_set(COOKIE_t cookie, int dly_sec, void (*callback)(COOKIE_t)) 
{
	struct timerDS *new_timer, *pre_timer, *timerp;
	unsigned long   when;

	LOCK_TIMER();
	if (!Initialized) tmr_init();

	timerp = timer_list->next;
	when = SYS_CLOCK()	+((unsigned long)CLOCKHZ)*((unsigned long)dly_sec);

	while (timerp) {
		/* search for existing timer for current task */
		if (timerp->cookie != cookie) {
			timerp = timerp->next;
			continue;
		}

		/* task just update its timer */
		timerp->expire = when;
		timerp->handler = callback;
		UNLOCK_TIMER();	/* release resource */
		return;
	}

	/* timer never be set for this task, allocate a timer for it */
no_rsc:
	if (free_timer_list == NULL) {
		UNLOCK_TIMER();
		DELAY(1);
		LOCK_TIMER();
		goto no_rsc;
	}

	new_timer = free_timer_list;
	free_timer_list = new_timer->next;
	new_timer->cookie = cookie;
	new_timer->expire = when;
	new_timer->handler = callback;

	/* Insert the timer in time list in chronological order */
	timerp = timer_list->next;
	pre_timer = timer_list;

	while (timerp) {
		if (timerp->expire > when)
			break;

		pre_timer = timerp;
		timerp = pre_timer->next;
	}

	pre_timer->next = new_timer;
	new_timer->next = timerp;
	UNLOCK_TIMER();
	return;
}

void tmr_cancel(COOKIE_t cookie) {
	struct timerDS *timerp;
	struct timerDS *pre_timer;

	LOCK_TIMER();
	if (!Initialized) {
		tmr_init();
		UNLOCK_TIMER();
		return;
	}

	pre_timer = timer_list;
	timerp = timer_list->next;

	while (timerp) {
		if (timerp->cookie == cookie) {
			pre_timer->next = timerp->next;
			timerp->next = free_timer_list;
			free_timer_list = timerp;
			UNLOCK_TIMER();
			return;
		}

		pre_timer = timerp;
		timerp = timerp->next;
	}

	UNLOCK_TIMER();
	return;
}

void tmr_check() 
{
	struct timerDS *timerp, *pre_timer;
	unsigned long   now;

	LOCK_TIMER();
	if (!Initialized) {
		tmr_init();
		UNLOCK_TIMER();
		return;
	}

	timerp = timer_list->next;
	pre_timer = timer_list;
	now = SYS_CLOCK();

	while (timerp) {
		if (timerp->expire <= now) {
			pre_timer->next = timerp->next;
			UNLOCK_TIMER();
			timerp->handler(timerp->cookie);
			LOCK_TIMER();
			timerp->next = free_timer_list;
			free_timer_list = timerp;
			timerp = pre_timer->next;
			now = SYS_CLOCK();
			continue;
		}

		pre_timer = timerp;
		timerp = timerp->next;
	}

	UNLOCK_TIMER();
}

int is_tmr_init()
{
	return Initialized;
}

void tmr_task() 
{
	if (!Initialized) tmr_init();

	while (1) {
		tmr_check();
		//DELAY(TIMER_CHK_INT);
		sleep(1);
	}
}
