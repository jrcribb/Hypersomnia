#pragma once
#include <mutex>
#include <atomic>
#include <condition_variable>

struct game_frame_buffer {
	augs::local_entropy new_window_entropy;
	render_command_buffer commands;
	augs::window_settings new_settings;

	bool should_clip_cursor = false;
	bool should_pause_cursor = false;
	bool should_quit = false;
	vec2i screen_size;
};

class game_frame_buffer_swapper {
	std::atomic<bool> already_waiting = false;
	std::atomic<bool> swap_complete = false;

	std::mutex swapper_m;
	std::mutex waiter_m;

	std::condition_variable swapper_cv;
	std::condition_variable waiter_cv;

public:
	template <class T>
	void swap_buffers(
		game_frame_buffer& read_buffer, 
		game_frame_buffer& write_buffer,
		T&& synchronized_op
	) {
		{
			std::unique_lock<std::mutex> lk(swapper_m);
			swapper_cv.wait(lk, [this]{ return already_waiting.load(); });
		}


		{
			/* The other thread MUST be waiting while in this block */
			std::swap(read_buffer, write_buffer);
			already_waiting.store(false);

			synchronized_op();
		}

		notify_swap_completion();
	}

	void notify_swap_completion() {
		{
			std::unique_lock<std::mutex> lk(waiter_m);
			swap_complete.store(true);
		}

		waiter_cv.notify_all();
	}

	void wait_swap() {
		swap_complete.store(false);
		
		{
			std::unique_lock<std::mutex> lk(swapper_m);
			already_waiting.store(true);
		}

		swapper_cv.notify_all();

		{
			std::unique_lock<std::mutex> lk(waiter_m);
			waiter_cv.wait(lk, [this]{ return swap_complete.load(); });
		}
	}
};

