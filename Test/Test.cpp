// Test.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <iostream>
#include"../MathParser/Parse.h"
#include<ctime>

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
		std::cout << "The expression " << sci::toCodepage(expression) << " did not parse. The reported bad expression was " << sci::toCodepage(badExpression) << ", the error was " << sci::toCodepage(output) << std::endl;
		return false;
	}
	if (output != expectedResult)
	{
		std:: cout << "The expression " << sci::toCodepage(expression) << " gave the output " << sci::toCodepage(output) << " but the expected output was " << sci::toCodepage(expectedResult) << std::endl;
		return false;
	}
	return true;
}

template<int N>
bool testFails(sci::string expression, sci::string expectedBadExpression, sci::string expectedError, VariableStore<N>& variables, FunctionMultiFactory<VariableStore<N>>& functionFactories)
{
	sci::string badExpression;
	sci::string output;
	bool isAssignment;
	bool parsed;

	parsed = parseLine(expression, false, badExpression, variables, functionFactories, output, isAssignment);
	if (parsed)
	{
		std::cout << "The expression " << sci::toCodepage(expression) << " parsed. When it should not have." << std::endl;
		return false;
	}
	if (badExpression != expectedBadExpression)
	{
		std::cout << "The expression " << sci::toCodepage(expression) << " was correctly rejected, but the bad expression was " << sci::toCodepage(badExpression) << " when it was expected to have been " << sci::toCodepage(expectedBadExpression) << std::endl;
		return false;
	}
	if (output != expectedError)
	{
		std::cout << "The expression " << sci::toCodepage(expression) << " was correctly rejected, but the error was " << sci::toCodepage(output) << " when it was expected to have been " << sci::toCodepage(expectedError) << std::endl;
		return false;
	}
	return true;
}

template<int N>
void timeTest(sci::string expression, VariableStore<N>& variables, FunctionMultiFactory<VariableStore<N>>& functionFactories)
{
	sci::string badExpression;
	sci::string output;
	bool isAssignment;
	bool parsed;

	auto start = clock();
	parsed = parseLine(expression, false, badExpression, variables, functionFactories, output, isAssignment);
	auto end = clock();
	double seconds = double(end - start) / double(CLOCKS_PER_SEC);
	std::cout << "The expression " << sci::toCodepage(expression) << " was evaluated in " << seconds << " seconds" << std::endl;
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
	success &= test(sU("3+-1"), sU("2"), variables, functionFactories);
	success &= test(sU("3-+1"), sU("2"), variables, functionFactories);
	success &= test(sU("3--1"), sU("4"), variables, functionFactories);
	success &= test(sU("3--1"), sU("4"), variables, functionFactories);
	success &= test(sU("3+--1"), sU("4"), variables, functionFactories);
	success &= test(sU("3-+-1"), sU("4"), variables, functionFactories);
	success &= test(sU("x="), sU(""), variables, functionFactories);
	success &= test(sU("x=sin(1)"), sU(""), variables, functionFactories);
	success &= test(sU("x=atan2(1,2)"), sU(""), variables, functionFactories);

	//Check direct infinite loops get caught
	success &= testFails(sU("x=x"), sU("x"), sU("infinite_recursion_error"), variables, functionFactories);
	success &= testFails(sU("x=1+x"), sU("x"), sU("infinite_recursion_error"), variables, functionFactories);
	success &= testFails(sU("x=x+1"), sU("x"), sU("infinite_recursion_error"), variables, functionFactories);
	
	//Check indirect infinite loops get caught
	success &= test(sU("x=2"), sU(""), variables, functionFactories);
	success &= test(sU("y=x+1"), sU(""), variables, functionFactories);
	success &= testFails(sU("x=y+1"), sU("y"), sU("infinite_recursion_error"), variables, functionFactories);
	
	//Check indirect infinite loops get caught with the variable second
	success &= test(sU("x=2"), sU(""), variables, functionFactories);
	success &= test(sU("y=x+1"), sU(""), variables, functionFactories);
	success &= testFails(sU("x=1+y"), sU("y"), sU("infinite_recursion_error"), variables, functionFactories);

	//check infinite loops via functions
	success &= test(sU("x=1"), sU(""), variables, functionFactories);
	success &= testFails(sU("x=sin(x)"), sU("sin(x)"), sU("infinite_recursion_error"), variables, functionFactories);

	//check an expression ending in an operator gets caught
	success &= testFails(sU("x=2+"), sU("2+"), sU("second_operand_missing_error"), variables, functionFactories);
	
	//check what happens when a used variable gets set to bad expression
	success &= test(sU("x=2"), sU(""), variables, functionFactories);
	success &= test(sU("y=x+1"), sU(""), variables, functionFactories);
	success &= testFails(sU("x=3+"), sU("3+"), sU("second_operand_missing_error"), variables, functionFactories); //x should now have been removed
	success &= testFails(sU("y"), sU("y"), sU("empty_variable_error"), variables, functionFactories);

	//check other errors are generated correctly
	success &= testFails(sU("sin(1"), sU("sin(1"), sU("missing_function_close_bracket_error"), variables, functionFactories);
	success &= testFails(sU("notafunction()"), sU("notafunction()"), sU("function_not_found_error"), variables, functionFactories);
	success &= testFails(sU("sin(1,1)"), sU("sin(1,1)"), sU("incorrect_number_of_function_arguments_error"), variables, functionFactories);
	success &= testFails(sU("{1.2,1.2"), sU("{1.2,1.2"), sU("missing_array_close_bracket_error"), variables, functionFactories);
	success &= testFails(sU("1.2,1.2}"), sU("1.2,1.2}"), sU("missing_array_open_bracket_error"), variables, functionFactories);
	success &= testFails(sU("{1.2,1.2a}"), sU("1.2a"), sU("invalid_or_missing_array_element_error"), variables, functionFactories);
	success &= testFails(sU("notavarialble"), sU("notavarialble"), sU("variable_not_found_error"), variables, functionFactories);
	success &= testFails(sU("sin(x=27)"), sU("sin(x"), sU("invalid_characters_before_equals_error"), variables, functionFactories);
	success &= testFails(sU("x=y=2"), sU("x=y=2"), sU("more_than_one_equals_error"), variables, functionFactories);
	success &= testFails(sU("x=1a"), sU("1a"), sU("variable_not_found_error"), variables, functionFactories);
	success &= testFails(sU("x=5*(2+1"), sU("(2+1"), sU("missing_close_bracket_error"), variables, functionFactories);
	success &= testFails(sU("x=5(2+1)"), sU("5(2+1)"), sU("missing_operator_error"), variables, functionFactories);
	success &= testFails(sU("x=1)"), sU("1)"), sU("missing_open_bracket_error"), variables, functionFactories);
	success &= testFails(sU("x=2+1)*5"), sU("2+1)"), sU("missing_open_bracket_error"), variables, functionFactories);
	success &= testFails(sU("x={(2}+1)"), sU("{(2}+1)"), sU("interleaved_brackets_error"), variables, functionFactories);
	success &= testFails(sU("x=({2)+1}"), sU("({2)+1}"), sU("interleaved_brackets_error"), variables, functionFactories);
	success &= testFails(sU("=5"), sU(""), sU("invalid_characters_before_equals_error"), variables, functionFactories);
	success &= testFails(sU("7=7"), sU("7"), sU("invalid_characters_before_equals_error"), variables, functionFactories);
	success &= testFails(sU("my variable=5"), sU("my variable"), sU("invalid_characters_before_equals_error"), variables, functionFactories);
	success &= test(sU(""), sU(""), variables, functionFactories);
	success &= test(sU(" "), sU(""), variables, functionFactories);
	success &= test(sU("\t"), sU(""), variables, functionFactories);
	//check whitespace removal
	success &= testFails(sU(" my variable=5"), sU("my variable"), sU("invalid_characters_before_equals_error"), variables, functionFactories);
	success &= testFails(sU("my  variable=5"), sU("my variable"), sU("invalid_characters_before_equals_error"), variables, functionFactories);
	success &= testFails(sU("my variable =5"), sU("my variable"), sU("invalid_characters_before_equals_error"), variables, functionFactories);
	success &= testFails(sU("my+ variable=5"), sU("my+variable"), sU("invalid_characters_before_equals_error"), variables, functionFactories);
	success &= testFails(sU("my +variable=5"), sU("my+variable"), sU("invalid_characters_before_equals_error"), variables, functionFactories);
	success &= testFails(sU("x=2 2"), sU("2 2"), sU("missing_operator_error"), variables, functionFactories);

	//check function calls with comma containing arguments
	success &= test(sU("x=sin({1,1})"), sU(""), variables, functionFactories);
	success &= test(sU("x=sin(atan2(1,2))"), sU(""), variables, functionFactories);
	//and some more nested function tests
	success &= test(sU("atan2(sin(0),2)"), sU("0"), variables, functionFactories);
	success &= test(sU("hypot(atan2(0,2),3)"), sU("3"), variables, functionFactories);
	success &= testFails(sU("sin()"), sU("sin()"), sU("incorrect_number_of_function_arguments_error"), variables, functionFactories);
	success &= testFails(sU("atan2(1,)"), sU("atan2(1,)"), sU("incorrect_number_of_function_arguments_error"), variables, functionFactories);

	//array specific tests
	success &= testFails(sU("{}"), sU("{}"), sU("invalid_or_missing_array_element_error"), variables, functionFactories);
	success &= testFails(sU("{{}{}}"), sU("{}"), sU("invalid_or_missing_array_element_error"), variables, functionFactories);
	success &= testFails(sU("{1,}"), sU(",}"), sU("invalid_or_missing_array_element_error"), variables, functionFactories);
	success &= testFails(sU("{1,,3}"), sU(",,"), sU("invalid_or_missing_array_element_error"), variables, functionFactories);
	success &= testFails(sU("{,2}"), sU("{,"), sU("invalid_or_missing_array_element_error"), variables, functionFactories);
	success &= testFails(sU("{1,2}{3}"), sU("{3}"), sU("array_2nd_dimension_sizes_do_not_match_error"), variables, functionFactories);
	success &= testFails(sU("{1}{2,3}"), sU("{2,3}"), sU("array_2nd_dimension_sizes_do_not_match_error"), variables, functionFactories);
	success &= test(sU("x={1,2}{3,4}"), sU(""), variables, functionFactories);
	success &= testFails(sU("x={{1,2},{3,4}}"), sU(","), sU("unexpected_character_between_array_brackets_error"), variables, functionFactories);
	success &= testFails(sU("x={{1,2}{3,4}"), sU("{{1,2}{3,4}"), sU("missing_array_close_bracket_error"), variables, functionFactories);
	success &= testFails(sU("x={1,2},{3,4}"), sU(",{3,4}"), sU("unexpected_characters_after_array_error"), variables, functionFactories);
	success &= testFails(sU("{1,2}noOperator"), sU("noOperator"), sU("unexpected_characters_after_array_error"), variables, functionFactories);

	//freeze tests
	success &= test(sU("freezeX=2"), sU(""), variables, functionFactories);
	success &= test(sU("freezeY=freeze(freezeX)"), sU(""), variables, functionFactories);
	success &= test(sU("freezeX=3"), sU(""), variables, functionFactories);
	success &= test(sU("freezeY"), sU("2"), variables, functionFactories);
	success &= testFails(sU("freeze({1,2, 3} + {4, 5})"), sU("{1,2,3}+{4,5}"), sU("incompatible_sizes_error"), variables, functionFactories);

	//array broadcast tests
	success &= test(sU("x={1,2,3}"), sU(""), variables, functionFactories);
	success &= test(sU("y={{0}{3}{6}}"), sU(""), variables, functionFactories);
	success &= test(sU("z={{1,2,3}{4,5,6}{7,8,9}}"), sU(""), variables, functionFactories);
	success &= test(sU("x+y"), sU("{ { 1, 2, 3 } { 4, 5, 6 } { 7, 8, 9 } }"), variables, functionFactories);
	success &= test(sU("y+x"), sU("{ { 1, 2, 3 } { 4, 5, 6 } { 7, 8, 9 } }"), variables, functionFactories);
	success &= test(sU("x+z"), sU("{ { 2, 4, 6 } { 5, 7, 9 } { 8, 10, 12 } }"), variables, functionFactories);
	success &= test(sU("z+x"), sU("{ { 2, 4, 6 } { 5, 7, 9 } { 8, 10, 12 } }"), variables, functionFactories);
	success &= test(sU("y+z"), sU("{ { 1, 2, 3 } { 7, 8, 9 } { 13, 14, 15 } }"), variables, functionFactories);
	success &= test(sU("z+y"), sU("{ { 1, 2, 3 } { 7, 8, 9 } { 13, 14, 15 } }"), variables, functionFactories);
	success &= testFails(sU("{{1,2,3}{4,5,6}}+z"), sU("{{1,2,3}{4,5,6}}+z"), sU("incompatible_sizes_error"), variables, functionFactories);

	//Live dependency invalidation
	success &= test(sU("liveX=2"), sU(""), variables, functionFactories);
	success &= test(sU("liveY=sin(liveX)"), sU(""), variables, functionFactories);
	success &= testFails(sU("liveX=3+"), sU("3+"), sU("second_operand_missing_error"), variables, functionFactories);
	success &= testFails(sU("liveY"), sU("liveY"), sU("empty_variable_error"), variables, functionFactories);

	//Dynamic array shape changes
	success &= test(sU("shapeX={1,2}"), sU(""), variables, functionFactories);
	success &= test(sU("shapeY=shapeX+1"), sU(""), variables, functionFactories);
	success &= test(sU("shapeX={1,2,3}"), sU(""), variables, functionFactories);
	success &= test(sU("shapeY"), sU("{ 2, 3, 4 }"), variables, functionFactories);

	//Dynamic shape incompatibility
	success &= test(sU("shapeA={1,2}"), sU(""), variables, functionFactories);
	success &= test(sU("shapeB=shapeA+{3,4}"), sU(""), variables, functionFactories);
	success &= test(sU("shapeA={1,2,3}"), sU(""), variables, functionFactories);
	success &= testFails(sU("shapeB"), sU("shapeB"), sU("incompatible_sizes_error"), variables, functionFactories);

	//Scientific notation and negative exponents
	success &= test(sU("1e-3+2"), sU("2.001"), variables, functionFactories);
	success &= test(sU("2^-3"), sU("0.125"), variables, functionFactories);

	success &= testFails(sU("sum({1,2,3}+{4,5})"),
		sU("sum({1,2,3}+{4,5})"),
		sU("incompatible_sizes_error"), variables, functionFactories);

	//random 
	time_t seed1Val = time(nullptr);
	sci::ostringstream seed1;
	seed1 << seed1Val;
	time_t seed2Val = time(nullptr);
	sci::ostringstream seed2;
	seed2 << seed2Val;

	success &= test(sU("r1=rand2d(3000,3000,") + seed1.str() + sU(")"), sU(""), variables, functionFactories);
	success &= test(sU("r2=rand2d(3000,3000,") + seed2.str() + sU(")"), sU(""), variables, functionFactories);
	//timeTest(sU("r1+r2"), variables, functionFactories);

	if (success)
		std::cout << "All tests passed" << std::endl;
	else
	{
		std::cout << "Some tests failed" << std::endl;
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
