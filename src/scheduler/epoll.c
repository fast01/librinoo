/**
 * @file   epoll.c
 * @author Reginald Lips <reginald.l@gmail.com>
 * @date   Tue Mar 20 15:37:04 2012
 *
 * @brief  This file manages the poll API working with epoll.
 *
 *
 */

#include	"rinoo/rinoo.h"

/**
 * Epoll initialization. It calls epoll_create and
 * initializes internal structures.
 *
 * @param sched Pointer to the scheduler to use.
 *
 * @return 0 if succeeds, else -1.
 */
int rinoo_epoll_init(t_rinoosched *sched)
{
	t_rinooepoll *data;

	XASSERT(sched != NULL, -1);

	data = calloc(1, sizeof(*data));
	if (data == NULL) {
		return -1;
	}
	data->fd = epoll_create(42); /* Size does not matter any more ;) */
	XASSERT(data->fd != -1, -1);
	if (sigemptyset(&data->sigmask) < 0) {
		close(data->fd);
		free(data);
		return -1;
	}
	if (sigaddset(&data->sigmask, SIGPIPE) < 0) {
		close(data->fd);
		free(data);
		return -1;
	}
	sigaction(SIGPIPE, &(struct sigaction){ .sa_handler = SIG_IGN }, NULL);
	sched->poll->context = data;
	return 0;
}

/**
 * Destroys the poller in a scheduler. Closes the epoll fd and free's memory.
 *
 * @param sched Pointer to the scheduler to use.
 */
void rinoo_epoll_destroy(t_rinoosched *sched)
{
	XASSERTN(sched != NULL);
	XASSERTN(sched->poll->context != NULL);

	close(((t_rinooepoll *) sched->poll->context)->fd);
	free(sched->poll->context);
}

/**
 * Insert a socket into epoll. It calls epoll_ctl.
 *
 * @param socket Pointer to the socket to add.
 * @param mode Polling mode to use to add.
 *
 * @return 0 if succeeds, else -1.
 */
int rinoo_epoll_insert(t_rinoosocket *socket, t_rinoosched_mode mode)
{
	struct epoll_event ev = { 0, { 0 } };

	XASSERT(socket != NULL, -1);

	if ((mode & RINOO_MODE_IN) == RINOO_MODE_IN) {
		ev.events |= EPOLLIN;
	}
	if ((mode & RINOO_MODE_OUT) == RINOO_MODE_OUT) {
		ev.events |= EPOLLOUT;
	}
	ev.events |= EPOLLONESHOT;
	ev.data.fd = socket->fd;
	if (unlikely(epoll_ctl(((t_rinooepoll *) socket->sched->poll->context)->fd,
			       EPOLL_CTL_ADD,
			       socket->fd,
			       &ev) != 0)) {
		return -1;
	}
	return 0;
}

/**
 * Adds a polling mode for a socket to epoll. It calls epoll_ctl.
 *
 * @param socket Pointer to the socket to add.
 * @param mode Polling mode to use to add.
 *
 * @return 0 if succeeds, else -1.
 */
int rinoo_epoll_addmode(t_rinoosocket *socket, t_rinoosched_mode mode)
{
	struct epoll_event ev = { 0, { 0 } };

	XASSERT(socket != NULL, -1);

	if ((mode & RINOO_MODE_IN) == RINOO_MODE_IN) {
		ev.events |= EPOLLIN;
	}
	if ((mode & RINOO_MODE_OUT) == RINOO_MODE_OUT) {
		ev.events |= EPOLLOUT;
	}
	ev.events |= EPOLLONESHOT;
	ev.data.fd = socket->fd;
	if (unlikely(epoll_ctl(((t_rinooepoll *) socket->sched->poll->context)->fd,
			       EPOLL_CTL_MOD,
			       socket->fd,
			       &ev) != 0)) {
		return -1;
	}
	return 0;
}

/**
 * Removes a socket from epoll. It calls epoll_ctl.
 *
 * @param socket Pointer to the socket to remove from epoll.
 *
 * @return 0 if succeeds, else -1.
 */
int rinoo_epoll_remove(t_rinoosocket *socket)
{
	XASSERT(socket != NULL, -1);

	if (unlikely(epoll_ctl(((t_rinooepoll *) socket->sched->poll->context)->fd,
			       EPOLL_CTL_DEL,
			       socket->fd,
			       NULL) != 0)) {
		return -1;
	}
	return 0;
}

/**
 * Start polling. It calls epoll_wait.
 *
 * @param sched Pointer to the scheduler to use.
 *
 * @return 0 if succeeds, else -1.
 */
int rinoo_epoll_poll(t_rinoosched *sched, u32 timeout)
{
	int i;
	int nbevents;
	t_rinoosocket *cursocket;
	t_rinooepoll *data;

	XASSERT(sched != NULL, -1);

	data = (t_rinooepoll *) sched->poll->context;
	nbevents = epoll_pwait(data->fd, data->events, RINOO_EPOLL_MAX_EVENTS, timeout, &data->sigmask);
	if (unlikely(nbevents == -1)) {
		/* We don't want to raise an error in this case */
		return 0;
	}
	for (i = 0; i < nbevents; i++) {
		cursocket = rinoo_sched_get(sched, data->events[i].data.fd);
		if ((data->events[i].events & EPOLLIN) == EPOLLIN) {
			errno = 0;
			rinoo_socket_resume(cursocket);
		}
		cursocket = rinoo_sched_get(sched, data->events[i].data.fd);
		/**
		 * If cursocket has been removed in the EPOLLIN step,
		 * rinoo_sched_get will return NULL.
		 */
		if (cursocket != NULL) {
			if ((data->events[i].events & EPOLLOUT) == EPOLLOUT) {
				errno = 0;
				rinoo_socket_resume(cursocket);
			}
			cursocket = rinoo_sched_get(sched, data->events[i].data.fd);
			/* socket previously removed ? */
			if (cursocket != NULL &&
			    ((data->events[i].events & EPOLLERR) == EPOLLERR ||
			     (data->events[i].events & EPOLLHUP) == EPOLLHUP)) {
				errno = ECONNRESET;
				rinoo_socket_resume(cursocket);
			}
		}
	}
	return 0;
}
