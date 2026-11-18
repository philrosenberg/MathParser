#pragma once
#include"types.h"
#include<forward_list>
#include"Node.h"
#include<scieng/codepage.h>

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
	static const sci::string& getDisallowedCharacters()
	{
		return m_disallowedCharacters;
	}
	static bool checkValidVariableName(const sci::string & name)
	{
		//zero length variables are not allowed
		if (name.length() == 0)
			return false;

		//variables that are numbers are not allowed
		std::string codepageName = sci::toCodepage(name);
		if (codepageName.length() > 0)
		{
			char* end = nullptr;
			std::strtod(&codepageName[0], &end);
			if (end == &codepageName[0] + codepageName.length())
				return false;
		}

		//veriables containign prohibited characters are not allowed
		return name.find_first_of(getDisallowedCharacters()) == sci::string::npos;
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
	static const inline sci::string m_disallowedCharacters = sU("+-*/(){},^= \t");
	std::forward_list<node> m_nodes;
	dict m_variables;

};