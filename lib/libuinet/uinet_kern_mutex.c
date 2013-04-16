/*-
 * Copyright (c) 2010 Kip Macy
 * All rights reserved.
 * Copyright (c) 2013 Patrick Kelsey. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice unmodified, this list of conditions, and the following
 *    disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 * OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 * IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
 * NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 * THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 * Dervied in part from libplebnet's pn_lock.c.
 *
 */

#include <sys/param.h>
#include <sys/conf.h>
#include <sys/mutex.h>
#include <sys/proc.h>

#include <pthread.h>

struct mtx Giant;


static void
assert_mtx(struct lock_object *lock, int what)
{

	mtx_assert((struct mtx *)lock, what);
}

static void
lock_mtx(struct lock_object *lock, int how)
{

	mtx_lock((struct mtx *)lock);
}

static int
unlock_mtx(struct lock_object *lock)
{
	struct mtx *m;

	m = (struct mtx *)lock;
	mtx_assert(m, MA_OWNED | MA_NOTRECURSED);
	mtx_unlock(m);
	return (0);
}

/*
 * Lock classes for sleep and spin mutexes.
 */
struct lock_class lock_class_mtx_sleep = {
	.lc_name = "sleep mutex",
	.lc_flags = LC_SLEEPLOCK | LC_RECURSABLE,
	.lc_assert = assert_mtx,
	.lc_lock = lock_mtx,
	.lc_unlock = unlock_mtx,
#ifndef UINET
#ifdef DDB
	.lc_ddb_show = db_show_mtx,
#endif
#ifdef KDTRACE_HOOKS
	.lc_owner = owner_mtx,
#endif
#endif
};

/*
 * XXX should never be used; provided here for linkage with subr_lock.c
 */
struct lock_class lock_class_mtx_spin;

void
_thread_lock_flags(struct thread *td, int opts, const char *file, int line)
{

	mtx_lock(td->td_lock);
}

void
mtx_init(struct mtx *m, const char *name, const char *type, int opts)
{
	pthread_mutexattr_t attr;

	lock_init(&m->lock_object, &lock_class_mtx_sleep, name, type, opts);
	pthread_mutexattr_init(&attr);
	pthread_mutex_init(&m->mtx_lock, &attr);
}

void
mtx_destroy(struct mtx *m)
{

	pthread_mutex_destroy(&m->mtx_lock);
}

void
mtx_sysinit(void *arg)
{
	struct mtx_args *margs = arg;

	mtx_init(margs->ma_mtx, margs->ma_desc, NULL, margs->ma_opts);
}

void
_mtx_lock_flags(struct mtx *m, int opts, const char *file, int line)
{

	pthread_mutex_lock(&m->mtx_lock);
}

void
_mtx_unlock_flags(struct mtx *m, int opts, const char *file, int line)
{

	pthread_mutex_unlock(&m->mtx_lock);
}

int
_mtx_trylock(struct mtx *m, int opts, const char *file, int line)
{

	return (pthread_mutex_trylock(&m->mtx_lock));
}

void
_mtx_lock_spin_flags(struct mtx *m, int opts, const char *file, int line)
{

	pthread_mutex_lock(&m->mtx_lock);
}

void
_mtx_unlock_spin_flags(struct mtx *m, int opts, const char *file, int line)
{

	pthread_mutex_unlock(&m->mtx_lock);
}
