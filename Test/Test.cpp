// Test.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <iostream>
#include"../MathParser/Parse.h"

template<int N>
bool test(sci::string expression, sci::string expectedResult, VariableStore<N> &variables, FunctionMultiFactory<VariableStore<N>> &functionFactories)
{
	sci::string badExpression;
	sci::string output;
	bool isAssignment;
	bool parsed;

	parsed = parseLine(expression, false, badExpression, variables, functionFactories, output, isAssignment);
	if (!parsed)
	{
		std::cout << "The expression " << sci::toCodepage(expression) << " did not parse. The reported bad expression was " << sci::toCodepage(badExpression) << std::endl;
		return false;
	}
	if (output != expectedResult)
	{
		std:: cout << "The expression " << sci::toCodepage(expression) << " gave the output " << sci::toCodepage(output) << " but the expected output was " << sci::toCodepage(expectedResult) << std::endl;
		return false;
	}
	return true;
}

int main()
{
	const int nDims = 2;

	VariableStore<nDims> variables;
	FunctionMultiFactory<VariableStore<nDims>> functionFactories;
	functionFactories.addStandardFunctions();

	bool success = true;
	success &= test(sU("8-4-2"), sU("2"), variables, functionFactories);
	success &= test(sU("8+4-2"), sU("10"), variables, functionFactories);
	success &= test(sU("8/2/2"), sU("2"), variables, functionFactories);
	success &= test(sU("8*2/4"), sU("4"), variables, functionFactories);
	success &= test(sU("8-4+2"), sU("6"), variables, functionFactories);
	success &= test(sU("3*-3"), sU("-9"), variables, functionFactories);
	success &= test(sU("8*(2/2)"), sU("8"), variables, functionFactories);
	success &= test(sU("8*-(2/2)"), sU("-8"), variables, functionFactories);
	success &= test(sU("8*2+1"), sU("17"), variables, functionFactories);
	success &= test(sU("8*(2+1)"), sU("24"), variables, functionFactories);
	success &= test(sU("2^2^0"), sU("2"), variables, functionFactories);
	success &= test(sU("(2^2)^0"), sU("1"), variables, functionFactories);
	success &= test(sU("(2*3)^2"), sU("36"), variables, functionFactories);
	success &= test(sU("2*3^2"), sU("18"), variables, functionFactories);
	success &= test(sU("(2+3)^2"), sU("25"), variables, functionFactories);
	success &= test(sU("2+3^2"), sU("11"), variables, functionFactories);
	success &= test(sU("-3^2"), sU("-9"), variables, functionFactories);
	success &= test(sU("0-3^2"), sU("-9"), variables, functionFactories);
	success &= test(sU("(-3)^2"), sU("9"), variables, functionFactories);

	if (success)
		std::cout << "All tests passed" << std::endl;
	else
	{
		std::cout << "soume tests failed" << std::endl;
		return 1;
	}
}

// Run program: Ctrl + F5 or Debug > Start Without Debugging menu
// Debug program: F5 or Debug > Start Debugging menu

// Tips for Getting Started: 
//   1. Use the Solution Explorer window to add/manage files
//   2. Use the Team Explorer window to connect to source control
//   3. Use the Output window to see build output and other messages
//   4. Use the Error List window to view errors
//   5. Go to Project > Add New Item to create new code files, or Project > Add Existing Item to add existing code files to the project
//   6. In the future, to open this project again, go to File > Open > Project and select the .sln file
