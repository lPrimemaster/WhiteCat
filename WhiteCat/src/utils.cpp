#include "utils.h"
#include <iostream>
#include <vector>
#include <fstream>
#include <sstream>

std::string getSource(const std::string& sourceFile, const std::string& type);
GLuint compileShader(const GLchar* source, GLenum type);
GLuint cProgram(GLuint vertexShader, GLuint fragmentShader);

void OGLWrapper::Initialize(GLFWwindow ** window, const Vector2f size, const std::string name)
{
	//Init glfw 3
	if (!glfwInit())
	{
		std::clog << "Glfw failed to initialize." << std::endl;
	}
	else
	{
		std::clog << "Glfw initialized successfully." << std::endl;
	}

	//Set opengl version to 450
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 5);
	glfwWindowHint(GLFW_SAMPLES, 4);

	//Create a window on the default monitor
	*window = glfwCreateWindow(size.x, size.y, name.c_str(), NULL, NULL);

	//Make this the context
	glfwMakeContextCurrent(*window);
	glfwSwapInterval(1); //Explicitly enable vsync

	//Init glew experimental
	glewExperimental = true;
	GLenum status = glewInit();
	if (status != GLEW_OK)
	{
		std::clog << "Glew failed to initialize - Cause: " << glewGetErrorString(status) << std::endl;
	}
	else
	{
		std::clog << "Glew initialized successfully - VERSION " << glewGetString(GLEW_VERSION) << std::endl;
	}

	//Explicitly set the glDepth func to GL_LESS
	glDepthFunc(GL_LESS);
	glEnable(GL_MULTISAMPLE);
	glEnable(GL_DEPTH_TEST);

	glEnable(GL_CULL_FACE);
	glCullFace(GL_BACK);

	//This is modifialble later...
	glLineWidth(2.0f);
	glPointSize(2.0f);
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f); //Clear to black
}

GLuint OGLWrapper::CreateProgram(std::string vs, std::string fs)
{
	std::string vSource, fSource;
	try
	{
		vSource = getSource(vs, "vert");
		fSource = getSource(fs, "frag");
	}
	catch (std::runtime_error& e)
	{
		std::cout << e.what() << std::endl;
	}

	GLuint vsID, fsID;
	try
	{
		vsID = compileShader(vSource.c_str(), GL_VERTEX_SHADER);
		fsID = compileShader(fSource.c_str(), GL_FRAGMENT_SHADER);
	}
	catch (std::runtime_error& e)
	{
		std::cout << e.what() << std::endl;
		exit(-1);
	}
	GLuint programID;
	try
	{
		programID = cProgram(vsID, fsID);
	}
	catch (std::runtime_error& e)
	{
		std::cout << e.what() << std::endl;
	}

	glDeleteShader(vsID);
	glDeleteShader(fsID);

	return programID;
}

std::string getSource(const std::string& sourceFile, const std::string& type)
{
	std::ifstream infile("shaders/" + sourceFile + "." + type + ".glsl");
	std::string source;
	std::stringstream stringStream;

	if (!infile.is_open())
	{
		throw std::runtime_error("Couldn't open shader source: " + sourceFile);
	}

	stringStream << infile.rdbuf();
	source = stringStream.str();

	return source;
}

GLuint compileShader(const GLchar* source, GLenum type)
{
	GLuint shader = glCreateShader(type);
	glShaderSource(shader, 1, &source, nullptr);
	glCompileShader(shader);

	GLint status;
	GLchar infolog[512];

	glGetShaderiv(shader, GL_COMPILE_STATUS, &status);
	if (!status)
	{
		glGetShaderInfoLog(shader, 512, nullptr, infolog);
		throw std::runtime_error("Error compiling shader: " + std::string(infolog));
	}

	return shader;
}

GLuint cProgram(GLuint vertexShader, GLuint fragmentShader)
{
	GLuint program = glCreateProgram();

	glAttachShader(program, vertexShader);
	glAttachShader(program, fragmentShader);

	glLinkProgram(program);

	GLint status;
	GLchar infolog[512];

	glGetProgramiv(program, GL_LINK_STATUS, &status);
	if (!status)
	{
		glGetProgramInfoLog(program, 512, nullptr, infolog);
		throw std::runtime_error("Error linking program: " + std::string(infolog));
	}

	return program;
}
