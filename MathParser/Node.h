#pragma once
#include"types.h"
#include"FunctionBase.h"
#include"Operations.h"
#include"VariableStore.h"
#include<memory>

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
	friend bool parseLine< VariableStore<nDims>>(const sci::string& line, sci::string& badLine, VariableStore<nDims>* variableDestination, const VariableStore<nDims>& variableSource, const FunctionMultiFactory<VariableStore<nDims>>& functions, Node& node, bool& isAssignment);
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
		return false;
	}

	variable calculate() const
	{
		getShape(); // this updates the shape
		variable result(m_shape);
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

	index getShape() const
	{
		if (m_op == runFunction)
			m_shape = m_function->getShapeAndCacheIfNeeded();
		else if (m_op == constantRead)
			m_shape = m_constant.shape();
		else if (m_op == nodePassthrough)
			m_shape = m_nodes[0]->getShape();
		else
		{
			index shape1 = m_nodes[0]->getShape();
			index shape2 = m_nodes[1]->getShape();
			for (size_t i = 0; i < m_shape.size(); ++i)
			{
				m_shape[i] = 0;
				if (shape1[i] * shape2[i] > 0)
					m_shape[i] = std::max(shape1[i], shape2[i]);
			}
		}

		return m_shape;
	}
	constexpr double getValue(const index& position) const
	{
		return m_op(m_function.get(), &m_constant, m_shape, m_nodes, position);
	}
	static constexpr size_t getSizeFromShape(const index &shape)
	{
		return multArray<0>(shape);
	}
private:
	//set is private to restrict the callers. This is because expression must have
	//been processed to remove whitespace and check for multiple equals signs before
	//being passed in here
	//To set a node from user code, use parseLine
	bool set(const sci::string& expression, sci::string& badExpression, const VariableStore<nDims>& variables, const FunctionMultiFactory<VariableStore<nDims>>& functions)
	{
		m_expression = expression;
		badExpression.clear();
		std::array<sci::string, 3> split = splitExpression(expression);
		if (split[0].empty())
		{
			//this is a zero sized variable - we can allow this
			m_ownedNodes[0].reset(nullptr);
			m_ownedNodes[1].reset(nullptr);
			m_nodes[0] = nullptr;
			m_nodes[1] = nullptr;
			m_function.reset(nullptr);
			m_constant.reshape({ 0,0 });
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
					return false;
				}
				sci::string functionName = split[0].substr(0, bracketIndex);
				sci::string functionParameters = split[0].substr(bracketIndex + 1, split[0].length() - bracketIndex - 2);
				m_function = functions.getFunction(functionName);
				if (!m_function)
				{
					badExpression = split[0];
					return false;
				}
				bool success = m_function->setInputs(functionParameters, badExpression, variables, functions);
				if (!success)
				{
					return false;
				}
				m_op = runFunction;
				return true;
			}

			//check if it's an array value
			size_t curlyBracketIndex = split[0].find(sU('{'));
			if (curlyBracketIndex != sci::string::npos)
			{
				bool result = parseArray(split[0], m_constant);
				if (result)
					m_op = constantRead;
				else
				{
					badExpression = split[0];
				}
				return result;
			}

			//check if it's a 1d value
			std::string codePageExpr = basicAscii(split[0]);
			char* check = nullptr;
			double value = std::strtod(codePageExpr.c_str(), &check);
			if (check != codePageExpr.c_str())
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
				return false;
			}

			return true;
		}
		else if (split[2].empty())
		{
			//this means we ended on an operator other than equals - obviously bad
			badExpression = expression;
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
			m_ownedNodes[0]->set(split[0], badExpression, variables, functions);
			if (!badExpression.empty())
			{
				return false;
			}
			m_ownedNodes[1]->set(split[2], badExpression, variables, functions);
			if (!badExpression.empty())
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
	static constexpr size_t multArray(const std::array<size_t, nDims> &array)
	{
		if constexpr (startIndex == nDims)
			return 1;
		else
			return array[startIndex] * multArray<startIndex + 1>(array);
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
		return c == sU('+') || c == sU('-') || c == sU('*') || c == sU('/') || c == sU('*');
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

	static size_t parseArray(const sci::string& str, variable& result)
	{
		//check for a sensible length
		if (str.length() < 2)
			return false;

		//when it arrives here it should already have been trimmed so the first and last elements should be { and }

		// check if this is a 1d or 2d array by checking the 2nd element
		size_t nextNonWhiteSpace = str.find_first_not_of(sU(" \t"), 1);
		if (nextNonWhiteSpace == sci::string::npos)
			return false;
		bool twoD = isOpenCurlyBracket(str[nextNonWhiteSpace]);

		if (twoD)
		{
			if (!isCloseCurlyBracket(str.back()))
				return false;
		}

		size_t start = twoD ? nextNonWhiteSpace : 0;
		size_t end = twoD ? str.length() - 1 : str.length();

		if (!isOpenCurlyBracket(str[start]))
			return false;

		//check how many rows and columns we have and check each row
		//has the same number of columns
		size_t nCommas = -1;
		size_t nRowCommas = 0;
		size_t nRows = 0;
		int bracketDepth = 0;
		for (size_t i = start; i < end; ++i)
		{
			if (isOpenCurlyBracket(str[i]))
			{
				++nRows;
				if (bracketDepth != 0)
					return false;
				++bracketDepth;
			}
			if (isComma(str[i]))
			{
				++nRowCommas;
			}
			if (isCloseCurlyBracket(str[i]))
			{
				if (bracketDepth != 1)
					return false;
				if (nRows > 1 && nRowCommas != nCommas)
					return false;
				if (nRows == 1)
					nCommas = nRowCommas;
				--bracketDepth;
				nRowCommas = 0;
			}
			if (!isCloseCurlyBracket(str[i]) && bracketDepth == 0 && !isWhiteSpace(str[i]))
				return false;
		}
		if (bracketDepth != 0)
			return false;

		size_t nColumns = nCommas + 1;
		result.reshape({ nRows, nColumns });

		size_t pos = start;
		std::string codePageStr;
		for (auto& r : result)
		{
			if (isOpenCurlyBracket(str[pos]))
				++pos;
			size_t end = str.find_first_of(sU(",}"), pos);
			codePageStr = basicAscii(str.substr(pos, end - pos));
			char* check = nullptr;
			r = std::strtod(codePageStr.c_str(), &check);
			if (check == codePageStr.c_str())
				return false;
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

	static constexpr std::array<sci::string, 3> splitExpression(const sci::string& expression)
	{
		std::array<sci::string, 3> result;
		const std::array<sci::string, 3> badResult{ sU(""), sU(""), sU("") };

		if (expression.length() == 0)
			return badResult;


		//check if the whole expression is enclosed in brackets and reparse without the brackets if so
		if (isOpenBracket(expression[0]))
		{
			//find the close bracket
			size_t closeBracketIndex = findCloseBracket(expression, 0);
			if (closeBracketIndex == -1)
				return badResult;
			if (closeBracketIndex == expression.length() - 1)
				return splitExpression(expression.substr(1, expression.length() - 2));
		}

		//check if the expression starts with +
		if (isPlus(expression[0]))
		{
			//chop off the plus and reparse
			return splitExpression(expression.substr(1));
		}

		//check if the expression starts with a minus
		if (isMinus(expression[0]))
		{
			//reparse as -1*the_rest. If we already did this and the expression is "-1" just return it as a constant
			//check for "-1"
			if (expression == sU("-1"))
				return std::array<sci::string, 3>{sU("-1"), sU(""), sU("")};

			//otherwise clip off the - and return -1*the rest
			return splitExpression(sU("(-1)*") + expression.substr(1));
		}

		//find positions of +/-, multiply and divide and power outside of any brackets.
		//The point we split on will be the last operation to be carried out in the expression
		//So we want to any + or - (as they are commutative), or if there are none, the rightmost / or *,
		//or if ther are none, the leftmost ^. note that a^b^c = a^(b^c), not (a^b)^c, which
		//is the correct maths convention, but oposite to Excel and some other software.
		size_t multiplyDivideIndex = -1;
		size_t powerIndex = -1;
		size_t addSubtractIndex = -1;
		int bracketCount = 0;
		int curlyBracketCount = 0;
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

			if (bracketCount != 0 || curlyBracketCount != 0)
				continue;

			else if (isPlusMinus(expression[i]))
			{
				//check this isn't a sign change after a multiply or divide or part of an exponent
				if (i > 0 && (isMultiply(expression[i - 1]) || isDivide(expression[i - 1]) || isExp(expression[i - 1])))
				{
					//do nothing, don't split here
				}
				else
				{
					if (curlyBracketCount != 0)
						return badResult;
					addSubtractIndex = i;
				}
			}
			else if (isMultiply(expression[i]))
			{
				if (curlyBracketCount != 0)
					return badResult;
				multiplyDivideIndex = i;
			}
			else if (isDivide(expression[i]))
			{
				if (curlyBracketCount != 0)
					return badResult;
				multiplyDivideIndex = i;
			}
			else if (isPower(expression[i]) && powerIndex == -1) //get the first, not last power
			{
				if (curlyBracketCount != 0)
					return badResult;
				powerIndex = i;
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


		//if we reach here we have a variable or constant or function
		return std::array<sci::string, 3>{expression, sU(""), sU("")};
	}
};