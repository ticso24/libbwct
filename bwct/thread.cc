/*
 * Copyright (c) 2001,02,03,08 Bernd Walter Computer Technology
 * Copyright (c) 2008 FIZON GmbH
 * All rights reserved.
 *
 * $URL: https://seewolf.fizon.de/svn/projects/matthies/Henry/Server/trunk/contrib/libfizonbase/thread.cc $
 * $Date: 2020-03-10 14:37:35 +0100 (Tue, 10 Mar 2020) $
 * $Author: ticso $
 * $Rev: 42224 $
 */

#include <bwct/bwct.h>

Thread::Thread()
{
	id = 0;
}

Thread::Thread(const Thread& cpy) :
	Base(cpy)
{
	id = 0;
}

Thread::~Thread()
{
}

void
Thread::detach()
{
	pthread_detach(id);
}

void
Thread::terminate()
{
	if (id != 0) {
		if (pthread_cancel(id) == 0) {
			pthread_join(id, NULL);
		}
		id = 0;
	}
}

void *
Thread::starthelp(void *data)
{
	try {
		((Thread*)data)->threadstart();
	} catch (std::exception& e) {
		syslog(LOG_DEBUG, "exception %s:%s",
		    typeid(e).name(), e.what());
	} catch (...) {
		syslog(LOG_DEBUG, "exception");
	}
	((Thread*)data)->threadend();
	delete ((Thread*)data);
//	pthread_exit(NULL);
	return NULL;
}

void
Thread::start()
{

	cassert(id == 0);
	pthread_attr_t thread_attr;
	int s;
	size_t tmp_size=0;
	s = pthread_attr_init(&thread_attr);
	cassert(s == 0);
	//s = pthread_attr_getstacksize(&thread_attr , &tmp_size);
	//cassert(s == 0);
	tmp_size = 8ULL * 1024 * 1024; // FreeBSD default on amd64 has 2MB
	s = pthread_attr_setstacksize(&thread_attr , tmp_size);
	cassert(s == 0);
	tmp_size = 1ULL * 1024 * 1024;
	s = pthread_attr_setguardsize(&thread_attr , tmp_size);
	cassert(s == 0);
	if (pthread_create(&id, &thread_attr, starthelp, this) != 0) {
		pthread_attr_destroy(&thread_attr);
		throw Error("creating thread failed");
	}
	pthread_attr_destroy(&thread_attr);
	detach(); // TODO make it an optional attribute
}

void
Thread::atforkwipe()
{
	id = 0;
}

void
Thread::setname(const String& name)
{
	pthread_set_name_np(id, name.c_str());
}

void
Thread::join()
{

	cassert(id == 0);
	if (pthread_join(id, &retvalue) != 0)
		throw Error("joining thread failed");
}

void
setthreadname(const String& name)
{
	pthread_set_name_np(pthread_self(), name.c_str());
}

Mutex::Mutex()
{
	locked = false;
	dead = false;
	pthread_mutex_init(&mutex, NULL);
}

Mutex::~Mutex()
{
	abort_assert (!locked);
	pthread_mutex_destroy(&mutex);
}

int
Mutex::lock()
{
	cassert (!dead);
	while (pthread_mutex_lock(&mutex) != 0);
	cassert (!locked);
	locked = true;
	return 0;
}

int
Mutex::trylock()
{
	int ret;

	cassert (!dead);
	ret = pthread_mutex_trylock(&mutex);
	if (ret == 0) {
		cassert (!locked);
		locked = true;
	}
	return ret;
}

int
Mutex::unlock()
{
	cassert (!dead);
	cassert (locked);
	locked = false;
	return pthread_mutex_unlock(&mutex);
}

bool
Mutex::islocked()
{
	cassert (!dead);
	return locked;
}

void
Mutex::setdead()
{
	dead = true;
}

Mutex::Guard::Guard(Mutex& nmtx, bool lck)
{
	mtx = &nmtx;
	locked = false;
	if (lck)
		lock();
}

bool
Mutex::Guard::islocked()
{
	return locked;
}

Mutex::Guard::~Guard()
{
	if (locked)
		mtx->unlock();
}

void
Mutex::Guard::lock()
{
	cassert (!locked);
	mtx->lock();
	locked = true;
}

int
Mutex::Guard::trylock()
{
	int ret;
	cassert (!locked);
	ret = mtx->trylock();
	if (ret == 0) {
		locked = true;
	}
	return ret;
}

void
Mutex::Guard::unlock()
{
	cassert (locked == 1);
	mtx->unlock();
	locked = false;
}

CV::CV()
{
	pthread_cond_init(&cv, NULL);
}

CV::~CV()
{
	pthread_cond_destroy(&cv);
}

int
CV::wait(Mutex &mtx, time_t timeout)
{
	int ret;
	mtx.locked = false;
	struct timespec ts;
	clock_gettime(CLOCK_REALTIME_FAST, &ts);
	ts.tv_sec += timeout;
	ret = pthread_cond_timedwait(&cv, &mtx.mutex, &ts);
	mtx.locked = true;
	return ret;
}

int
CV::wait(Mutex &mtx)
{
	int ret;
	mtx.locked = false;
	ret = pthread_cond_wait(&cv, &mtx.mutex);
	mtx.locked = true;
	return ret;
}

int
CV::signal()
{
	return pthread_cond_signal(&cv);
}

int
CV::broadcast()
{
	return pthread_cond_broadcast(&cv);
}

RW_Lock::RW_Lock()
{
	pthread_rwlock_init(&rwlock, NULL);
}

RW_Lock::~RW_Lock()
{
	pthread_rwlock_destroy(&rwlock);
}

void
RW_Lock::rdlock()
{
	pthread_rwlock_rdlock(&rwlock);
}

void
RW_Lock::wrlock()
{
	pthread_rwlock_wrlock(&rwlock);
}

void
RW_Lock::unlock()
{
	pthread_rwlock_unlock(&rwlock);
}

int
RW_Lock::tryrdlock()
{
	return pthread_rwlock_tryrdlock(&rwlock);
}

int
RW_Lock::trywrlock()
{
	return pthread_rwlock_trywrlock(&rwlock);
}

cyclic::cyclic(uint64_t nperiod)
{
	period = nperiod;
}

cyclic::~cyclic()
{
	// TODO terminate thread
	abort_assert(0);
}

void *
cyclic::threadstart()
{
	for (;;) {
		job();
		if (period != 0) {
			usleep(period);
		}
	}
	return NULL;
}

void
cyclic::threadend()
{
}

