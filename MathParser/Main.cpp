#include"Function.h"
#include"VariableStore.h"
#include"Node.h"
#include"Parse.h"
#include<iostream>

template<typename... Ts>
void test(Ts... ts)
{
	std::cos(std::forward<double>(ts)...);
}

void test2()
{
	test(4.0);
}


int main()
{
	sci::string badExpression;
	sci::string output;
	bool isAssignment;

	VariableStore variables;
	FunctionMultiFactory functionFactories;
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
