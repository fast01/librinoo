/**
 * @file   rinoo_task.c
 * @author Reginald Lips <reginald.l@gmail.com> - Copyright 2012
 * @date   Fri Jan 13 18:07:14 2012
 *
 * @brief  rinoo_task unit test
 *
 *
 */

#include	"rinoo/rinoo.h"

t_rinoosched *sched;
int checker = 0;

void task3(t_rinoosched *unused(sched), void *unused(arg))
{
	printf("%s start\n", __FUNCTION__);
	checker = 3;
	rinoo_sched_stop(sched);
	printf("%s end\n", __FUNCTION__);
}

void task2(t_rinoosched *sched, void *unused(arg))
{
	t_rinootask *t3;

	printf("%s start\n", __FUNCTION__);
	XTEST(checker == 1);
	t3 = rinoo_task(sched, task3, NULL);
	checker = 2;
	rinoo_task_run(t3);
	XTEST(checker == 3);
	printf("%s end\n", __FUNCTION__);
}

void task1(t_rinoosched *sched, void *unused(arg))
{
	t_rinootask *t2;

	printf("%s start\n", __FUNCTION__);
	XTEST(checker == 0);
	t2 = rinoo_task(sched, task2, NULL);
	checker = 1;
	rinoo_task_run(t2);
	XTEST(checker == 3);
	printf("%s end\n", __FUNCTION__);
}

/**
 * Main function for this unit test
 *
 *
 * @return 0 if test passed
 */
int main()
{
	t_rinootask *t1;

	sched = rinoo_sched();
	XTEST(sched != NULL);
	t1 = rinoo_task(sched, task1, NULL);
	XTEST(t1 != NULL);
	rinoo_task_schedule(t1, NULL);
	rinoo_sched_loop(sched);
	rinoo_sched_destroy(sched);
	XTEST(checker == 3);
	XPASS();
}
