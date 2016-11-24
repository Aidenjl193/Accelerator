#pragma once
#include <cstdlib>
#include <thread>
#include <vector>
#include <iostream>
#include <map>
#include <mutex>

namespace Accelerator
{
	class TaskThread
	{
	private:
		std::thread t;

	public:
		//Constructors and de-constructors
		~TaskThread();

		//Variables

		volatile bool threadEnabled = false;

		volatile int taskID = 0;

		//Member functions
		void MakeThread();

		void Work();
	};


	class TaskManager
	{
	public:
		//Deconstructor
		~TaskManager();

		//Variables

		//Will hold all of the threads we can use

		static std::vector<TaskThread*> threads;

		static std::map<int, void*> tasks;

		static std::mutex taskmtx;

		//Member functions

		static void InitializeThreads(int amt);

		static void ResizePool(int newSize);

		static int AssignTask(void* t);

		static void Join(int id);
	};
}