// (C) 2018 University of Bristol, Bar-Ilan University. See License.txt

/*
 * Worker.h
 *
 */

#ifndef TOOLS_WORKER_H_
#define TOOLS_WORKER_H_

#include "WaitQueue.h"

template <class T>
class Worker
{
	pthread_t thread;
	WaitQueue<T*> input;
	WaitQueue<int> output;
	Timer timer;

	static void* run_thread(void* worker)
	{
		((Worker*)worker)->run();
		return 0;
	}

	void run()
	{
		T* job = 0;
		while (input.pop(job))
		{
			TimeScope ts(timer);
			output.push(job->run());
		}
	}

public:
	Worker() : timer(CLOCK_THREAD_CPUTIME_ID)
	{
		pthread_create(&thread, 0, Worker::run_thread, this);
	}

	~Worker()
	{
		input.stop();
		pthread_join(thread, 0);
		cout << "Worker time: " << timer.elapsed() << endl;
	}

	void request(T& job)
	{
		input.push(&job);
	}

	int done()
	{
		int res = 0;
		output.pop(res);
		return res;
	}
};

#endif /* TOOLS_WORKER_H_ */
