#pragma once
#include"types.h"
#include"FunctionBase.h"
#include"Operations.h"
#include"VariableStore.h"
#include<memory>

class FunctionMultiFactory;

class Node
{

	//note that all public member functions are const
	//this is by design, so only controlled functions can modify Nodes

public:
	friend bool parseLine(const sci::string& line, sci::string& badLine, VariableStore* variableDestination, const VariableStore& variableSource, const FunctionMultiFactory& functions, Node& node, bool& isAssignment);
	friend class VariableStore;
	Node()
	{
		m_function = nullptr;
		m_op = nullptr;
		for (auto& n : m_nodes)
			n = nullptr;
		for (auto& s : m_shape)
			s = 0;
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

	index getShape() const;
	constexpr double getValue(const index& position) const
	{
		return m_op(m_function.get(), &m_constant, m_shape, m_nodes, position);
	}
private:
	//set is private to restrict the callers. This is because expression must have
	//been processed to remove whitespace and check for multiple equals signs before
	//being passed in here
	//To set a node from user code, use parseLine
	bool set(const sci::string& expression, sci::string& badExpression, const VariableStore& variables, const FunctionMultiFactory& functions);
	sci::string m_expression;
	std::array<const Node*, nSubnodes> m_nodes;
	std::array<std::unique_ptr<Node>, nSubnodes> m_ownedNodes;
	variable m_constant;
	mutable index m_shape; //mutable because it is just a cache for the shape, updated when getShape is called to avoid calculating it for every getValue call
	std::unique_ptr<Function> m_function;
	op m_op;
};