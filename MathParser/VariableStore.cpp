#include"VariableStore.h"
#include"Node.h"

Node* VariableStore::getOrCreateVariable(sci::string variableName)
{
	auto result = m_variables.find(variableName);
	if (result != m_variables.end())
		return result->second;
	else
	{
		m_nodes.push_front(Node());
		auto nodeIt = m_nodes.begin();
		Node* nodePtr = &*nodeIt;
		m_variables.insert(std::make_pair(variableName, nodePtr));
		return nodePtr;
	}
}