/*
 * Copyright (c) 2025 Bernd Walter Computer Technology
 * Copyright (c) 2025 FIZON GmbH
 * All rights reserved.
 *
 * $URL: https://seewolf.fizon.de/svn/projects/matthies/Henry/Server/trunk/contrib/libfizonbase/threadpool.cc $
 * $Date: 2025-06-08 17:13:35 +0200 (Sun, 08 Jun 2025) $
 * $Author: ticso $
 * $Rev: 49329 $
 */

#include "bwct.h"

void
Threadpool::worker()
{
	Mutex::Guard mtx(queue_mtx);
	while (!stop_threads || queue.max >= 0) {
		if (queue.max >= 0) {
			auto job = std::move(queue[0]);
			queue.pop_front();
			active_workers++;
			mtx.unlock();
			job();
			mtx.lock();
			active_workers--;
		} else {
			queue_cv.wait(queue_mtx);
		}
	}
}

Threadpool::Threadpool(int num)
{
	stop_threads = false;
	for (int i = 0; i < num; i++) {
		workers[i] = std::thread([this]() { this->worker(); });
	}
}

Threadpool::~Threadpool()
{
	// signal and wait for workers to stop before we desonstruct ourself
	{
		Mutex::Guard mtx(queue_mtx);
		stop_threads = true;
		queue_cv.broadcast();
	}
	for (int i = 0; i <= workers.max; i++) {
		workers[i].join();
	}
}
