#include "rinoo/scheduler/module.h"

t_channel *rinoo_channel(t_sched *sched)
{
	t_channel *channel;

	channel = calloc(1, sizeof(*channel));
	if (channel == NULL) {
		return NULL;
	}
	channel->sched = sched;
	return channel;
}

void rinoo_channel_destroy(t_channel *channel)
{
	free(channel);
}

int rinoo_channel_read(t_channel *channel, void **dest, size_t *size)
{
	t_task *task;

	if (channel->sched != rinoo_sched_self()) {
		return -1;
	}
	if (channel->buf == NULL) {
		channel->task = rinoo_task_self();
		rinoo_task_release(rinoo_sched_self());
		if (channel->buf == NULL) {
			return -1;
		}
	}
	*dest = channel->buf;
	*size = channel->size;
	task = channel->task;
	channel->buf = NULL;
	channel->size = 0;
	channel->task = NULL;
	rinoo_task_schedule(task, NULL);
	return 0;
}

int rinoo_channel_write(t_channel *channel, void *buf, size_t size)
{
	t_task *task;

	if (channel->sched != rinoo_sched_self()) {
		return -1;
	}
	channel->buf = buf;
	channel->size = size;
	task = channel->task;
	if (task != NULL) {
		rinoo_task_schedule(task, NULL);
	}
	rinoo_task_release(rinoo_sched_self());
	return 0;
}
