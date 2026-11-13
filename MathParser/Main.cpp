#include"Function.h"
#include"VariableStore.h"
#include"Node.h"
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
	/*std::vector<sci::string> expressions
	{
		sU("4/2/2"),
		sU("4^3^2"),
		sU("27/2+1"),
		sU("(27/2+1)"),
		sU("27/(2+1)"),
		sU("27/2+-1"),
		sU("-27/2+1"),
		sU("27/-2+1"),
		sU("2*pi")
	};
	for (size_t i = 0; i < expressions.size(); ++i)
	{
		std::cout << "Parsing " << basicAscii(expressions[i]) << std::endl;
		bool isAssignment;
		parseLine(expressions[i], false, badExpression, variables, functionFactories, output, isAssignment);
		if (badExpression.length() > 0)
			std::cout << "Result: error " << basicAscii(badExpression) << std::endl;
		else
			std::cout << "Result: " << basicAscii(output) << std::endl;
	}*/

	beginConsoleLoop(variables, functionFactories);

}
