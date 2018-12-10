#include <iostream>
#include <assert.h>
#include "Application.h"
#include "utils.h"

void graphicsThread(std::mutex* mtx, WC_Data* data)
{
	GLFWwindow* window = nullptr;
	OGLWrapper::Initialize(&window, Vector2f(1280, 720), "Window");

	GLuint program = OGLWrapper::CreateProgram("shader", "shader");

	glUseProgram(program);

	GLuint vao, vbo;

	glCreateVertexArrays(1, &vao);
	glCreateBuffers(1, &vbo);

	const unsigned int count = 3000;

	

	while (!Application::checkReady())
	{
		std::cout << "Waiting for data... " << std::endl;
		std::this_thread::yield();
	}

	assert(data->pos_data != nullptr);
	//Data ok -> proceed

	std::cout << "Data arrived and is healthy, proceeding..." << std::endl;

	glBindVertexArray(vao);
	glBindBuffer(GL_ARRAY_BUFFER, vbo);

	glBufferData(GL_ARRAY_BUFFER, data->size * sizeof(Point), (GLvoid*)(data->pos_data), GL_DYNAMIC_DRAW);

	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, nullptr);

	glBindVertexArray(0);

	while (!glfwWindowShouldClose(window))
	{
		while (!Application::checkReady())
		{
			std::cout << "Data changed. Waiting..." << std::endl;
			std::this_thread::yield();
		}

		mtx->lock();

		glBindBuffer(GL_ARRAY_BUFFER, vbo);
		glBufferData(GL_ARRAY_BUFFER, data->size * sizeof(Point), (GLvoid*)(data->pos_data), GL_DYNAMIC_DRAW);
		glBindBuffer(GL_ARRAY_BUFFER, 0);

		glBindVertexArray(vao);
		glDrawArrays(GL_LINE_STRIP, 0, data->size);
		glBindVertexArray(0);

		mtx->unlock();

		glfwPollEvents();
		glfwSwapBuffers(window);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	}

	glfwDestroyWindow(window);
	glDeleteProgram(program);
	glDeleteVertexArrays(1, &vao);
	glDeleteBuffers(1, &vbo);

	printf("GThread!\n");
}

void physicsThread(std::mutex* mtx, WC_Data* data)
{
	data->pos_data = static_cast<Point*>(calloc(sizeof(Point), 3000));
	data->size = 3000;

	//Issue that data is ok
	Application::setDataReady();
	std::this_thread::sleep_for(std::chrono_literals::operator""ms((unsigned long long)2000));
	//Application::setDataNotReady();
	//std::this_thread::sleep_for(std::chrono_literals::operator""ms((unsigned long long)20));

	static float t = 0.1f;
	static float min = -1.0;
	static float max = 1.0;

	//Make this thread increment t by one every real second...
	while (true)
	{
		mtx->lock();
		for (int i = 0; i < data->size; i++)
		{
			data->pos_data[i].x = (float)i / (data->size - 1);
			data->pos_data[i].y = 0.5 * (cos(0.03 * i - 10 * t) + cos(0.036 * i - 50 * t));
			data->pos_data[i].y = (data->pos_data[i].y - min) * (2.0 / (max - min)) - 1.0;
		}

		t += 0.001f;
		mtx->unlock();

		std::this_thread::sleep_for(std::chrono_literals::operator""ms((unsigned long long)10));

		
	}
	
	std::this_thread::sleep_for(std::chrono_literals::operator""ms((unsigned long long)2000));
	Application::setDataReady();

	/* 
	To implement: 
		ODE solving for various potentials (Analytical)
		ODE solving for time dependent potentials (Approximation Methods)
	*/
	printf("PThread!\n");
}

int main(int argc, char* argv[])
{
	Application app;

	app.setup(WC_GFUNC, TO_STDFUNC(graphicsThread));
	app.setup(WC_PFUNC, TO_STDFUNC( physicsThread));

	app.startThread(WC_GTHREAD);
	std::this_thread::sleep_for(std::chrono_literals::operator""ms((unsigned long long)2000));
	app.startThread(WC_PTHREAD);

	app.joinThread(WC_GTHREAD);
	app.joinThread(WC_PTHREAD);

	return 0;
}