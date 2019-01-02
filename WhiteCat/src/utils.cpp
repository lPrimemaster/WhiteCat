#include "utils.h"
#include <iostream>
#include <vector>
#include <fstream>
#include <sstream>

#include <string>

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

//Code from https://en.wikipedia.org/wiki/LU_decomposition
/* INPUT: A - array of pointers to rows of a square matrix having dimension N
 *        Tol - small tolerance number to detect failure when the matrix is near degenerate
 * OUTPUT: Matrix A is changed, it contains both matrices L-E and U as A=(L-E)+U such that P*A=L*U.
 *        The permutation matrix is not stored as a matrix, but in an integer vector P of size N+1
 *        containing column indexes where the permutation matrix has "1". The last element P[N]=S+N,
 *        where S is the number of row exchanges needed for determinant computation, det(P)=(-1)^S
 */
int LUPDecompose(double **A, int N, double Tol, int *P)
{
	int i, j, k, imax;
	double maxA, *ptr, absA;

	for (i = 0; i <= N; i++)
		P[i] = i; //Unit permutation matrix, P[N] initialized with N

	for (i = 0; i < N; i++) {
		maxA = 0.0;
		imax = i;

		for (k = i; k < N; k++)
			if ((absA = fabs(A[k][i])) > maxA) {
				maxA = absA;
				imax = k;
			}

		if (maxA < Tol) return 0; //failure, matrix is degenerate

		if (imax != i) {
			//pivoting P
			j = P[i];
			P[i] = P[imax];
			P[imax] = j;

			//pivoting rows of A
			ptr = A[i];
			A[i] = A[imax];
			A[imax] = ptr;

			//counting pivots starting from N (for determinant)
			P[N]++;
		}

		for (j = i + 1; j < N; j++) {
			A[j][i] /= A[i][i];

			for (k = i + 1; k < N; k++)
				A[j][k] -= A[j][i] * A[i][k];
		}
	}
	
	return 1; //decomposition done 
}

/* INPUT: A,P filled in LUPDecompose; b - rhs vector; N - dimension
 * OUTPUT: x - solution vector of A*x=b
 */
void LUPSolve(double **A, int *P, double *b, int N, double *x)
{
	for (int i = 0; i < N; i++) {
		x[i] = b[P[i]];

		for (int k = 0; k < i; k++)
			x[i] -= A[i][k] * x[k];
	}

	for (int i = N - 1; i >= 0; i--) {
		for (int k = i + 1; k < N; k++)
			x[i] -= A[i][k] * x[k];

		x[i] = x[i] / A[i][i];
	}
}

/* INPUT: A,P filled in LUPDecompose; N - dimension
 * OUTPUT: IA is the inverse of the initial matrix
 */
void LUPInvert(double **A, int *P, int N, double **IA)
{

	for (int j = 0; j < N; j++) {
		for (int i = 0; i < N; i++) {
			if (P[i] == j)
				IA[i][j] = 1.0;
			else
				IA[i][j] = 0.0;

			for (int k = 0; k < i; k++)
				IA[i][j] -= A[i][k] * IA[k][j];
		}

		for (int i = N - 1; i >= 0; i--) {
			for (int k = i + 1; k < N; k++)
				IA[i][j] -= A[i][k] * IA[k][j];

			IA[i][j] = IA[i][j] / A[i][i];
		}
	}
}

/* INPUT: A,P filled in LUPDecompose; N - dimension.
 * OUTPUT: Function returns the determinant of the initial matrix
 */
double LUPDeterminant(double **A, int *P, int N)
{

	double det = A[0][0];

	for (int i = 1; i < N; i++)
		det *= A[i][i];

	if ((P[N] - N) % 2 == 0)
		return det;
	else
		return -det;
}

double* CDCCalculate(Matrix T, double b_0)
{
	//Cuppen's Divide and conquer algorithm -  https://en.wikipedia.org/wiki/Divide-and-conquer_eigenvalue_algorithm
	//And http://people.inf.ethz.ch/arbenz/ewp/Lnotes/chapters5-6.pdf
	//And "New fast divide-and-conquer algorithms for the symmetric tridiagonal eigenvalue problem" by Shengguo Li, Xiangke Liao, Jie Liu, Hao Jiang
	//Access at https://arxiv.org/abs/1510.04591

	const int N = T.dimension();

	//Create a beta matrix
	Matrix b(N);
	int cval = static_cast<int>(std::ceil(N / 2));
	int fval = static_cast<int>(std::floor(N / 2));

	b[cval][cval] = -b_0;
	b[cval][fval] = -b_0;
	b[fval][cval] = -b_0;
	b[fval][fval] = -b_0;

	Matrix Tcap = T - b;

	return nullptr;
}

/* INPUT: T - Tridiagonal matrix; N - dimension.
 * OUTPUT: evec - eigenvectors return; eval - eigenvalues return (Should be nullptr).
 * http://www.mymathlib.com/c_source/matrices/eigen/jacobi_cyclic_method.c
 */
void JEACalculate(double* A, int N, double* eigenvectors, double* eigenvalues)
{
	const int n = N;

	int row, i, j, k, m;
	double *pAk, *pAm, *p_r, *p_e;
	double threshold_norm;
	double threshold;
	double tan_phi, sin_phi, cos_phi, tan2_phi, sin2_phi, cos2_phi;
	double sin_2phi, cos_2phi, cot_2phi;
	double dum1;
	double dum2;
	double dum3;
	double r;
	double max;

	// Take care of trivial cases

	if (n < 1) return;
	if (n == 1) {
		eigenvalues[0] = *A;
		*eigenvectors = 1.0;
		return;
	}

	// Initialize the eigenvalues to the identity matrix.

	for (p_e = eigenvectors, i = 0; i < n; i++)
		for (j = 0; j < n; p_e++, j++)
			if (i == j) *p_e = 1.0; else *p_e = 0.0;

	// Calculate the threshold and threshold_norm.

	for (threshold = 0.0, pAk = A, i = 0; i < (n - 1); pAk += n, i++)
		for (j = i + 1; j < n; j++) threshold += *(pAk + j) * *(pAk + j);
	threshold = sqrt(threshold + threshold);
	threshold_norm = threshold * DBL_EPSILON;
	max = threshold + 1.0;
	while (threshold > threshold_norm) {
		threshold /= 10.0;
		if (max < threshold) continue;
		max = 0.0;
		for (pAk = A, k = 0; k < (n - 1); pAk += n, k++) {
			for (pAm = pAk + n, m = k + 1; m < n; pAm += n, m++) {
				if (fabs(*(pAk + m)) < threshold) continue;

				// Calculate the sin and cos of the rotation angle which
				// annihilates A[k][m].

				cot_2phi = 0.5 * (*(pAk + k) - *(pAm + m)) / *(pAk + m);
				dum1 = sqrt(cot_2phi * cot_2phi + 1.0);
				if (cot_2phi < 0.0) dum1 = -dum1;
				tan_phi = -cot_2phi + dum1;
				tan2_phi = tan_phi * tan_phi;
				sin2_phi = tan2_phi / (1.0 + tan2_phi);
				cos2_phi = 1.0 - sin2_phi;
				sin_phi = sqrt(sin2_phi);
				if (tan_phi < 0.0) sin_phi = -sin_phi;
				cos_phi = sqrt(cos2_phi);
				sin_2phi = 2.0 * sin_phi * cos_phi;
				cos_2phi = cos2_phi - sin2_phi;

				// Rotate columns k and m for both the matrix A 
				//     and the matrix of eigenvectors.

				p_r = A;
				dum1 = *(pAk + k);
				dum2 = *(pAm + m);
				dum3 = *(pAk + m);
				*(pAk + k) = dum1 * cos2_phi + dum2 * sin2_phi + dum3 * sin_2phi;
				*(pAm + m) = dum1 * sin2_phi + dum2 * cos2_phi - dum3 * sin_2phi;
				*(pAk + m) = 0.0;
				*(pAm + k) = 0.0;
				for (i = 0; i < n; p_r += n, i++) {
					if ((i == k) || (i == m)) continue;
					if (i < k) dum1 = *(p_r + k); else dum1 = *(pAk + i);
					if (i < m) dum2 = *(p_r + m); else dum2 = *(pAm + i);
					dum3 = dum1 * cos_phi + dum2 * sin_phi;
					if (i < k) *(p_r + k) = dum3; else *(pAk + i) = dum3;
					dum3 = -dum1 * sin_phi + dum2 * cos_phi;
					if (i < m) *(p_r + m) = dum3; else *(pAm + i) = dum3;
				}
				for (p_e = eigenvectors, i = 0; i < n; p_e += n, i++) {
					dum1 = *(p_e + k);
					dum2 = *(p_e + m);
					*(p_e + k) = dum1 * cos_phi + dum2 * sin_phi;
					*(p_e + m) = -dum1 * sin_phi + dum2 * cos_phi;
				}
			}
			for (i = 0; i < n; i++)
				if (i == k) continue;
				else if (max < fabs(*(pAk + i))) max = fabs(*(pAk + i));
		}
	}
	for (pAk = A, k = 0; k < n; pAk += n, k++) eigenvalues[k] = *(pAk + k);
}