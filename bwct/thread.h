/*
 * Copyright (c) 2001,02 Bernd Walter Computer Technology
 * All rights reserved.
 *
 * $URL$
 * $Date$
 * $Author$
 * $Rev$
 */

#ifndef _THREAD
#define _THREAD

class Thread;
class Mutex;
class CV;

#ifdef HAVE_PTHREAD
#include <pthread.h>
#endif
#include <bwct/tool.h>

class Thread : public Base {
protected:
	~Thread();
	void *retvalue;
private:
#ifdef HAVE_PTHREAD
	pthread_t id;
#else
	pid_t id;
#endif
	Thread(Thread &src);
#ifdef HAVE_PTHREAD
	void detach();
#endif
	virtual void *threadstart() = 0;
	virtual void threadend() = 0;
	static void *starthelp(void *data);
public:
	friend class Listen;
	Thread();
	Thread(const Thread& cpy);
	void start();
#ifdef HAVE_PTHREAD
	void join();
#endif
};

#ifdef HAVE_PTHREAD
class Mutex : public Base {
private:
	pthread_mutex_t mutex;
	Mutex(Mutex &src);
	int lck;
	int dead;
public:
	friend class CV;
	Mutex();
	~Mutex();
	int lock();
	int trylock();
	int unlock();
	int locked();
	void setdead();
};
#else
class Mutex : public Base {
private:
	Mutex(Mutex &src);
public:
	Mutex() {};
	~Mutex() {};
	int lock() {
		return 0;
	};
	int trylock() {
		return 0;
	};
	int unlock() {
		return 0;
	};
	int locked() {
		return 0;
	};
	void setdead() {
	};
};
#endif

class Mtx_Guard : public Base {
private:
	Mutex *mtx;
	int locked;
public:
	Mtx_Guard(Mutex& nmtx);
	~Mtx_Guard();
	void lock();
	void unlock();
};

#ifdef HAVE_PTHREAD
class CV : public Base {
private:
	CV(CV &src);
	pthread_cond_t cv;
public:
	CV();
	~CV();
	int wait(Mutex &mtx);
	int signal();
	int broadcast();
};

#ifdef FreeBSD
class RW_Lock : public Base {
private:
	pthread_rwlock_t rwlock;
	pthread_rwlockattr_t attr;
public:
	RW_Lock();
	~RW_Lock();
	void rdlock();
	void wrlock();
	void unlock();
	int tryrdlock();
	int trywrlock();
};
#endif /* FreeBSD */
#endif

class cyclic : public Thread {
private:
	uint64_t period;
	virtual void *threadstart();
public:
	cyclic(uint64_t nperiod);
	~cyclic();
	virtual void job() = 0;
};

#endif /* !_THREAD */
