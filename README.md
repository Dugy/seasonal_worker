# Seasonal Worker
It happens often that you need some functions to be executed asynchronously *and* you don't need the return values any time soon. It can be dealt with using a `std::thread`, but it can be annoying to have a thread for each function call. This is a situation for Seasonal Worker. It is a class that manages a thread that asynchronously executes functors it's added from any thread.

The thread executes the functors if some are not done and passively waits if it has no functors to execute. When not working, there is no polling for work and no timeouts to wait for when functors are added or destructor is called.

## Usage
The class follows the RAII principle, with the only method besides constructor and destructor needed for use being `addTask()`, which takes a functor that can be called with no arguments and no return value expected (which can be a lambda) and calls it on the separate thread. To hasten its exit, it's possible to call `discardTasks()` which makes the worker delete all tasks after finishing the current one (if doing anything).

```C++
	{
		std::cout << "Starting" << std::endl;
		SeasonalWorker worker;
    
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
		worker.addTask([] () {
			std::this_thread::sleep_for(std::chrono::milliseconds(200));
			std::cout << "Second task done" << std::endl;
		});
		std::cout << "Destroying worker" << std::endl;
	}
	std::cout << "Worker destroyed" << std::endl;
```
