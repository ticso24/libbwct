/*
 * Copyright (c) 2001,02 Bernd Walter Computer Technology
 * All rights reserved.
 *
 * $URL$
 * $Date$
 * $Author$
 * $Rev$
 */

#include "config.h"
#include "thread.h"

Thread::Thread() {
	id = 0;
}

Thread::Thread(const Thread& cpy) :
	Base(cpy)
{
	id = 0;
}

Thread::~Thread() {
}

#ifdef HAVE_PTHREAD
void
Thread::detach() {
	pthread_detach(id);
}
#endif

void *
Thread::starthelp(void *data) {
	try {
		((Thread*)data)->threadstart();
	} catch (std::exception& e) {
		syslog(LOG_DEBUG, "exception %s:%s",
		    typeid(e).name(), e.what());
	} catch (...) {
		syslog(LOG_DEBUG, "exception");
	}
	((Thread*)data)->threadend();
	pthread_exit(NULL);
	return NULL;
}

void
Thread::start() {

	cassert(id == 0);
#ifdef HAVE_PTHREAD
	if (pthread_create(&id, NULL, starthelp, this) != 0)
		throw Error("creating thread failed");
	detach(); // TODO make it an optional attribute
#else
	while ((id = fork()) == -1) {
		if (id == -1 && errno != EAGAIN)
			throw Error("fork failed");
		if (id == 0) {
			starthelp(this);
			exit(0);
		}
	}
#endif
}

#ifdef HAVE_PTHREAD
void
Thread::join() {

	cassert(id == 0);
	if (pthread_join(id, &retvalue) != 0)
		throw Error("joining thread failed");
}
#endif

#ifdef HAVE_PTHREAD

Mutex::Mutex() {
	lck = 0;
	dead = 0;
	pthread_mutex_init(&mutex, NULL);
}

Mutex::~Mutex() {
	cassert (lck == 0);
	pthread_mutex_destroy(&mutex);
}

int
Mutex::lock() {
	int ret;

	cassert (dead == 0);
	ret = pthread_mutex_lock(&mutex);
	if (ret == 0) {
		cassert (lck == 0);
		lck = 1;
	}
	return ret;
}

int
Mutex::trylock() {
	int ret;

	cassert (dead == 0);
	ret = pthread_mutex_trylock(&mutex);
	if (ret == 0) {
		cassert (lck == 0);
		lck = 1;
	}
	return ret;
}

int
Mutex::unlock() {
	cassert (dead == 0);
	cassert (lck == 1);
	lck = 0;
	return pthread_mutex_unlock(&mutex);
}

int
Mutex::locked() {
	cassert (dead == 0);
	return lck;
}

void
Mutex::setdead() {
	dead = 1;
}

Mtx_Guard::Mtx_Guard(Mutex& nmtx) {
	mtx = &nmtx;
	locked = 0;
}

Mtx_Guard::~Mtx_Guard() {
	if (locked == 1)
		mtx->unlock();
}

void
Mtx_Guard::lock() {
	cassert (locked == 0);
	mtx->lock();
	locked = 1;
}

void
Mtx_Guard::unlock() {
	cassert (locked == 1);
	mtx->unlock();
	locked = 0;
}

CV::CV() {
	pthread_cond_init(&cv, NULL);
}

CV::~CV() {
	pthread_cond_destroy(&cv);
}

int
CV::wait(Mutex &mtx) {
	return pthread_cond_wait(&cv, &mtx.mutex);
}

int
CV::signal() {
	return pthread_cond_signal(&cv);
}

int
CV::broadcast() {
	return pthread_cond_broadcast(&cv);
}

#ifdef FreeBSD
RW_Lock::RW_Lock() {
	pthread_rwlock_init(&rwlock, NULL);
}

RW_Lock::~RW_Lock() {
	pthread_rwlock_destroy(&rwlock);
}

void
RW_Lock::rdlock() {
	pthread_rwlock_rdlock(&rwlock);
}

void
RW_Lock::wrlock() {
	pthread_rwlock_wrlock(&rwlock);
}

void
RW_Lock::unlock() {
	pthread_rwlock_unlock(&rwlock);
}

int
RW_Lock::tryrdlock() {
	return pthread_rwlock_tryrdlock(&rwlock);
}

int
RW_Lock::trywrlock() {
	return pthread_rwlock_trywrlock(&rwlock);
}

#endif /* FreeBSD */

#endif

void *
cyclic::threadstart() {
	for (;;) {
		sleep(period);
		job();
	}
}

cyclic::cyclic(uint64_t nperiod) {
	period = nperiod;
}

cyclic::~cyclic() {
	// TODO terminate thread
	cassert(0);
}

