/*
 * Copyright (c) 2001,02,03,08 Bernd Walter Computer Technology
 * Copyright (c) 2008 FIZON GmbH
 * All rights reserved.
 *
 * $URL: https://seewolf.fizon.de/svn/projects/matthies/Henry/Server/trunk/contrib/libfizonbase/threadpool.h $
 * $Date: 2025-05-15 13:42:25 +0200 (Thu, 15 May 2025) $
 * $Author: ticso $
 * $Rev: 49223 $
 */

#ifndef _THREADPOOL
#define _THREADPOOL

#define WITH_POOL

#ifdef WITH_POOL
#include <thread>
#include <future>
#include <functional>

namespace bwct
	{
	class Threadpool : public Base {
	private:
		Mutex queue_mtx;
		CV queue_cv;
		Array<std::thread> workers;
		Array<std::function<void()>> queue;
		int active_workers;
		bool stop_threads;

		void worker();

	public:
		template <typename F, typename... Args>
		auto AddJob(F&& f, Args&&... args) -> std::future<decltype(f(args...))>;

		Threadpool(int num);
		~Threadpool();

		int get_active()
		{
			return active_workers;
		}

		int get_inactive()
		{
			return workers.max + 1 - active_workers;
		}

		int get_num()
		{
			return workers.max + 1;
		}
	};

	template <typename F, typename... Args>
	auto
	Threadpool::AddJob(F&& f, Args&&... args) -> std::future<decltype(f(args...))>
	{
		auto func = std::bind(std::forward<F>(f), std::forward<Args>(args)...);
		auto job = std::make_shared<std::packaged_task<decltype(f(args...))()>>(func);
		auto wrapper_func = [job]() { (*job)(); };
		bool async = true;

		Mutex::Guard mtx(queue_mtx);
		if (get_inactive() == 0) {
			// test if we are already running by one of our workers
			auto thread = std::this_thread::get_id();
			for (int i = 0; async && i <= workers.max; i++) {
				if (thread == workers[i].get_id()) {
					async = false;
				}
			}
		}
		if (async) {
			queue << wrapper_func;
			queue_cv.signal();
			mtx.unlock();
		} else {
			mtx.unlock();
			(*job)();
		}

		return job->get_future();
	}
}

#endif

#endif /* !_THREADPOOL */
