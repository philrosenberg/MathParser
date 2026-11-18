#pragma once
#include"Function.h"
#include"VariableStore.h"
#include<iostream>
#include<scieng/codepage.h>

//parse a line
//if the line contains one equals sign, the variable will be put in the variableDestination if the expression is valid and variableDestination is not null, isAssignment will be set true, the return of node.set will be returned if variableDestination is not null or false otherwise
//if the line contains zero equals signs, the node will be set to the expression if it is valid, isAssignment will be set false, the return of node.set will be returned
//if the line contains more than one equals, isAssignment will be set to false, false will be returned.
//There is a source and destination VariableStore. This is to allow expressions to be parsed using a const VariableStore if needed
template<class VariableStore>
bool parseLine(const sci::string& line, sci::string& badLine, sci::string &error, VariableStore* variableDestination, const VariableStore& variableSource, const FunctionMultiFactory<VariableStore>& functions, typename VariableStore::node& node, bool& isAssignment)
{
	isAssignment = false;

	//remove whitespace and check for equals
	sci::string trimmedLine;
	trimmedLine.reserve(line.size());
	size_t equalsIndex = -1;
	size_t equalsCount = 0;
	bool addedSpaceLast = false;
	bool addedNonSpace = false;
	for(size_t i=0; i<line.length(); ++i)
	{
		//if it's not a space, add it to the trimmed line
		if (line[i] != sU(' ') && line[i] != sU('\t'))
		{
			trimmedLine.push_back(line[i]);
			addedNonSpace = true;
			addedSpaceLast = false;
		}
		//if it's a space, but not next to any disallowed variable characters also add it
		//and there has been at least one non-space added, then add it. But not more than
		//one consecutively
		else if (i != line.length() - 1 && i != 0 && !addedSpaceLast && addedNonSpace &&
			VariableStore::getDisallowedCharacters().find(line[i - 1]) == sci::string::npos &&  //to the left is a variable character
			(VariableStore::getDisallowedCharacters().find(line[i + 1]) == sci::string::npos || // (to the right is a variable character
			line[i + 1] == sU(' ') || line[i + 1] == sU('\t')))                                 // or a space)
		{
			trimmedLine.push_back(line[i]);
			addedSpaceLast = true;
		}


		if (line[i] == sU('='))
		{
			equalsIndex = trimmedLine.size() - 1;
			equalsCount++;
		}
	}
	//there could be a single trailing space, remove it if so
	if (trimmedLine.back() == sU(' ') || trimmedLine.back() == sU('\t'))
		trimmedLine.erase(trimmedLine.end() - 1);

	if (equalsCount > 1)
	{
		//If there is more than 1 equals sign - this is a bad line
		error = sU("more_than_one_equals_error");
		badLine = trimmedLine;
		return false;
	}
	if (equalsCount == 0)
	{
		//If there is no equals sign, calculate the result of the expression and print it
		return node.set(trimmedLine, badLine, error, variableSource, functions);
	}
	else
	{
		//If there is one equals sign, set the variable
		isAssignment = true;
		if (!variableDestination)
		{
			error = sU("no_variable_store_provided_error");
			badLine = trimmedLine;
			return false;
		}
		sci::string name = trimmedLine.substr(0, equalsIndex);
		sci::string expr = trimmedLine.substr(equalsIndex + 1);

		typename VariableStore::node* variable = variableDestination->getOrCreateVariable(name);
		if (!variable)
		{
			error = sU("invalid_characters_before_equals_error");
			badLine = name;
			return false;
		}
		bool result = variable->set(expr, badLine, error, variableSource, functions);
		if (!result)
		{
			if (badLine.empty())
				badLine = trimmedLine;
			sci::string tempString1;
			sci::string tempString2;
			variable->set(sU(""), tempString1, tempString2, variableSource, functions);
		}
		return result;
	}
}

template<class VariableStore>
bool parseLine(const sci::string& line, bool skipOutputCalculation, sci::string& badLine, VariableStore& variables, const FunctionMultiFactory<VariableStore>& functions, sci::string& output, bool& isAssignment)
{
	output = sU("");
	typename VariableStore::node node;
	isAssignment = false;
	bool result = parseLine(line, badLine, output, &variables, variables, functions, node, isAssignment);
	if (!result)
		return false;
	if (!isAssignment && !skipOutputCalculation)
	{
		sci::ostringstream str;
		typename VariableStore::node::index shape = node.getShape();
		size_t size = VariableStore::node::getSizeFromShape(shape, output);
		if (size == 0) // zero dimensions. Error
		{
			badLine = line;
			output = sU("empty_variable_error");
			return false;
		}
		else if (!output.empty())
		{
			//catches errors set in getSizefromShape
			badLine = line;
			return false;
		}
		else if (size == 1)  // scalar
			str << node.calculate()[0][0];
		else if (shape[0] == 1) //row
		{
			str << sU("{ ") << node.getValue({ 0,0 });
			for (size_t i = 1; i < shape[1]; ++i)
				str << sU(", ") << node.getValue({ 0, i });
			str << sU(" }");
		}
		else if (shape[1] == 1) //col
		{
			str << sU("{ ") << node.getValue({ 0,0 });
			for (size_t i = 1; i < shape[0]; ++i)
				str << sU(", ") << node.getValue({ i, 0 });
			str << sU(" }");
		}
		else
		{
			str << sU("{ ");
			for (size_t i = 0; i < shape[0]; ++i)
			{
				str << sU("{ ") << node.getValue({ i, 0 });
				for (size_t j = 1; j < shape[1]; ++j)
					str << sU(", ") << node.getValue({ i,j });
				str << sU(" } ");
			}
			str << sU("}");
		}

		output = str.str();
	}
	return result;
}

//this parses an expression, not a full line. It will generate an error if the expression contains an assignment
template<class VariableStore>
bool parseExpression(const sci::string& line, sci::string& badLine, sci::string &error, const VariableStore& variables, const FunctionMultiFactory<VariableStore>& functions, typename VariableStore::node& output)
{
	bool isAssignment = false;
	bool result = parseLine(line, badLine, error, (VariableStore*)nullptr, variables, functions, output, isAssignment);
	if (result && isAssignment)
		error = sU("expression_contained_unexpected_assignment_error");
	return result && !isAssignment;
}

template<class VariableStore>
void beginConsoleLoop(VariableStore& variables, FunctionMultiFactory<VariableStore>& functionFactories)
{
	std::string line;
	sci::string output;
	sci::string badExpression;
	bool isAssignment;
	while (1)
	{
		std::cout << "math> ";
		std::getline(std::cin, line);
		if (line == "exit")
			break;
		bool good = parseLine(sci::toSci(line), false, badExpression, variables, functionFactories, output, isAssignment);
		if (output.length() > 0)
		{
			std::cout << sci::toCodepage(output) << "\n";
		}
		if (!good)
		{
			std::cout << "Bad expression: " << sci::toCodepage(badExpression) << "\n";
		}
	};
}
