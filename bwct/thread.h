/*
 * Copyright (c) 2001,02,03,08 Bernd Walter Computer Technology
 * Copyright (c) 2008 FIZON GmbH
 * All rights reserved.
 *
 * $URL: https://seewolf.fizon.de/svn/projects/matthies/Henry/Server/trunk/contrib/libfizonbase/thread.h $
 * $Date: 2017-09-24 03:12:48 +0200 (Sun, 24 Sep 2017) $
 * $Author: ticso $
 * $Rev: 33771 $
 */

#ifndef _THREAD
#define _THREAD

class Thread;
class Mutex;
class CV;

#include "bwct.h"
#include <pthread.h>

class Thread : public Base {
protected:
	~Thread();
	void *retvalue;
	void terminate();
private:
	pthread_t id;
	Thread(Thread &src);
	void detach();
	virtual void threadend() = 0;
	static void *starthelp(void *data);
public:
	virtual void *threadstart() = 0;
	friend class Listen;
	Thread();
	Thread(const Thread& cpy);
	virtual void start();
	void join();
	void atforkwipe();
	void setname(const String& name);
};

void setthreadname(const String& name);

class Mutex : public Base {
private:
	pthread_mutex_t mutex;
	Mutex(Mutex &src);
	bool locked;
	bool dead;
public:
	class Guard : public Base {
	private:
		Mutex *mtx;
		bool locked;
	public:
		Guard(Mutex& nmtx, bool lck = true);
		~Guard();
		void lock();
		int trylock();
		void unlock();
		bool islocked();
	};

	friend class CV;
	Mutex();
	~Mutex();
	int lock();
	int trylock();
	int unlock();
	bool islocked();
	void setdead();
};

class CV : public Base {
private:
	CV(CV &src);
	pthread_cond_t cv;
public:
	CV();
	~CV();
	int wait(Mutex &mtx);
	int wait(Mutex &mtx, time_t timeout);
	int signal();
	int broadcast();
};

class RW_Lock : public Base {
private:
	pthread_rwlock_t rwlock;
// clang++: error: private field 'attr' is not used [-Werror,-Wunused-private-field]
//	pthread_rwlockattr_t attr;
public:
	class Guard : public Base {
	private:
		RW_Lock *rw_lock;
		bool locked;
	public:
		Guard(RW_Lock& nrw_lock);
		~Guard();
		void rdlock();
		void wrlock();
		int tryrdlock();
		int trywrlock();
		void unlock();
		bool islocked();
	};
	RW_Lock();
	~RW_Lock();
	void rdlock();
	void wrlock();
	void unlock();
	int tryrdlock();
	int trywrlock();
};

class cyclic : public Thread {
private:
	uint64_t period;
	virtual void *threadstart();
	virtual void threadend();
public:
	cyclic(uint64_t nperiod);
	~cyclic();
	virtual void job() = 0;
};

template <class T>
class a_global : public Base {
private:
	Mutex mtx;
	T* ptr;
public:
	a_global() {
		ptr = NULL;
	}
	void init() {
		if (ptr == NULL) {
			Mutex::Guard mutex(mtx);
			if (ptr == NULL) {
				ptr = new T;
			}
			mutex.unlock();
		}
	}
	bool isinit() const {
		return (ptr != NULL);
	}
	T* operator->() {
		init();
		return ptr;
	}
};

#endif /* !_THREAD */
