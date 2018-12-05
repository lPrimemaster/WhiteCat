#include <iostream>
#include "Application.h"
#include "utils.h"

void graphicsThread(std::mutex* mtx, void* data)
{
	GLFWwindow* window = nullptr;
	OGLWrapper::Initialize(&window, Vector2f(1280, 720), "Window");

	GLuint program = OGLWrapper::CreateProgram("shader", "shader");

	glUseProgram(program);

	GLuint vao, vbo;

	glCreateVertexArrays(1, &vao);
	glCreateBuffers(1, &vbo);

	const unsigned int count = 3000;

	Point* pdata = (Point*)malloc(sizeof(Point) * count);

	static float t = 0.1f;
	static float min = -1.0;
	static float max = 1.0;

	glBindVertexArray(vao);
	glBindBuffer(GL_ARRAY_BUFFER, vbo);

	glBufferData(GL_ARRAY_BUFFER, count * sizeof(Point), pdata, GL_DYNAMIC_DRAW);

	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, nullptr);

	glBindVertexArray(0);

	while (!glfwWindowShouldClose(window))
	{

		for (int i = 0; i < count; i++)
		{
			pdata[i].x = (float)i / (count - 1);
			pdata[i].y = 0.5 * (cos(0.03 * i - 10 * t) + cos(0.036 * i - 50 * t));
			pdata[i].y = (pdata[i].y - min) * (2.0 / (max - min)) - 1.0;
		}

		t += 0.001f;

		glBindBuffer(GL_ARRAY_BUFFER, vbo);
		glBufferData(GL_ARRAY_BUFFER, count * sizeof(Point), pdata, GL_DYNAMIC_DRAW);
		glBindBuffer(GL_ARRAY_BUFFER, 0);

		glBindVertexArray(vao);
		glDrawArrays(GL_LINE_STRIP, 0, count);
		glBindVertexArray(0);

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