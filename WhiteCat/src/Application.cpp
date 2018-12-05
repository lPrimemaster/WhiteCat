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


Application::Application()
{

}


Application::~Application()
{
	if(gData != nullptr)
		free(gData);
	if (pData != nullptr)
		free(pData);

	delete graphicsThread;
	delete  physicsThread;
}

void Application::setup(Ftype type, GeneralFunc function)
{
	SWITCH_T(graphicsFunc = function, physicsFunc = function);
}

void Application::startThread(Ttype type)
{
	// Only one isntance of gThread and pThread should run
	if (type == WC_GTHREAD && gtStarted || type == WC_PTHREAD && ptStarted) return;

	SWITCH_T(graphicsThread = new std::thread(std::move(graphicsFunc), mutex, gData); gtStarted = true,
		physicsThread = new std::thread(std::move(physicsFunc), mutex, pData); ptStarted = true);
}

void Application::joinThread(Ttype type)
{
	SWITCH_T(graphicsThread->join(), physicsThread->join());
}
