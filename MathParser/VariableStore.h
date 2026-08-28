#pragma once
#include"types.h"
#include<forward_list>

class Node;
class FunctionMultiFactory;
using dict = std::unordered_map<sci::string, Node*>;

class VariableStore
{
public:
	Node* getVariable(sci::string variableName) const
	{
		auto result = m_variables.find(variableName);
		if (result == m_variables.end())
			return nullptr;
		else
			return result->second;
	}
	Node* getOrCreateVariable(sci::string variableName);
	void removeVariable(sci::string variableName)
	{
		auto variable = m_variables.find(variableName);
		if (variable != m_variables.end())
			m_variables.erase(variable);
	}
private:
	std::forward_list<Node> m_nodes;
	dict m_variables;

};