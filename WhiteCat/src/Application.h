#pragma once
#include "utils.h"
#include <thread>
#include <mutex>
#include <functional>
#include <atomic>

#define WC_GFUNC 0
#define WC_PFUNC 1

#define WC_GTHREAD 0
#define WC_PTHREAD 1

typedef unsigned int Ftype;
typedef unsigned int Ttype;

typedef std::function<void(std::mutex*, WC_Data*)> GeneralFunc;

class Application
{
public:
	Application();
	~Application();

	void setup(Ftype type, GeneralFunc function);
	void startThread(Ttype type);
	void joinThread(Ttype type);

	static bool checkReady();
	static void setDataReady();
	static void setDataNotReady();


private:
	std::thread* graphicsThread;
	std::thread*  physicsThread;
	std::mutex* mutex;

	GeneralFunc graphicsFunc;
	GeneralFunc  physicsFunc;

	bool ptStarted = false;
	bool gtStarted = false;

	WC_Data* common;
	static std::atomic<int> dataFlag;
};

