#pragma once

#include <thread>
#include <chrono>

namespace tofu 
{
	class ScheduledUpdateThread
	{
	public:
		using func_type = std::function<void(ScheduledUpdateThread&)>;

		template<class TFunc>
		ScheduledUpdateThread(std::chrono::system_clock::duration period, const TFunc& func)
			: _started(false)
			, _end(false)
			, _period(period)
			, _func(func)
		{
			_thread = std::thread{ [this]() { Entrypoint(); } };
		}

		~ScheduledUpdateThread() {
			End(true);
		}

		void Start()
		{
			std::lock_guard<std::mutex> lock(_start_mutex);

			_started = true;
			_start_cv.notify_all();
		}

		void End(bool wait = false)
		{
			_end = true;
            if(!_started)
            {
                Start();
            }

            if(wait && _thread.joinable())
            {
                _thread.join();
            }
		}

		bool IsRunning() const
		{
			return _thread.joinable();
		}

	private:
		void Entrypoint()
		{
			{
				std::unique_lock<std::mutex> lock(_start_mutex);
				_start_cv.wait(lock, [&] { return _started; });
			}

			using namespace std::chrono;
			time_point start = system_clock::now();

			time_point next = start;

			while (!_end) 
			{
				auto now = system_clock::now();
				if (now < next)
				{
					std::this_thread::sleep_for(next - now);
					continue;
				}
				else if (_period * 2 < now - next)
				{
					// 二周遅れ以上だから適当にスキップ
					next = now;
				}

				_func(*this);
				next += _period;
			}
		}

		std::thread _thread;
		std::mutex _start_mutex;
		std::condition_variable _start_cv;
		bool _started;
		bool _end;

		// 1Tickあたりの時間
		std::chrono::system_clock::duration _period;
		func_type _func;
	};

}

