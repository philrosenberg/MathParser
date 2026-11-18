#pragma once
#include"types.h"
#include<forward_list>
#include"Node.h"

template<class VariableStore>
class FunctionMultiFactory;

template<int nDims>
class VariableStore
{
public:
	using node = Node<nDims>;
	using dict = std::unordered_map<sci::string, Node<nDims>*>;
	node* getVariable(sci::string variableName) const
	{
		auto result = m_variables.find(variableName);
		if (result == m_variables.end())
			return nullptr;
		else
			return result->second;
	}
	node* getOrCreateVariable(sci::string variableName)
	{
		if (!checkValidVariableName(variableName))
			return nullptr;
		auto result = m_variables.find(variableName);
		if (result != m_variables.end())
			return result->second;
		else
		{
			m_nodes.push_front(node());
			auto nodeIt = m_nodes.begin();
			node* nodePtr = &*nodeIt;
			m_variables.insert(std::make_pair(variableName, nodePtr));
			return nodePtr;
		}
	}
	static bool checkValidVariableName(const sci::string & name)
	{
		const sci::string invalidCharacters(sU("+-*/(){},^"));
		return name.find_first_of(invalidCharacters) == sci::string::npos;
	}
	//There is currently no way to safely remove variables without
	// leaving dangling pointers, so we don't do it
	/*void removeVariable(sci::string variableName)
	{
		auto variable = m_variables.find(variableName);
		if (variable != m_variables.end())
			m_variables.erase(variable);
	}*/
private:
	std::forward_list<node> m_nodes;
	dict m_variables;

};