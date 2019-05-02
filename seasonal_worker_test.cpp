#include <iostream>
#include "seasonal_worker.hpp"

int main()
{
	std::cout << "Test1" << std::endl;
	{
		const SeasonalWorker worker;

		std::cout << "Test 1" << std::endl;
		worker.addTask([] () {
			std::this_thread::sleep_for(std::chrono::milliseconds(200));
			std::cout << "First task done" << std::endl;
		});
		std::this_thread::sleep_for(std::chrono::milliseconds(1000));
		std::cout << "Waiting done" << std::endl;

		std::cout << "Test 2" << std::endl;
		worker.addTask([] () {
			std::this_thread::sleep_for(std::chrono::milliseconds(200));
			std::cout << "First task done" << std::endl;
		});
		worker.addTask([] () {
			std::this_thread::sleep_for(std::chrono::milliseconds(200));
			std::cout << "Second task done" << std::endl;
		});
		std::this_thread::sleep_for(std::chrono::milliseconds(1000));
		std::cout << "Waiting done" << std::endl;

		std::cout << "Test 3" << std::endl;
		worker.addTask([] () {
			std::this_thread::sleep_for(std::chrono::milliseconds(200));
			std::cout << "First task done" << std::endl;
		});
		std::this_thread::sleep_for(std::chrono::milliseconds(100));
		worker.addTask([] () {
			std::this_thread::sleep_for(std::chrono::milliseconds(200));
			std::cout << "Second task done" << std::endl;
		});
		std::this_thread::sleep_for(std::chrono::milliseconds(1000));
		std::cout << "Waiting done" << std::endl;

		std::cout << "Test 4" << std::endl;
		for (unsigned int i = 0; i < 10; i++) {
			worker.addTask([i] () {
				std::this_thread::sleep_for(std::chrono::milliseconds(200));
				std::cout << "Task " << i << " done" << std::endl;
			});
		}
		std::cout << "Tasks added" << std::endl;
		std::this_thread::sleep_for(std::chrono::milliseconds(1000));
		std::cout << "Waiting done" << std::endl;
		worker.discardTasks();
	}
	std::cout << "Destroyed" << std::endl;
	return 0;
}
