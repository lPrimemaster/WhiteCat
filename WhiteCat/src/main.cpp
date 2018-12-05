#include <iostream>
#include "Application.h"
#include "utils.h"

void graphicsThread(std::mutex* mtx, void* data)
{
	GLFWwindow* window = nullptr;
	OGLWrapper::Initialize(&window, Vector2f(1280, 720), "Window");

	while (!glfwWindowShouldClose(window))
	{
		glfwPollEvents();
		glfwSwapBuffers(window);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	}

	glfwDestroyWindow(window);

	printf("GThread!\n");
}

void physicsThread(std::mutex* mtx, void* data)
{
	printf("PThread!\n");
}

int main(int argc, char* argv[])
{
	Application app;

	app.setup(WC_GFUNC, TO_STDFUNC_P(graphicsThread));
	app.setup(WC_PFUNC, TO_STDFUNC_P(physicsThread));

	app.startThread(WC_GTHREAD);
	app.startThread(WC_PTHREAD);

	app.joinThread(WC_GTHREAD);
	app.joinThread(WC_PTHREAD);

	return 0;
}