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

double atan2(std::array<double, 2> inputs)
{
	return std::atan2(inputs[0], inputs[1]);
}

double hypot(std::array<double, 2> inputs)
{
	return std::hypot(inputs[0], inputs[1]);
}

double min(std::array<double, 2> inputs)
{
	return std::min(inputs[0], inputs[1]);
}

double max(std::array<double, 2> inputs)
{
	return std::max(inputs[0], inputs[1]);
}

void rand1d(variable &result, std::array<const Node*, 2> input)
{
	auto input0Shape = input[0]->getShape();
	auto input1Shape = input[1]->getShape();
	result.reshape({ 0,0 });
	if (variableSize(input0Shape) == 0)
		return;
	if (variableSize(input1Shape) == 0)
		return;
	size_t size = size_t(input[0]->getValue({ 0,0 }));
	result.reshape({ size, 1 });
	srand(unsigned int(input[1]->getValue({ 0,0 })));
	for (auto& r : result)
		r = rand() / (double)(RAND_MAX);
}

void rand2d(variable &result, std::array<const Node*, 3> input)
{
	auto input0Shape = input[0]->getShape();
	auto input1Shape = input[1]->getShape();
	auto input2Shape = input[2]->getShape();
	result.reshape({ 0,0 });
	if (variableSize(input0Shape) == 0)
		return;
	if (variableSize(input0Shape) == 0)
		return;
	if (variableSize(input0Shape) == 0)
		return;
	size_t size0 = size_t(input[0]->getValue({ 0,0 }));
	size_t size1 = size_t(input[1]->getValue({ 0,0 }));
	result.reshape({ size0, size1 });
	srand(unsigned int(input[2]->getValue({ 0,0 })));
	for (auto& r : result)
		r = rand() / (double)(RAND_MAX);
}

void sum(variable &result, std::array<const Node*, 1> input)
{
	result.reshape({ 1,1 });
	result[0][0] = 0.0;
	index shape = input[0]->getShape();
	for (size_t i = 0; i < shape[0]; ++i)
		for (size_t j = 0; j < shape[1]; ++j)
			result += input[0]->getValue({ i,j });
}

int main()
{
	sci::string badExpression;
	sci::string output;
	bool isAssignment;

	VariableStore variables;
	FunctionMultiFactory functionFactories;
	parseLine(sU("pi=3.14159265359"), true, badExpression, variables, functionFactories, output, isAssignment);
	parseLine(sU("e=2.71828182845"), true, badExpression, variables, functionFactories, output, isAssignment);
	functionFactories.set(sU("cos"), std::make_unique<EasyElementwiseFunction1Param<std::cos>::Factory>());
	functionFactories.set(sU("sin"), std::make_unique<EasyElementwiseFunction1Param<std::sin>::Factory>());
	functionFactories.set(sU("tan"), std::make_unique<EasyElementwiseFunction1Param<std::tan>::Factory>());
	functionFactories.set(sU("acos"), std::make_unique<EasyElementwiseFunction1Param<std::acos>::Factory>());
	functionFactories.set(sU("asin"), std::make_unique<EasyElementwiseFunction1Param<std::asin>::Factory>());
	functionFactories.set(sU("atan"), std::make_unique<EasyElementwiseFunction1Param<std::atan>::Factory>());
	functionFactories.set(sU("atan2"), std::make_unique<EasyElementwiseFunction<2, atan2>::Factory>());
	functionFactories.set(sU("cosh"), std::make_unique<EasyElementwiseFunction1Param<std::cosh>::Factory>());
	functionFactories.set(sU("sinh"), std::make_unique<EasyElementwiseFunction1Param<std::sinh>::Factory>());
	functionFactories.set(sU("tanh"), std::make_unique<EasyElementwiseFunction1Param<std::tanh>::Factory>());
	functionFactories.set(sU("acosh"), std::make_unique<EasyElementwiseFunction1Param<std::acosh>::Factory>());
	functionFactories.set(sU("asinh"), std::make_unique<EasyElementwiseFunction1Param<std::asinh>::Factory>());
	functionFactories.set(sU("atanh"), std::make_unique<EasyElementwiseFunction1Param<std::atanh>::Factory>());
	functionFactories.set(sU("exp"), std::make_unique<EasyElementwiseFunction1Param<std::exp>::Factory>());
	functionFactories.set(sU("log"), std::make_unique<EasyElementwiseFunction1Param<std::log>::Factory>());
	functionFactories.set(sU("log10"), std::make_unique<EasyElementwiseFunction1Param<std::log10>::Factory>());
	functionFactories.set(sU("ln"), std::make_unique<EasyElementwiseFunction1Param<std::log>::Factory>());
	functionFactories.set(sU("sqrt"), std::make_unique<EasyElementwiseFunction1Param<std::sqrt>::Factory>());
	functionFactories.set(sU("cbrt"), std::make_unique<EasyElementwiseFunction1Param<std::cbrt>::Factory>());
	functionFactories.set(sU("hypot"), std::make_unique<EasyElementwiseFunction<2, hypot>::Factory>());
	functionFactories.set(sU("erf"), std::make_unique<EasyElementwiseFunction1Param<std::erf>::Factory>());
	functionFactories.set(sU("erfc"), std::make_unique<EasyElementwiseFunction1Param<std::erfc>::Factory>());
	functionFactories.set(sU("tgamma"), std::make_unique<EasyElementwiseFunction1Param<std::tgamma>::Factory>());
	functionFactories.set(sU("lgamma"), std::make_unique<EasyElementwiseFunction1Param<std::lgamma>::Factory>());
	functionFactories.set(sU("ceil"), std::make_unique<EasyElementwiseFunction1Param<std::ceil>::Factory>());
	functionFactories.set(sU("floor"), std::make_unique<EasyElementwiseFunction1Param<std::floor>::Factory>());
	functionFactories.set(sU("round"), std::make_unique<EasyElementwiseFunction1Param<std::round>::Factory>());
	functionFactories.set(sU("abs"), std::make_unique<EasyElementwiseFunction1Param<std::abs>::Factory>());
	functionFactories.set(sU("min"), std::make_unique<EasyElementwiseFunction<2, min>::Factory>());
	functionFactories.set(sU("max"), std::make_unique<EasyElementwiseFunction<2, max>::Factory>());
	functionFactories.set(sU("rand1d"), std::make_unique<CachingFunction<2, rand1d>::Factory>());
	functionFactories.set(sU("rand2d"), std::make_unique<CachingFunction<3, rand2d>::Factory>());
	functionFactories.set(sU("sum"), std::make_unique<CachingFunction<1, sum>::Factory>());
	functionFactories.set(sU("freeze"), std::make_unique<FreezeFunction::Factory>());
	std::vector<sci::string> expressions
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
	}

	beginConsoleLoop(variables, functionFactories);

}
