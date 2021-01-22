#pragma once

#include <functional>
#include <typeindex>
#include <vector>
#include <ranges>
#include <set>
#include <cassert>

namespace tofu
{
	template<class T>
	using job_tag = std::type_index{ typeid(T) };

	class Job
	{
	public:
		using task_t = std::function<void()>;
		Job(std::type_index tag, std::initializer_list<std::type_index> dependency, const task_t& task)
			: _tag(tag)
			, _dependency(dependency)
			, _task(task)
		{
		}

		void Reset() noexcept
		{
			_done = false;
		}

		void Run()
		{
			if(!_done)
				_task();
			_done = true;
		}

		bool IsDone() const noexcept
		{
			return _done;
		}

		const std::vector<std::type_index>& GetDependency() const noexcept
		{
			return _dependency;
		}

		std::type_index GetTag() const noexcept
		{
			return _tag;
		}

	private:
		std::type_index	_tag;
		task_t _task;
		std::vector<std::type_index> _dependency;

		bool _done = false;
	};

	class JobScheduler
	{
	public:
		void Register(const std::shared_ptr<Job>& job)
		{
			_jobs.push_back(job);
		}
		void Unregister(const std::shared_ptr<Job>& job)
		{
			if (auto it = std::find(_jobs.begin(), _jobs.end(), job); it != _jobs.end())
				_jobs.erase(it);
		}

		void Run()
		{
			for (auto& job : _jobs)
			{
				job->Reset();
			}
			_finished.clear();

			while(_finished.size() == _jobs.size())
			{
				bool executed = false;
				for (auto& job : _jobs)
				{
					if (job->IsDone())
						continue;
					if (!CanExecute(*job))
						continue;

					job->Run();
					executed = true;
					_finished.insert(job->GetTag());
					break;
				}

				// 1つもタスクを実行できなかったのはおかしい
				assert(executed);
				if (!executed)
				{
					break;
				}
			}
		}

	private:
		bool CanExecute(const Job& job)
		{
			for (auto depend_to : job.GetDependency())
			{
				if (_finished.find(depend_to) == _finished.end())
					return false;
			}
			return true;
		}

	private:
		std::vector<std::shared_ptr<Job>> _jobs;
		std::set<std::type_index> _finished;
	};
}
