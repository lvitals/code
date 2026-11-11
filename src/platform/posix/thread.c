/* Threads, POSIX (pthreads): what the search of a project's text runs
 * on, one on each core (platform.h). */
#define _XOPEN_SOURCE 700
#include <errno.h>
#include <pthread.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>
#include "../platform.h"

struct plat_thread {
	pthread_t t;
	void (*fn)(void *);
	void *arg;
};

struct plat_lock {
	pthread_mutex_t mu;
	pthread_cond_t cv;
};

int
plat_cpus(void)
{
	long n = sysconf(_SC_NPROCESSORS_ONLN);

	return n > 0 ? (int)n : 1;
}

long
plat_ms(void)
{
	struct timespec ts;

	clock_gettime(CLOCK_MONOTONIC, &ts);
	return (long)ts.tv_sec * 1000L + ts.tv_nsec / 1000000L;
}

static void *
run(void *a)
{
	struct plat_thread *t = a;

	t->fn(t->arg);
	return 0;
}

struct plat_thread *
plat_thread_start(void (*fn)(void *), void *arg)
{
	struct plat_thread *t = malloc(sizeof *t);

	if(t == 0)
		return 0;
	t->fn = fn;
	t->arg = arg;
	if(pthread_create(&t->t, 0, run, t) != 0) {
		free(t);
		return 0;
	}
	return t;
}

void
plat_thread_join(struct plat_thread *t)
{
	pthread_join(t->t, 0);
	free(t);
}

struct plat_lock *
plat_lock_new(void)
{
	struct plat_lock *l = malloc(sizeof *l);

	if(l == 0)
		return 0;
	if(pthread_mutex_init(&l->mu, 0) != 0) {
		free(l);
		return 0;
	}
	if(pthread_cond_init(&l->cv, 0) != 0) {
		pthread_mutex_destroy(&l->mu);
		free(l);
		return 0;
	}
	return l;
}

void
plat_lock_free(struct plat_lock *l)
{
	pthread_cond_destroy(&l->cv);
	pthread_mutex_destroy(&l->mu);
	free(l);
}

void
plat_lock(struct plat_lock *l)
{
	pthread_mutex_lock(&l->mu);
}

void
plat_unlock(struct plat_lock *l)
{
	pthread_mutex_unlock(&l->mu);
}

void
plat_wait(struct plat_lock *l, int ms)
{
	struct timespec ts;

	if(ms < 0) {
		pthread_cond_wait(&l->cv, &l->mu);
		return;
	}
	clock_gettime(CLOCK_REALTIME, &ts);
	ts.tv_sec += ms / 1000;
	ts.tv_nsec += (long)(ms % 1000) * 1000000L;
	if(ts.tv_nsec >= 1000000000L) {
		ts.tv_sec++;
		ts.tv_nsec -= 1000000000L;
	}
	pthread_cond_timedwait(&l->cv, &l->mu, &ts);
}

void
plat_wake(struct plat_lock *l)
{
	pthread_cond_broadcast(&l->cv);
}
