#pragma once
#include <iostream>

#include <thread>
#include <condition_variable>
#include <barrier>
#include <future>
#include <atomic>

#include <functional>
#include <vector>
#include <type_traits>
namespace opt
{
	template <typename Fn_type> class gates;

	template <typename Fn_type> class OPTManeger
	{
		// prevent copy, assign, move
		OPTManeger(const OPTManeger&) = delete;
		OPTManeger(OPTManeger&&) = delete;
		OPTManeger& operator = (const OPTManeger&) = delete;
		OPTManeger& operator = (OPTManeger&&) = delete;

		// base variables
		const int N_THREADS;
		const int N_DATA;
		std::vector<std::thread> threads;
		std::mutex finish_lock;
		std::mutex start_lock;
		std::condition_variable c_v;
		gates<std::function<void()>>* sync_begin;
		gates<std::function<void()>>* sync_finish;

		std::function<void()> sync_finish_complete;

		// helper variables
		std::promise<void> start_promise;
		std::shared_future<void> start_future;
		std::atomic<int> data_idx;
		bool processing_done;
		bool can_begin;
		bool stop_threading;

		// worker function pointer or wrapper
		Fn_type worker;

		//init and stop
		void init();
		void stop();


		// public functions
	public:
		OPTManeger(const int , const int , Fn_type&);
		//void run();
		//void collect();
		void run_and_collect();

		// destructor
		~OPTManeger();

	};

	// poor man's implementation of std::barrier
	// as for the hell of me I can't make it work as expected
	template <typename Fn_type> class gates
	{
		const int N_EXPECTED;
		std::atomic<int> arrived_count;
		bool completion_fn_done;
		Fn_type complete;
		std::condition_variable c_v;
		std::mutex thread_lock;
	public:
		gates(): N_EXPECTED(0) {};
		gates(int, Fn_type);
		void arrive_and_wait();
	};
}