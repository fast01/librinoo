#include "rinoo/rinoo.h"

void task1(void *channel)
{
	int i;
	int *x;
	size_t len;

	for (i = 0; i < 10; i++) {
		rinoo_channel_write(channel, &i, sizeof(i));
		rinoo_channel_read(channel, (void **) &x, &len);
		XTEST(i == *x);
	}
}

void task2(void *channel)
{
	int i;
	int *x;
	size_t len;

	for (i = 0; i < 10; i++) {
		rinoo_channel_read(channel, (void **) &x, &len);
		XTEST(i == *x);
		rinoo_channel_write(channel, &i, sizeof(i));
	}
}

/**
 * Main function for this unit test
 *
 *
 * @return 0 if test passed
 */
int main()
{
	t_sched *sched;
	t_channel *channel;


	sched = rinoo_sched();
	XTEST(sched != NULL);
	channel = rinoo_channel(sched);
	XTEST(channel != NULL);
	XTEST(rinoo_task_start(sched, task1, channel) == 0);
	XTEST(rinoo_task_start(sched, task2, channel) == 0);
	rinoo_sched_loop(sched);
	rinoo_channel_destroy(channel);
	rinoo_sched_destroy(sched);
	XPASS();
}
