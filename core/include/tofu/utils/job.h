#pragma once

#include <functional>
#include <typeindex>
#include <vector>
#include <ranges>
#include <set>
#include <cassert>

// 超簡易な、依存関係を解決するシングルスレッドジョブスケジューラ
namespace tofu
{
    using job_tag = std::type_index;
    using condition_tag = std::type_index;

    template<class T>
    job_tag get_job_tag() noexcept
    {
        return job_tag{ typeid(T) };
    }

    template<class T>
    condition_tag get_condition_tag() noexcept
    {
        return condition_tag{ typeid(T) };
    }

    class Job
    {
    public:
        using task_t = std::function<std::optional<condition_tag>()>;
        Job(job_tag tag, std::initializer_list<job_tag> dependency, std::initializer_list<condition_tag> conditions, const task_t& task)
            : _tag(tag)
            , _dependency(dependency)
            , _conditions(conditions)
            , _task(task)
        {
        }

        void Reset() noexcept
        {
            _done = false;
        }

        std::optional<condition_tag> Run()
        {
            std::optional<condition_tag> ret;
            if(!_done)
                ret = _task();
            _done = true;

            return ret;
        }

        void SetAsDone()
        {
            _done = true;
        }

        bool IsDone() const noexcept
        {
            return _done;
        }

        void AddDependency(job_tag tag)
        {
            _dependency.push_back(tag);
        }

        void AddCondition(condition_tag tag)
        {
            _conditions.push_back(tag);
        }

        const std::vector<job_tag>& GetDependency() const noexcept
        {
            return _dependency;
        }

        const std::vector<condition_tag>& GetConditions() const noexcept
        {
            return _conditions;
        }

        job_tag GetTag() const noexcept
        {
            return _tag;
        }

    private:
        job_tag    _tag;
        task_t _task;
        std::vector<job_tag> _dependency;
        std::vector<condition_tag> _conditions;

        bool _done = false;
    };

    template<class T, class... TArgs>
    std::shared_ptr<Job> make_job(std::initializer_list<job_tag> dependency, std::initializer_list<condition_tag> conditions, TArgs&&... args)
    {
        if constexpr (std::is_convertible_v<T, Job::task_t>)
        {
			return std::make_shared<Job>(get_job_tag<T>(), dependency, conditions, T{ std::forward<TArgs>(args)... });
        }
        else
        {
            auto task = T{ std::forward<TArgs>(args)... };
            return std::make_shared<Job>(get_job_tag<T>(), dependency, conditions, [task]() { task(); return std::nullopt; });
        }
    }

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

        std::shared_ptr<Job> GetJob(job_tag tag)
        {
            if (auto it = std::ranges::find_if(_jobs, [tag](std::shared_ptr<Job>& job) { return job->GetTag() == tag; }); it != _jobs.end())
                return (*it);
            return nullptr;
        }

        void Run()
        {
            for (auto& job : _jobs)
            {
                job->Reset();
            }
            _finished.clear();
            _condition_flags.clear();

            while(_finished.size() != _jobs.size())
            {
                bool executed = false;
                for (auto& job : _jobs)
                {
                    if (job->IsDone())
                        continue;
                    if (!CanExecute(*job))
                        continue;

                    if (SatisfyCondition(*job))
                    {
						auto flag = job->Run();
                        if (flag)
                        {
                            _condition_flags.insert(*flag);
                        }
                    }
                    else
                    {
                        job->SetAsDone();
                    }

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

        bool SatisfyCondition(const Job& job)
        {
            for (auto condition : job.GetConditions())
            {
                if (_condition_flags.find(condition) == _condition_flags.end())
                    return false;
            }
            return true;
        }

    private:
        std::vector<std::shared_ptr<Job>> _jobs;
        std::set<job_tag> _finished;
        std::set<job_tag> _condition_flags;
    };
}
