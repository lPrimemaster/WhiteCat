#pragma once
#include <iostream>
#include <algorithm>

#include "../utils.h"
//#include "Evaluator.h"

typedef double (*Potential)(double);

class Solver
{
public:
	Solver() = default;
	~Solver() = default;

	/* INPUT: S - Barrier size; N - Number of points (>2); U - funcpointer for a pontential function
	* OUTPUT: 1D Time independent Wave function at points Ni (Default Boundary Conditions)
	*/
	static Vector FDM(double S, uint N, Potential U);

};

