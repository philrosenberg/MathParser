#include"Parse.h"
#include<iostream>

int main()
{
	const int nDims = 2;

	sci::string badExpression;
	sci::string output;
	bool isAssignment;

	VariableStore<nDims> variables;
	FunctionMultiFactory<VariableStore<nDims>> functionFactories;
	functionFactories.addStandardFunctions();
	parseLine(sU("pi=3.14159265359"), true, badExpression, variables, functionFactories, output, isAssignment);
	parseLine(sU("e=2.71828182845"), true, badExpression, variables, functionFactories, output, isAssignment);

	//beginStreamLoop(std::cin, std::cout, variables, functionFactories);
	beginStreamLoop(std::wcin, std::wcout, variables, functionFactories);

}
