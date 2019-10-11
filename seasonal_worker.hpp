/*
* \brief Class for using one thread to asynchronously execute tasks
*
* The class' thread is suspended unless it has been assigned some work. Its destructor causes it to shut down immediately if not working and after finishing all tasks
* otherwise (the list of tasks can be cleared using discardTasks(), in that case, it only finishes its current task).
*/

#ifndef SEASONAL_WORKER_HPP
#define SEASONAL_WORKER_HPP

#include <thread>
#include <mutex>
#include <condition_variable>
#include <vector>
#include <functional>
#include <iostream>

class SeasonalWorker {
	mutable std::vector<std::function<void()>> tasks_;
	mutable std::mutex tasksMutex_;
	mutable std::condition_variable workingCondvar_;
	bool willExit_ = false;
	mutable bool willDiscardTasks_ = false;
	std::thread workerThread_; // Must be initialised last

	inline void seasonallyWork() const {
		std::unique_lock<std::mutex> lock(tasksMutex_, std::defer_lock);
		std::vector<std::function<void()>> currentTasks;
		while (true) {
			for (unsigned int i = 0; i < currentTasks.size(); i++) {
				try {
					currentTasks[i]();
				} catch(std::exception& e) {
					std::cerr << "SeasonalWorker job error: " << e.what() << std::endl;
				} catch(...) {
					std::cerr << "Unknown job error in SeasonalWorker" << std::endl;
				}
				if (willDiscardTasks_) {
					tasks_.clear();
					willDiscardTasks_ = false;
					break;
				}
			}
			currentTasks.clear();
			lock.lock();
			std::swap(tasks_, currentTasks);
			if (currentTasks.empty()) {
				if (willExit_) break;
				workingCondvar_.wait(lock, [this, &currentTasks] () {
					return (willExit_ || !currentTasks.empty());
				});
			}
			lock.unlock();
		}
	}

	inline void unlock() const {
		workingCondvar_.notify_one();
	}

public:
	/*!
	* \brief Constructs the thread and becomes ready to perform the tasks
	*/
	inline SeasonalWorker() : workerThread_(&SeasonalWorker::seasonallyWork, this) {

	}

	/*!
	* \brief Destructor, exits immediately or finishes all tasks (if there are any) and exits
	*/
	inline ~SeasonalWorker() {
		{
			std::lock_guard<std::mutex> guard(tasksMutex_);
			willExit_ = true;
			unlock();
		}
		workerThread_.join();
	}

	/*!
	* \brief Removes all queued tasks
	*
	* \note It may be useful to call this before the destructor
	*/
	inline void discardTasks() const {
		willDiscardTasks_ = true;
	}

	/*!
	* \brief Adds a task
	* \param A callable object representing the task, like a lambda
	*
	* \note The call is thread-safe
	*/
	inline void addTask(const std::function<void()>& task) const {
		std::lock_guard<std::mutex> guard(tasksMutex_);
		tasks_.push_back(task);
		unlock();
	}

	/*!
	* \brief Adds a task
	* \param A callable object representing the task, like a lambda
	*
	* \note The call is thread-safe
	*/
	inline void addTask(std::function<void()>&& task) const {
		std::lock_guard<std::mutex> guard(tasksMutex_);
		tasks_.push_back(task);
		unlock();
	}
};

#endif // SEASONAL_WORKER_HPP
