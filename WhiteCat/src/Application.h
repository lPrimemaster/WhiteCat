#pragma once
#include "utils.h"
#include <thread>
#include <mutex>
#include <functional>

#define WC_GFUNC 0
#define WC_PFUNC 1

#define WC_GTHREAD 0
#define WC_PTHREAD 1

typedef unsigned int Ftype;
typedef unsigned int Ttype;
typedef unsigned char byte;

typedef std::function<void(std::mutex*, void*)> GeneralFunc;

class Application
{
public:
	Application();
	~Application();

	void setup(Ftype type, GeneralFunc function);
	void startThread(Ttype type);
	void joinThread(Ttype type);


private:
	std::thread* graphicsThread;
	std::thread*  physicsThread;
	std::mutex* mutex;

	GeneralFunc graphicsFunc;
	GeneralFunc  physicsFunc;

	bool ptStarted = false;
	bool gtStarted = false;

	byte* gData;
	byte* pData;
};

