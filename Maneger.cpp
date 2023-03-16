#include "Maneger.h"
template <typename Fn_type>
OPTManeger<Fn_type>::OPTManeger(const int n_threads, const int n_data, Fn_type& in_fn) :
	N_THREADS(n_threads),
	N_DATA(n_data),
	data_idx(0),
	threads(),
	w_lock(), cv_mtx(),
	c_v(),
	sync_finish_complete(),
	start_promise(), start_future(),
	sync_finish(nullptr),
	processing_done(false), can_begin(false), stop(false),
	worker(in_fn)
{
	start_future = start_promise.get_future();
	init();
}

template <typename Fn_type>
void OPTManeger<Fn_type>::init()
{

	sync_finish_complete = [this]() mutable noexcept
	{
		std::unique_lock u_lock(cv_mtx);
		processing_done = true; // set flags
		can_begin = false; // prevent from running immedeately again
		data_idx = 0; // reset data "quee"
		u_lock.unlock(); // unlock before notify
		c_v.notify_one(); // notify waiting main we're done
	};

	// sync after completing and wait to collect()
	sync_finish = new std::barrier<std::function<void()>>(N_THREADS, sync_finish_complete);

	
	for (int i = 0; i < N_THREADS; i++)
		threads.push_back(
			std::thread(
				[this]() mutable noexcept
				{
					int thread_data_idx; // for saving data index before it gets mutated 
					start_future.wait(); // wait for main to signal start

					while (true) // running loop
					{
						std::unique_lock<std::mutex> u_lock(cv_mtx);
						c_v.wait(u_lock, [] { return can_begin || stop_threading; }); // wait to get signal to go again or stop
						u_lock.unlock();
						if (stop_threading) break; // stop running
						while (true) // processing loop
						{
							w_lock.lock(); // lock before getting data index
							if ((thread_data_idx = data_idx) < N_DATA) // set and check if we're in data range
							{
								data_idx++; // increment index for next thread
								w_lock.unlock(); // no more mutations of shared variables
								worker(thread_data_idx); // call worker to process data
							}
							else // no more data to compute
							{
								w_lock.unlock();
								break;
							}
						} // processing loop
						sync_finish->arrive_and_wait(); // wait for all threads to come here and signal finish to main
					} // running loop
				}
			)
		);

	start_promise.set_value(); // signal to start
}
template <typename Fn_type>
void OPTManeger<Fn_type>::run_and_collect()
{
	std::unique_lock<std::mutex> u_lock(cv_mtx);
	can_begin = true; // beging processing flag
	u_lock.unlock();
	c_v.notify_all(); // signal

	c_v.wait(u_lock, [] { return processing_done || stop_threading; });

}
template <typename Fn_type>
void OPTManeger<Fn_type>::stop()
{
	std::unique_lock<std::mutex> u_lock(cv_mtx);
	stop_threading = true; // beging processing flag
	u_lock.unlock();
	c_v.notify_all(); // signal
	for (std::thread& thread : threads)
		if (thread.joinable()) thread.join();
}
template <typename Fn_type>
OPTManeger<Fn_type>::~OPTManeger()
{
	stop();
	delete sync_finish;
}