#pragma once

#include <gl/glew.h>
#include <GLFW/glfw3.h>
#include <iostream>

#define TO_STDFUNC_P(x)\
std::function<void(std::mutex*, void*)>(x)

struct Point
{
	Point() : x(0.0f), y(0.0f) {  }
	Point(GLfloat x, GLfloat y) : x(x), y(y) {  }
	GLfloat x;
	GLfloat y;
};

struct Vector2f
{
	Vector2f()
	{
		x = 0.0f;
		y = 0.0f;
	}
	Vector2f(GLfloat x, GLfloat y)
	{
		this->x = x;
		this->y = y;
	}

	Vector2f operator+(Vector2f other)
	{
		return Vector2f(other.x + this->x, other.y + this->y);
	}

	Vector2f operator-(Vector2f other)
	{
		return Vector2f(-other.x + this->x, -other.y + this->y);
	}
	
	Vector2f operator*(Vector2f other) //Dot product
	{
		return Vector2f(other.x * this->x, other.y * this->y);
	}

	Point operator&()
	{
		return Point(x, y);
	}

	GLfloat x;
	GLfloat y;
};

namespace OGLWrapper
{
	/* Outputs a window with certain size and name, ready to be used by OGL context (only callable once) */
	void Initialize(GLFWwindow** window, const Vector2f size, const std::string name);

	/* Creates a Opengl usable program with attached FS and VS */
	GLuint CreateProgram(std::string vs, std::string fs);
}