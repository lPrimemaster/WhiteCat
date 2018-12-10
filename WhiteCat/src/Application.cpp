#include "Application.h"

#define SWITCH_T(x, y)\
switch (type)\
{\
case WC_GFUNC:\
	x;\
	break;\
case WC_PFUNC:\
	y;\
	break;\
default:\
	std::clog << "The type id " << type << " is not a valid id." << std::endl;\
	break;\
}\

std::atomic<int> Application::dataFlag(0);


Application::Application()
{
	common = static_cast<WC_Data*>(malloc(sizeof(WC_Data)));
	mutex = new std::mutex();
}


Application::~Application()
{
	if (common->pos_data != nullptr)
		free(common->pos_data);
	if(common != nullptr)
		free(common);

	delete graphicsThread;
	delete  physicsThread;
	delete mutex;
}

void Application::setup(Ftype type, GeneralFunc function)
{
	SWITCH_T(graphicsFunc = function, physicsFunc = function);
}

void Application::startThread(Ttype type)
{
	// Only one isntance of gThread and pThread should run
	if (type == WC_GTHREAD && gtStarted || type == WC_PTHREAD && ptStarted) return;

	SWITCH_T(graphicsThread = new std::thread(std::move(graphicsFunc), mutex, common); gtStarted = true,
		physicsThread = new std::thread(std::move(physicsFunc), mutex, common); ptStarted = true);
}

void Application::joinThread(Ttype type)
{
	SWITCH_T(graphicsThread->join(), physicsThread->join());
}

bool Application::checkReady()
{
	return dataFlag.load(std::memory_order_acquire);
}

void Application::setDataReady()
{
	dataFlag.store(1, std::memory_order_release);
}

void Application::setDataNotReady()
{
	dataFlag.store(0, std::memory_order_release);
}
