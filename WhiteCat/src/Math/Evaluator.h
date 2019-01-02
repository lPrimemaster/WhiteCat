#pragma once
#include <iostream>
#include <string>
#include <stack>
#include <algorithm>
#include <regex>

typedef unsigned int uint;

template<typename I, typename O>
class Evaluator
{
public:
	Evaluator() = default;
	~Evaluator() = default;

	Evaluator operator=(std::string expression) noexcept
	{
		this->expression = expression;

		//Replace verbal operators for characters
		std::regex patternS("Sin");
		this->expression = std::regex_replace(this->expression, patternS, "S");

		std::regex patternE("Exp");
		this->expression = std::regex_replace(this->expression, patternE, "E");

		std::regex patternL("Log");
		this->expression = std::regex_replace(this->expression, patternL, "L");

		return *this;
	}

	O operator()(I x)
	{
		return calcExpr(x);
	}
	O operator()(I x, I y)
	{
		return calcExpr(x, y);
	}
	O operator()(I x, I y, I z)
	{
		return calcExpr(x, y, z);
	}


private:

	std::string expression;

	O calcExpr(I var)
	{
		xVal = var;
		return evaluateExpr(expression);
	}

	O calcExpr(I var1, I var2)
	{
		xVal = var1;
		yVal = var2;
		return evaluateExpr(expression);
	}

	O calcExpr(I var1, I var2, I var3)
	{
		xVal = var1;
		yVal = var2;
		zVal = var3;
		return evaluateExpr(expression);
	}

	O evaluateExpr(std::string expr);
	void processCP(std::stack<I>& vStack, std::stack<char>& cStack);
	I processIV(std::string expr, uint pos, std::stack<I>& vStack);
	void processIO(char op, std::stack<I>& vStack, std::stack<char>& cStack);
	bool opCausesEV(char op, char prevOp);
	void executeOP(std::stack<I>& vStack, std::stack<char>& cStack);

	I xVal = I(0);
	I yVal = I(0);
	I zVal = I(0);

};

	/*Point* sODE_MidPoint(uint step);
	Point* sPDE(uint step);*/

template<typename I, typename O>
O Evaluator<I, O>::evaluateExpr(std::string expr)
{
	std::stack<I> valueStack;
	std::stack<char> operatorStack;

	//Push a left bracket so the evaluation always finishes
	operatorStack.push('(');

	uint pos = 0;
	while (pos <= expr.size())
	{
		if (expr[pos] == ' ') //Get rid of blank spaces
		{
			pos++;
		}
		else if (pos == expr.size() || expr[pos] == ')') //Check if end or bracket close
		{
			processCP(valueStack, operatorStack);
			pos++;
		}
		else if (expr[pos] >= '0' && expr[pos] <= '9' || expr[pos] == '.'
			|| expr[pos] == 'x' || expr[pos] == 'y' || expr[pos] == 'z') //Check if reading a number
		{
			pos = processIV(expr, pos, valueStack);
		}
		else //Else we have an operator present
		{
			processIO(expr[pos], valueStack, operatorStack);
			pos++;
		}
	}

	//There should only be one element back on the stack -- make checks here later
	I val = valueStack.top();
	valueStack.pop();
	return val;

}

template<typename I, typename O>
void Evaluator<I, O>::processCP(std::stack<I>& vStack, std::stack<char>& cStack)
{
	while (cStack.top() != '(')
	{
		executeOP(vStack, cStack);
	}

	cStack.pop(); //Remove opening bracket
}

template<typename I, typename O>
I Evaluator<I, O>::processIV(std::string expr, uint pos, std::stack<I>& vStack)
{
	I value = I(0); //Complex numbers wont work here for now ...
	bool decimal = false;
	uint count = 0;
	while (pos < expr.size() && expr[pos] >= '0' && expr[pos] <= '9' || expr[pos] == '.'
		|| expr[pos] == 'x' || expr[pos] == 'y' || expr[pos] == 'z')
	{
		if (expr[pos] == 'x')
		{
			value = xVal;
			pos++;
			break;
		}
		else if (expr[pos] == 'y')
		{
			value = yVal;
			pos++;
			break;
		}
		else if (expr[pos] == 'z')
		{
			value = zVal;
			pos++;
			break;
		}
		else if (expr[pos] == '.')
		{
			decimal = true;
			pos++;
		}

		if (!decimal)
		{
			value = 10 * value + (I)(expr[pos++] - '0');
		}
		else
		{
			value = value + pow(0.1, ++count) * (I)(expr[pos++] - '0');
		}
	}

	vStack.push(value);
	return pos;
}

template<typename I, typename O>
void Evaluator<I, O>::processIO(char op, std::stack<I>& vStack, std::stack<char>& cStack)
{
	while (cStack.size() > 0 && opCausesEV(op, cStack.top()))
	{
		executeOP(vStack, cStack);
	}

	cStack.push(op);
}

template<typename I, typename O>
bool Evaluator<I, O>::opCausesEV(char op, char prevOp)
{
	bool evaluate = false;

	switch (op)
	{
	case '+':
	case '-':
		evaluate = (prevOp != '(');
		break;
	case '*':
	case '/':
		evaluate = (prevOp != '(' && prevOp != '+' && prevOp != '-');
		break;
	case ')':
		evaluate = true;
		break;
	case '^':
	case 'S': //Sin
	case 'E': //Exp
	case 'L': //Log (Natural)
		evaluate = false;
	default:
		break;
	}

	return evaluate;
} 

template<typename I, typename O>
void Evaluator<I, O>::executeOP(std::stack<I>& vStack, std::stack<char>& cStack)
{
	I right_operand = vStack.top(); vStack.pop();
	I left_operand = I(0);
	if(!vStack.empty())
		left_operand = vStack.top();
	char op = cStack.top(); cStack.pop();

	I result = I(0);

	switch (op)
	{
	case '+':
		result = left_operand + right_operand; vStack.pop();
		break;
	case '-':
		result = left_operand - right_operand; vStack.pop();
		break;
	case '*':
		result = left_operand * right_operand; vStack.pop();
		break;
	case '/':
		result = left_operand / right_operand; vStack.pop();
		break;
	case '^':
		result = pow(left_operand, right_operand); vStack.pop();
		break;
	case 'S': //Sin
		result = sin(right_operand);
		break;
	case 'E': //Exp
		result = exp(right_operand);
		break;
	case 'L': //Log (Natural)
		result = log(right_operand);
		break;
	}

	vStack.push(result);
}

