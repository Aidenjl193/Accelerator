#include "TaskManager.h"

namespace Accelerator
{
	// Task Manager stuff

	//Initialize static stuff
	std::vector<TaskThread*> TaskManager::threads;
	std::mutex TaskManager::taskmtx;
	std::map<int, void*> TaskManager::tasks;

	TaskManager::~TaskManager()
	{
		//Clean up threads
		for (auto&& thread : threads)
		{
			delete thread;
		}

	}

	unsigned int TaskManager::ConcurentThreadsSupported()
	{
		//Find out how many threads we can utilize on the current PC
		return(std::thread::hardware_concurrency());
	}

	void TaskManager::InitializeThreads()
	{
		//Initialize threads
		for (int i = 1; i < std::thread::hardware_concurrency(); ++i)
		{
			threads.push_back(new TaskThread());
		}
	}

	int TaskManager::AssignTask(void* t)
	{
		tasks.insert({ (int)t, t }); //Need a better way to generate unique IDs

		//Cycle through each thread in the pool
		for (auto&& thread : threads)
		{
			//If the thread currently isn't running anything; start it
			if (!thread->threadEnabled)
			{
				thread->MakeThread();
				return (int)t;
			}
		}

		return (int)t; //Returns a handle to the task
	}

	void TaskManager::Join(int id)
	{
		TaskManager::taskmtx.lock(); //Make a copy of tasks so that we don't hold up any threads on the mutex
		std::map<int, void*> ts = tasks;
		TaskManager::taskmtx.unlock();

		while (true)
		{
			//Figure out if the task has been finished or not
			for (auto&& task : ts)
			{
				if (task.first == id)
					continue; //The task hasn't even been started yet
			}
			for (auto&& thread : threads)
			{
				if (thread->taskID == id)
					continue; //A thread is still working on the task
			}
			//If we made it this far we can conclude that the task has been finished
			return;
		}
	}

	//Task Thread stuff

	TaskThread::~TaskThread()
	{
		t.join();
	}

	void TaskThread::Work()
	{
		threadEnabled = true;

		//The main logic of our threads
		while (threadEnabled)
		{
			for (int i = 0; i < TaskManager::tasks.size(); ++i)
			{
				TaskManager::taskmtx.lock();

				if (TaskManager::tasks.size() == 0) //Make sure another thread hasn't claimed the task
				{
					TaskManager::taskmtx.unlock();
					threadEnabled = false;
					return;
				}

				void* func = TaskManager::tasks.begin()->second;
				taskID = TaskManager::tasks.begin()->first; //We do this so that the join function can work
				TaskManager::tasks.erase(TaskManager::tasks.begin());
				TaskManager::taskmtx.unlock();

				((void(*)(void)) func)();//Run the pointer to the function

				taskID = 0;
			}

			//If there are no tasks kill the thread so we don't hog the CPU
			if (TaskManager::tasks.size() == 0)
			{
				threadEnabled = false;
				return;
			}
		}
	}

	void TaskThread::MakeThread()
	{
		if (t.joinable()) // Shouldn't really happen
		{
			t.join();
		}
		t = std::thread([&](){Work(); });
	}
}