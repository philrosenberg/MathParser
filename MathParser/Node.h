#pragma once
#include"types.h"
#include"FunctionBase.h"
#include"Operations.h"
#include"VariableStore.h"
#include<memory>
#include<scieng/codepage.h>

template<class VariableStore>
class FunctionMultiFactory;

template<int nDims>
class Node
{

	//note that all public member functions are const
	//this is by design, so only controlled functions can modify Nodes

public:
	using variable = sci::GridData<double, nDims>;
	using index = std::array<size_t, nDims>;
	friend bool parseLine< VariableStore<nDims>>(const sci::string& line, sci::string& badLine, sci::string &error, VariableStore<nDims>* variableDestination, const VariableStore<nDims>& variableSource, const FunctionMultiFactory<VariableStore<nDims>>& functions, Node& node, bool& isAssignment);
	friend class VariableStore<nDims>;
	Node()
		:m_function(nullptr)
	{
		m_op = &nullOperator<VariableStore<nDims>>;
		for (auto& n : m_nodes)
			n = nullptr;
		for (auto& s : m_shape)
			s = 0;
	}

	constexpr bool contains(const Node* other) const
	{
		for (const auto& n : m_nodes)
		{
			if (n != nullptr && (n == other || n->contains(other)))
				return true;
		}
		if (m_function && m_function->contains(other))
			return true;
		return false;
	}

	variable calculate(sci::string& error) const
	{
		getShape(error); // this updates the shape
		variable result(m_shape); //if there were incompatible shapes, this will be zero size

		for (size_t i = 0; i < m_shape[0]; ++i)
			for (size_t j = 0; j < m_shape[1]; ++j)
				result[i][j] = getValue({ i, j });
		return result;
	}

	constexpr bool isValid() const
	{
		for (auto s : m_shape)
			if (s == 0)
				return false;
		return true;
	}
	static index combineShapes(const index &shape1, const index &shape2, sci::string& error)
	{
		index result;
		
		for (size_t i = 0; i < result.size(); ++i)
		{
			if (shape1[i] == shape2[i])
			{
				result[i] = shape1[i];
			}
			else if (shape1[i] == 0 || shape2[i] == 0)
			{
				result[i] = 0;
				error = sU("empty_variable_error");
			}
			else if (shape1[i] == 1)
				result[i] = shape2[i];
			else if (shape2[i] == 1)
				result[i] = shape1[i];
			else
			{
				result[i] = 0;
				error = sU("incompatible_sizes_error");
			}
		}
		return result;
	}
	index getShape(sci::string& error) const
	{
		if (m_op == runFunction)
			m_shape = m_function->getShapeAndCacheIfNeeded(error);
		else if (m_op == constantRead)
			m_shape = m_constant.shape();
		else if (m_op == nodePassthrough)
			m_shape = m_nodes[0]->getShape(error);
		else
			m_shape = combineShapes(m_nodes[0]->getShape(error), m_nodes[1]->getShape(error), error);

		return m_shape;
	}
	constexpr double getValue(const index& position) const
	{
		return m_op(m_function.get(), &m_constant, m_shape, m_nodes, position);
	}
	static constexpr size_t getSizeFromShape(const index &shape, sci::string& error)
	{
		return multArray<0>(shape, error);
	}
private:
	//set is private to restrict the callers. This is because expression must have
	//been processed to remove whitespace and check for multiple equals signs before
	//being passed in here
	//To set a node from user code, use parseLine
	bool set(const sci::string& expression, sci::string& badExpression, sci::string &error, const VariableStore<nDims>& variables, const FunctionMultiFactory<VariableStore<nDims>>& functions)
	{
		error.clear();
		//clear the subnodes
		m_ownedNodes[0].reset(nullptr);
		m_ownedNodes[1].reset(nullptr);
		m_nodes[0] = nullptr;
		m_nodes[1] = nullptr;
		m_function.reset(nullptr);
		m_constant.reshape({ 0,0 });
		m_expression = expression;
		badExpression.clear();

		std::array<sci::string, 3> split;
		if(!expression.empty()) //we can permit an empty expression when parsing. If still empty at evaluation it will be flagged
			split = splitExpression(expression, error);
		if (!error.empty())
		{
			badExpression = expression;
			return false;
		}
		if (split[0].empty())
		{
			//this is a zero sized variable - we can allow this
			m_op = constantRead;
			return true;
		}
		else if (split[1].empty())
		{
			//we only have the first element, this is either a value
			//or a variable or a function

			//check if it's a function by seeing if it contains a bracket
			size_t bracketIndex = split[0].find(sU('('));
			if (bracketIndex != sci::string::npos)
			{
				if (split[0].back() != sU(')'))
				{
					badExpression = split[0];
					error = sU("missing_function_close_bracket_error");
					return false;
				}
				sci::string functionName = split[0].substr(0, bracketIndex);
				sci::string functionParameters = split[0].substr(bracketIndex + 1, split[0].length() - bracketIndex - 2);
				m_function = functions.getFunction(functionName);
				if (!m_function)
				{
					badExpression = split[0];
					error = sU("function_not_found_error");
					return false;
				}
				bool success = m_function->setInputs(functionParameters, badExpression, error, variables, functions);
				if (!success)
				{
					badExpression = split[0];
					if(error.empty())
						error = sU("function_call_failed_error");
					return false;
				}
				//check for infinite recursion
				if (contains(this))
				{
					badExpression = expression;
					error = sU("infinite_recursion_error");
					return false;
				}
				m_op = runFunction;
				return true;
			}

			//check if it's an array value
			size_t curlyBracketIndex = split[0].find_first_of(sU("{}"));
			if (curlyBracketIndex != sci::string::npos)
			{
				bool result = parseArray(split[0], badExpression, error, m_constant);
				if (!result)
					return false;
				m_op = constantRead;
				return true;
			}

			//check if it's a 1d value
			std::string codePageExpr = basicAscii(split[0]);
			char* check = nullptr;
			double value = std::strtod(codePageExpr.c_str(), &check);
			if (check == codePageExpr.c_str()+codePageExpr.length())
			{
				//create a variable with this value
				index shape;
				for (auto& s : shape)
					s = 1;
				m_constant.clear();
				m_constant.reshape(shape, value);
				m_op = constantRead;
				return true;
			}



			//it wasn't a value or a function - check if it's a variable
			const Node* node = variables.getVariable(split[0]);
			if (!node)
			{
				//it wasn't a variable, return the expression as a bad expression
				badExpression = split[0];
				error = sU("variable_not_found_error");
				return false;
			}
			m_nodes[0] = node;
			m_op = nodePassthrough;

			//check for infinite looping
			//This should catch all cases.
			//e.g.
			//x=2
			//y=x+1
			//x=y+1
			//when parsing x=y+1, we eventually will parse just y in this coe section
			//by with point the y node, contains x and the x node contains y, so the
			//loop is caught
			if (contains(this))
			{
				badExpression = expression;
				error = sU("infinite_recursion_error");
				return false;
			}

			return true;
		}
		else if (split[2].empty())
		{
			//this means we ended on an operator other than equals - obviously bad
			badExpression = expression;
			error = sU("second_operand_missing_error");
			return false;
		}
		else
		{
			//create the sub nodes and get the operator
			m_ownedNodes[0].reset(new Node);
			m_ownedNodes[1].reset(new Node);
			m_nodes[0] = m_ownedNodes[0].get();
			m_nodes[1] = m_ownedNodes[1].get();
			m_op = getOp<VariableStore<nDims>>(split[1]);
			m_ownedNodes[0]->set(split[0], badExpression, error, variables, functions);
			if (!badExpression.empty() || !error.empty())
			{
				return false;
			}
			m_ownedNodes[1]->set(split[2], badExpression, error, variables, functions);
			if (!badExpression.empty() || !error.empty())
			{
				return false;
			}

			//check for infinite looping - I suspect this check isn't needed
			//as the check above when we just have a variable should always catch
			//an infinite loop. But It's here just in case a logic change means the
			//check above no longer catches all cases.
			if (contains(this))
			{
				badExpression = expression;
				error = sU("infinite_recursion_error");
				return false;
			}
		}
		return true;
	}
	sci::string m_expression;
	std::array<const Node<nDims>*, nSubnodes> m_nodes;
	std::array<std::unique_ptr<Node<nDims>>, nSubnodes> m_ownedNodes;
	variable m_constant;
	mutable index m_shape; //mutable because it is just a cache for the shape, updated when getShape is called to avoid calculating it for every getValue call
	std::unique_ptr<Function<VariableStore<nDims>>> m_function;
	decltype(getOp<VariableStore<nDims>>(sci::string())) m_op;

	template<int startIndex>
	static constexpr size_t multArray(const std::array<size_t, nDims> &array, sci::string &error)
	{
		if constexpr (startIndex == nDims)
			return 1;
		else
		{
			size_t next = multArray<startIndex + 1>(array, error);
			if (next > 0 && -1 / next <= array[startIndex])
			{
				error = sU("array_too_large_error");
				return -1; //this guarentees the next level up will generate the same error and return -1. We want to ensure we return nonzero to distinguish between zero size arrays 
			}
			return array[startIndex] * next;
		}
	}

	static constexpr bool isOpenBracket(schar c)
	{
		return c == sU('(');
	}

	static constexpr bool isCloseBracket(schar c)
	{
		return c == sU(')');
	}

	static constexpr bool isOpenCurlyBracket(schar c)
	{
		return c == sU('{');
	}

	static constexpr bool isComma(schar c)
	{
		return c == sU(',');
	}

	static constexpr bool isCloseCurlyBracket(schar c)
	{
		return c == sU('}');
	}

	static constexpr bool isOperator(schar c)
	{
		return c == sU('+') || c == sU('-') || c == sU('*') || c == sU('/') || c == sU('^');
	}

	static constexpr bool isPlusMinus(schar c)
	{
		return c == sU('+') || c == sU('-');
	}

	static constexpr bool isPlus(schar c)
	{
		return c == sU('+');
	}

	static constexpr bool isMinus(schar c)
	{
		return c == sU('-');
	}

	static constexpr bool isMultiply(schar c)
	{
		return c == sU('*');
	}

	static constexpr bool isDivide(schar c)
	{
		return c == sU('/');
	}

	static constexpr bool isPower(schar c)
	{
		return c == sU('^');
	}

	static constexpr bool isExp(schar c)
	{
		return c == sU('e') || c == sU('E');
	}

	static constexpr bool isWhiteSpace(schar c)
	{
		return c == sU(' ') || c == sU('\t');
	}

	static constexpr std::string basicAscii(const sci::string& str)
	{
		std::string result;
		result.resize(str.length(), '?');
		for (size_t i = 0; i < str.length(); ++i)
			if (str[i] < 128)
				result[i] = char(str[i]);
		return result;
	}

	static size_t parseArray(const sci::string& str, sci::string& badExpression, sci::string& error, variable& result)
	{
		//check for a sensible length
		if (str.length() < 2)
		{
			badExpression = str;
			error = sU("array_parse_error");
			return false;
		}

		//when it arrives here it should already have been trimmed so the first and last elements should be { and }

		// check if this is a 1d or 2d array by checking the 2nd element
		size_t nextNonWhiteSpace = str.find_first_not_of(sU(" \t"), 1);
		if (nextNonWhiteSpace == sci::string::npos)
		{
			badExpression = str;
			error = sU("array_parse_error");
			return false;
		}
		bool twoD = isOpenCurlyBracket(str[nextNonWhiteSpace]);

		if (twoD)
		{
			if (!isCloseCurlyBracket(str.back()))
			{
				badExpression = str;
				error = sU("missing_array_close_bracket_error");
				return false;
			}
		}

		size_t start = twoD ? nextNonWhiteSpace : 0;
		size_t end = twoD ? str.length() - 1 : str.length();

		if (!isOpenCurlyBracket(str[start]))
		{
			badExpression = str.substr(start, end-start);
			error = sU("missing_array_open_bracket_error");
			return false;
		}

		//check how many rows and columns we have and check each row
		//has the same number of columns
		size_t nCommas = -1;
		size_t nRowCommas = 0;
		size_t nRows = 0;
		int bracketDepth = 0;
		bool isEmpty = false;
		size_t rowStart = -1;
		for (size_t i = start; i < end; ++i)
		{
			if (isOpenCurlyBracket(str[i]))
			{
				++nRows;
				rowStart = i;
				if (bracketDepth != 0)
				{
					error = sU("curly_bracket_depth_greater_that_2_error");
					return false;
				}
				++bracketDepth;
			}
			if (isComma(str[i]))
			{
				++nRowCommas;
			}
			if (isCloseCurlyBracket(str[i]))
			{
				if (bracketDepth != 1)
				{
					error = sU("missing_array_open_bracket_error");
					badExpression = str.substr(rowStart, i - rowStart + 1);
					return false;
				}
				if (nRows == 1)
					nCommas = nRowCommas;
				else if (nRowCommas != nCommas)
				{
					error = sU("array_2nd_dimension_sizes_do_not_match_error");
					badExpression = str.substr(rowStart, i - rowStart + 1);
					return false;
				}
				--bracketDepth;
				nRowCommas = 0;
			}
			else if (twoD && !isOpenCurlyBracket(str[i]) && bracketDepth == 0 && !isWhiteSpace(str[i]))
			{
				badExpression = str.substr(i, 1);
				error = sU("unexpected_character_between_array_brackets_error");
				return false;
			}
			else if (!twoD && bracketDepth == 0 && !isWhiteSpace(str[i]))
			{
				badExpression = str.substr(i);
				error = sU("unexpected_characters_after_array_error");
				return false;
			}
		}
		if (bracketDepth != 0)
		{
			badExpression = str;
			error = sU("missing_array_close_bracket_error");
			return false;
		}

		size_t nColumns = nCommas + 1;
		result.reshape({ nRows, nColumns });

		size_t pos = start;
		std::string codePageStr;
		for (auto& r : result)
		{
			if (pos == sci::string::npos)
			{
				error = sU("invalid_or_missing_array_element_error");
				badExpression = str.substr(end);
				return false;
			}
			if (isOpenCurlyBracket(str[pos]))
				++pos;
			size_t end = str.find_first_of(sU(",}"), pos);
			codePageStr = basicAscii(str.substr(pos, end - pos));
			char* check = nullptr;
			r = std::strtod(codePageStr.c_str(), &check);
			if (check != codePageStr.c_str() + codePageStr.length() || codePageStr.length() == 0)
			{
				error = sU("invalid_or_missing_array_element_error");
				badExpression = sci::toSci(codePageStr);
				return false;
			}
			pos = str.find_first_not_of(sU(",}{ \t"), end + 1);
		}
		return true;
	}

	static size_t findCloseBracket(const sci::string& expression, size_t openBracketIndex)
	{
		size_t count = 0;
		for (size_t i = openBracketIndex + 1; i < expression.length(); ++i)
		{
			if (isCloseBracket(expression[i]))
			{
				if (count == 0)
					return i;
				else
					--count;
			}
			else if (isOpenBracket(expression[i]))
				++count;
		}
		//if we get to the end without finding the bracket, then we don't have one
		return -1;
	}

	static constexpr std::array<sci::string, 3> splitExpression(const sci::string& expression, sci::string &error)
	{
		std::array<sci::string, 3> result;
		const std::array<sci::string, 3> badResult{ sU(""), sU(""), sU("") };

		if (expression.length() == 0)
		{
			error = sU("parser_bug_zero_sized_expression_sent_to_splitExpression_error");
			return badResult;
		}


		//check if the whole expression is enclosed in brackets and reparse without the brackets if so
		if (isOpenBracket(expression[0]))
		{
			//find the close bracket
			size_t closeBracketIndex = findCloseBracket(expression, 0);
			if (closeBracketIndex == -1)
			{
				error = sU("missing_close_bracket_error");
				return badResult;
			}
			if (closeBracketIndex == expression.length() - 1)
				return splitExpression(expression.substr(1, expression.length() - 2), error);
		}

		//check if the expression starts with +
		if (isPlus(expression[0]))
		{
			//chop off the plus and reparse
			return splitExpression(expression.substr(1), error);
		}

		//check if the expression starts with a minus
		if (isMinus(expression[0]))
		{
			//reparse as -1*the_rest. If we already did this and the expression is "-1" just return it as a constant
			//check for "-1"
			if (expression == sU("-1"))
				return std::array<sci::string, 3>{sU("-1"), sU(""), sU("")};

			//otherwise clip off the - and return -1*the rest
			return splitExpression(sU("(-1)*") + expression.substr(1), error);
		}

		//find positions of +/-, multiply and divide and power outside of any brackets.
		//The point we split on will be the last operation to be carried out in the expression
		//So we want to any + or - (as they are commutative), or if there are none, the rightmost / or *,
		//or if ther are none, the leftmost ^. note that a^b^c = a^(b^c), not (a^b)^c, which
		//is the correct maths convention, but oposite to Excel and some other software.
		size_t multiplyDivideIndex = -1;
		size_t powerIndex = -1;
		size_t addSubtractIndex = -1;
		size_t lastCloseBracketIndex = -1;
		int bracketCount = 0;
		int curlyBracketCount = 0;
		bool hasSpace = false;
		for (size_t i = 0; i < expression.length(); ++i)
		{
			if (isOpenBracket(expression[i]))
			{
				++bracketCount;
				continue;
			}
			else if (isCloseBracket(expression[i]))
			{
				--bracketCount;
				lastCloseBracketIndex = i;
				continue;
			}
			else if (isOpenCurlyBracket(expression[i]))
			{
				++curlyBracketCount;
				continue;
			}
			else if (isCloseCurlyBracket(expression[i]))
			{
				--curlyBracketCount;
				continue;
			}

			if (bracketCount > 0 || curlyBracketCount != 0)
				continue;

			else if (isPlusMinus(expression[i]) && curlyBracketCount == 0)
			{
				//check this isn't a sign change after a multiply or divide or part of an exponent
				if (i > 0 && (isMultiply(expression[i - 1]) || isDivide(expression[i - 1]) || isExp(expression[i - 1])))
				{
					//do nothing, don't split here
				}
				//don't split here if we are immediately after another plus or minus, this allows chaining them
				else if (i > 0 && isPlusMinus(expression[i - 1]))
				{
					//do nothing, don't split here
				}

				else
				{
					addSubtractIndex = i;
				}
			}
			else if (isMultiply(expression[i]) && curlyBracketCount == 0)
			{
				multiplyDivideIndex = i;
			}
			else if (isDivide(expression[i]) && curlyBracketCount == 0)
			{
				multiplyDivideIndex = i;
			}
			else if (isPower(expression[i]) && powerIndex == -1 && curlyBracketCount == 0) //get the first, not last power
			{
				powerIndex = i;
			}
			else if (expression[i] == sU(' '))
				hasSpace = true;
		}

		//report a missing open bracket error or reevaluate the indices as though there
		//was an open bracket at the start of the expression
		if (bracketCount < 0)
		{
			if (powerIndex < lastCloseBracketIndex)
				powerIndex = -1;
			if (addSubtractIndex < lastCloseBracketIndex)
				addSubtractIndex = -1;
			if (multiplyDivideIndex < lastCloseBracketIndex)
				multiplyDivideIndex = -1;
			
			if (powerIndex == -1 && addSubtractIndex == -1 && multiplyDivideIndex ==-1)
			{
				error = sU("missing_open_bracket_error");
				return badResult;
			}
		}
		//split on an add or subtract if we found one
		if(addSubtractIndex != -1)
			return std::array<sci::string, 3>{expression.substr(0, addSubtractIndex),
				expression.substr(addSubtractIndex, 1),
				expression.substr(addSubtractIndex + 1)};
		//split on a multiply if we found one
		if (multiplyDivideIndex != -1)
			return std::array<sci::string, 3>{expression.substr(0, multiplyDivideIndex),
			expression.substr(multiplyDivideIndex, 1),
			expression.substr(multiplyDivideIndex + 1)};
		//split on a power if we found one
		if (powerIndex != -1)
			return std::array<sci::string, 3>{expression.substr(0, powerIndex),
			expression.substr(powerIndex, 1),
			expression.substr(powerIndex + 1)};

		if (hasSpace)
		{

			error = sU("missing_operator_error");
			return badResult;
		}
		//if we reach here we have a variable or constant or function
		return std::array<sci::string, 3>{expression, sU(""), sU("")};
	}
};