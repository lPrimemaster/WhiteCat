#include <iostream>
#include <assert.h>
#include <Windows.h>

#include <chrono>

#define GLEW_STATIC

#include "Application.h"
#include "utils.h"
#include "Math/Evaluator.h"
#include "Math/Solver.h"


#define DEBUG

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

	
	std::cout << "Waiting for data... " << std::endl;
	while (!Application::checkReady())
	{
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

	printf("GThread exited!\n");
}

void physicsThread(std::mutex* mtx, WC_Data* data)
{
	data->pos_data = static_cast<Point*>(calloc(sizeof(Point), 3000));
	data->size = 3000;


	//Issue that data is ok
	Application::setDataReady();

	static float t = 0.1f;
	static float min = -1.0f;
	static float max = 1.0f;

	std::chrono::time_point<std::chrono::system_clock> start = std::chrono::system_clock::now();
	/*for (int i = 0; i < data->size; i++)
	{
		float x = (float)i / (data->size - 1);
		od[i] = f(x);
	}*/

	auto pot = [](double x) -> double { return 0; }; // U(x) = 0, All x

	Vector eigenvectors = Solver::FDM(1.0, 100, pot);

	std::chrono::time_point<std::chrono::system_clock> end = std::chrono::system_clock::now();

	uint elapsed_ns = std::chrono::duration_cast<std::chrono::nanoseconds>(end - start).count();
	uint elapsed_us = std::chrono::duration_cast<std::chrono::microseconds>(end - start).count();
	uint elapsed_ms = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();

	std::cout << "Evaluation Time [Line " << __LINE__ << "] (ns): " << elapsed_ns << " | (us): " << elapsed_us << " | (ms): " << elapsed_ms << std::endl;

	//Make this thread increment t by one every real second...
	while (true)
	{
		mtx->lock();
		for (int i = 0; i < data->size; i++)
		{
			data->pos_data[i].x = (float)i / (data->size - 1);
			data->pos_data[i].y = *eigenvectors[i];/*0.5 * (cos(0.03 * i - 10 * t) + cos(0.036 * i - 50 * t))*/;
			data->pos_data[i].y = (data->pos_data[i].y - min) * (2.0 / (max - min)) - 1.0;
		}

		t += 0.001f;
		mtx->unlock();

		std::this_thread::sleep_for(std::chrono_literals::operator""ms((unsigned long long)10));
		
	}

	/* 
	To implement: 
		ODE solving for various potentials (Analytical)
		ODE solving for time dependent potentials (Approximation Methods)
	*/
	printf("PThread exited!\n");
}

void run()
{
	Matrix m(3); //3x3 mat
	Vector v(3); //vec 3

	*v[0] = 1; *v[1] = 2; *v[3] = 0;

	m[0][0] = 1; m[0][1] = 2; m[0][2] = 3;
	m[1][0] = 2; m[1][1] = 1; m[1][2] = 2;
	m[2][0] = 3; m[2][1] = 2; m[2][2] = 1;

	for (int i = 0; i < 3; i++)
	{
		printf("%lf %lf %lf\n", m[i][0], m[i][1], m[i][2]);
	}

	putchar('\n');

	double det = m.determinant();
	Matrix m2 = m.inverse();
	printf("Det: %f\n", det);
	putchar('\n');


	std::chrono::time_point<std::chrono::system_clock> start = std::chrono::system_clock::now();
	Vector res = Matrix::linearSolve(&m, &v);
	std::chrono::time_point<std::chrono::system_clock> end = std::chrono::system_clock::now();

	uint elapsed_ns = std::chrono::duration_cast<std::chrono::nanoseconds>(end - start).count();
	uint elapsed_us = std::chrono::duration_cast<std::chrono::microseconds>(end - start).count();
	uint elapsed_ms = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();

	std::cout << "Evaluation Time [Line " << __LINE__ << "] (ns): " << elapsed_ns << " | (us): " << elapsed_us << " | (ms): " << elapsed_ms << std::endl;

	printf("Result A*X = B:  %lf %lf %lf\n", *res[0], *res[1], *res[2]);

	putchar('\n');

	for (int i = 0; i < 3; i++)
	{
		printf("%lf %lf %lf\n", m2[i][0], m2[i][1], m2[i][2]);
	}
	putchar('\n');
	for (int i = 0; i < 3; i++)
	{
		printf("%lf %lf %lf\n", m[i][0], m[i][1], m[i][2]);
	}

	Application app;

	app.setup(WC_GFUNC, TO_STDFUNC(graphicsThread));
	app.setup(WC_PFUNC, TO_STDFUNC( physicsThread));

	app.startThread(WC_GTHREAD);
	app.startThread(WC_PTHREAD);

	app.joinThread(WC_GTHREAD);
	app.joinThread(WC_PTHREAD);
}

#ifdef DEBUG
int main(int argc, char* argv[])
{
	run();
	return 0;
}
#else
int WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PSTR lpCmdLine, INT nCmdShow)
{
	run();
	return 0;
}
#endif
