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
	std::vector<std::function<void()>> tasks_;
	std::mutex tasksMutex_;
	std::mutex workingMutex_;
	std::mutex sleepCheckMutex_;
	std::condition_variable workingCondvar_;
	std::thread workerThread_;
	bool willExit_ = false;
	bool willDiscardTasks_ = false;

	void seasonallyWork() {
		std::unique_lock<std::mutex> workingLock(workingMutex_);
		std::vector<std::function<void()>> currentTasks;
		while (true) {
			for (const std::function<void()>& func : currentTasks) {
				try {
					func();
				} catch(std::exception& e) {
					std::cerr << "SeasonalWorker job error: " << e.what() << std::endl;
				} catch(...) {
					std::cerr << "Unknown job error in SeasonalWorker" << std::endl;
				}
				if (willDiscardTasks_) {
					std::lock_guard<std::mutex> guard(tasksMutex_);
					tasks_.clear();
					willDiscardTasks_ = false;
					break;
				}
			}
			currentTasks.clear();
			{
				std::unique_lock<std::mutex> sleepLock(sleepCheckMutex_);
				{
					std::lock_guard<std::mutex> guard(tasksMutex_);
					std::swap(tasks_, currentTasks);
				}
				if (currentTasks.empty()) {
					if (willExit_) break;
					workingCondvar_.wait(workingLock);
				}
			}
		}
	}

	void unlock() {
		std::unique_lock<std::mutex> sleepLock(sleepCheckMutex_, std::defer_lock);
		if (!sleepLock.try_lock()) {
			std::unique_lock<std::mutex> sleepLock(workingMutex_);
			workingCondvar_.notify_one();
		}
	}

public:
	/*!
	* \brief Constructs the thread and becomes ready to perform the tasks
	* \param The calling period
	* \param The function that is called periodically
	* \param If the thread starts running or is paused until resume() is called
	*/
	SeasonalWorker() : workerThread_(&SeasonalWorker::seasonallyWork, this) {

	}

	/*!
	* \brief Destructor, exits immediately or finishes all tasks (if there are any) and exits
	*/
	~SeasonalWorker() {
		willExit_ = true;
		unlock();
		workerThread_.join();
	}

	/*!
	* \brief Removes all queued tasks
	*
	* \note It may be useful to call this before the destructor
	*/
	void discardTasks() {
		willDiscardTasks_ = true;
	}

	/*!
	* \brief Adds a task
	* \param A callable object representing the task, like a lambda
	*
	* \note The call is thread-safe
	*/
	void addTask(const std::function<void()>& task) {
		{
			std::lock_guard<std::mutex> guard(tasksMutex_);
			tasks_.push_back(task);
		}
		unlock();
	}

	/*!
	* \brief Adds a task
	* \param A callable object representing the task, like a lambda
	*
	* \note The call is thread-safe
	*/
	void addTask(std::function<void()>&& task) {
		{
			std::lock_guard<std::mutex> guard(tasksMutex_);
			tasks_.push_back(task);
		}
		unlock();
	}
};

#endif // SEASONAL_WORKER_HPP
