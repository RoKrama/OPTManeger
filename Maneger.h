#pragma once
#include <iostream>

#include <thread>
#include <condition_variable>
#include <barrier>
#include <future>

#include <functional>
#include <vector>

template <typename Fn_type> class OPTManeger
{
	// prevent copy, assign, move
	OPTManeger(OPTManeger&) = delete;
	OPTManeger(OPTManeger&&) = delete;
	OPTManeger& operator = (OPTManeger&) = delete;
	OPTManeger& operator = (OPTManeger&&) = delete;

	// base variables
	const int N_THREADS;
	const int N_DATA;
	std::vector<std::thread> threads;
	std::mutex w_lock;
	std::mutex cv_mtx;
	std::condition_variable c_v;

	std::function<void()> sync_finish_complete;
	std::barrier<std::function<void()>>* sync_finish;

	// helper variables
	std::promise<void> start_promise;
	std::shared_future<void> start_future;
	int data_idx;
	bool processing_done;
	bool can_begin;
	bool stop_threading;

	// worker function pointer or wrapper
	Fn_type worker;

	//init and stop
	void init();
	void stop();

	// destructor
	~OPTManeger();

	// public functions
public:
	OPTManeger(const int, const int, Fn_type&);
	void run();
	void collect();
	void run_and_collect();

};