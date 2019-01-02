#pragma once

#include <gl/glew.h>
#include <GLFW/glfw3.h>
#include <iostream>

#include <stack>
#include <string>
#include <algorithm>

#include <assert.h>

#define TO_STDFUNC(x)\
std::function<void(std::mutex*, WC_Data*)>(x)

typedef unsigned int uint;
typedef unsigned char byte;
struct Point;

//Code from https://en.wikipedia.org/wiki/LU_decomposition
int LUPDecompose(double **A, int N, double Tol, int *P);
void LUPSolve(double **A, int *P, double *b, int N, double *x);
void LUPInvert(double **A, int *P, int N, double **IA);
double LUPDeterminant(double **A, int *P, int N);

void JEACalculate(double* A, int N, double* eigenvectors, double* eigenvalues);

struct WC_Data
{
	size_t size;
	Point* pos_data = nullptr;
	byte* addata;
};
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

struct Vector
{
	Vector(size_t dimension)
	{
		data = (double*)calloc(dimension, sizeof(double));
		dim = dimension;
	}
	~Vector()
	{
		free(data);
	}
	Vector(const Vector &v) : Vector(v.dim)
	{
		for (int i = 0; i < v.dim; i++)
		{
			this->data[i] = v.data[i];
		}
	}

	double* operator[](unsigned int i)
	{
		return &data[i];
	}

	unsigned int dimension() const
	{
		return dim;
	}

	double* data = nullptr;
	size_t dim = 0;
};
//C++ is row-major order -> a_ij = a[i][j]
struct Matrix
{
	Matrix(size_t dimension)
	{
		P = (int*)calloc(dimension, sizeof(double));
		data = (double**)malloc(sizeof(double*) * dimension);
		for (int i = 0; i < dimension; i++)
		{
			//Set data entries to 0;
			data[i] = (double*)calloc(dimension, sizeof(double));
		}

		//LUPDecompose(data, dim, 0.001, P);

		dim = dimension;
	}
	~Matrix()
	{
		free(data);
		free(P);
	}
	Matrix(const Matrix &m) : Matrix(m.dim)
	{
		fillData(m.data, this->data);
	}

	double* operator[](unsigned int i)
	{
		return data[i];
	}

	Matrix operator+(Matrix& rhs)
	{
		assert(this->dim == rhs.dim);
		Matrix nmat = Matrix(rhs.dim);
		for (int i = 0; i < rhs.dim; i++)
		{
			for (int j = 0; j < rhs.dim; j++)
			{
				nmat.data[i][j] = this->data[i][j] + rhs.data[i][j];
			}
		}
		return nmat;
	}

	Matrix operator-(Matrix& rhs)
	{
		assert(this->dim == rhs.dim);
		Matrix nmat = Matrix(rhs.dim);
		for (int i = 0; i < rhs.dim; i++)
		{
			for (int j = 0; j < rhs.dim; j++)
			{
				nmat.data[i][j] = this->data[i][j] - rhs.data[i][j];
			}
		}
		return nmat;
	}

	unsigned int dimension() const
	{
		return dim;
	}

	//Solves A*X = B -> returns X (size N vector)
	static Vector linearSolve(Matrix* A, Vector* B)
	{
		double** ls_data = A->toLU();
		Vector result = Vector(A->dim);
		assert(A->dim == B->dim);

		LUPSolve(ls_data, A->P, B->data, A->dim, result.data);

		free(ls_data);

		return result;
	}

	//Calculates all Eigenvalues & Eigenvectors of A
	static void calcEigenV(Matrix* A)
	{
		const int D = A->dim;
		double** evecs = (double**)malloc(D * sizeof(double*));
		for (int i = 0; i < D; i++)
		{
			evecs[i] = (double*)malloc(D * sizeof(double));
		}
		double*  evals = (double*)malloc(D * sizeof(double));

		JEACalculate(*(A->data), D, *evecs, evals);

		assert(evecs != nullptr && evals != nullptr);

		A->eigenvals = evals;
		A->eigenvecs = evecs;
	}

	double* eigenValues() const
	{
		return eigenvals;
	}

	double** eigenVectors() const
	{
		return eigenvecs;
	}

	double determinant()
	{
		double** det_data = toLU();
		double det = LUPDeterminant(det_data, P, dim);
		free(det_data);
		return det;
	}

	Matrix inverse()
	{
		Matrix inv = Matrix(dim);
		double** inv_data = toLU();
		LUPInvert(inv_data, P, dim, inv.data);
		free(inv_data);
		return inv;
	}

private:

	double** allocateData()
	{
		double** d_data = (double**)malloc(sizeof(double*) * dim);
		for (int i = 0; i < dim; i++)
		{
			//Set data entries to 0;
			d_data[i] = (double*)calloc(dim, sizeof(double));
		}
		return d_data;
	}

	void fillData(double** src, double** dest)
	{
		for (int i = 0; i < dim; i++)
		{
			for (int j = 0; j < dim; j++)
			{
				dest[i][j] = src[i][j];
			}
		}
	}

	//returns a copy array of a LU Matrix of the original data
	double** toLU()
	{
		double** lu_data = allocateData();
		fillData(data, lu_data);
		LUPDecompose(lu_data, dim, 0.001, P);
		return lu_data;
	}

	double* eigenvals = nullptr;
	double** eigenvecs = nullptr;

	double** data = nullptr;
	int* P = nullptr;
	size_t dim = 0;
};

namespace OGLWrapper
{
	/* Outputs a window with certain size and name, ready to be used by OGL context (only callable once) */
	void Initialize(GLFWwindow** window, const Vector2f size, const std::string name);

	/* Creates a Opengl usable program with attached FS and VS */
	GLuint CreateProgram(std::string vs, std::string fs);
}