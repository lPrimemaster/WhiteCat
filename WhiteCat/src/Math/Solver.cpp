#include "Solver.h"

Vector Solver::FDM(double S, uint N, Potential U)
{
	const double rightBorder = S;
	const double leftBorder = 0.0;

	const double step = S / (N - 1);

	const double m = 1; //Unit mass of the electron
	const double hbar = 1; //Natural units system
	const double t_0 = hbar * hbar / (2 * m * step * step);

	Matrix U_m(N - 2);
	Matrix F_m(N - 2);

	//Construct the potential matrix
	for (int i = 0; i < N - 2; i++)
	{
		U_m[i][i] = U(step * i);
	}

	//Construct the FDM 2nd derivative approx matrix (Tri-diagonal matrix)
	for (int i = 0; i < N - 2; i++)
	{
		for (int j = 0; j < N - 2; j++)
		{
			if (i == j)
				F_m[i][j] = 2 * t_0;
			else if (i == j + 1 || i + 1 == j)
				F_m[i][j] = -t_0;
			else
				continue;
		}
	}

	//Construct the full Hamiltonian matrix
	Matrix H_m = U_m + F_m;

	//Solve the eigenvalues and eigenvectors - with default boundary equations X[0] == X[N] == 0
	Matrix::calcEigenV(&H_m);

	double* evals = H_m.eigenValues();
	double** evecs = H_m.eigenVectors();

	Vector eigenvectors(N - 2);

	for (int i = 0; i < N - 2; i++)
	{
		*eigenvectors[i] = evecs[i][i];
	}

	return eigenvectors;
}
