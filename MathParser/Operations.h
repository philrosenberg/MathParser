#pragma once
#include"types.h"
template<class VariableStore>
class Function;
template<int nDims>
class Node;

//this was the type of all the functions below
//template<class VariableStore>
//typedef double (*op)(Function<VariableStore>*, const typename VariableStore::node::variable* v, const typename VariableStore::node::index& shape, const std::array<const typename VariableStore::node*, nSubnodes>&, const typename VariableStore::node::index& position);

template<class VariableStore>
double add(Function<VariableStore>*, const typename VariableStore::node::variable* v, const typename VariableStore::node::index& shape, const std::array<const typename VariableStore::node*, nSubnodes>& nodes, const typename VariableStore::node::index& position)
{
	return nodes[0]->getValue(position) + nodes[1]->getValue(position);
}

template<class VariableStore>
double subtract(Function<VariableStore>*, const typename VariableStore::node::variable* v, const typename VariableStore::node::index& shape, const std::array<const typename VariableStore::node*, nSubnodes>& nodes, const typename VariableStore::node::index& position)
{
	return nodes[0]->getValue(position) - nodes[1]->getValue(position);
}

template<class VariableStore>
double multiply(Function<VariableStore>*, const typename VariableStore::node::variable* v, const typename VariableStore::node::index& shape, const std::array<const typename VariableStore::node*, nSubnodes>& nodes, const typename VariableStore::node::index& position)
{
	return nodes[0]->getValue(position) * nodes[1]->getValue(position);
}

template<class VariableStore>
double divide(Function<VariableStore>*, const typename VariableStore::node::variable* v, const typename VariableStore::node::index& shape, const std::array<const typename VariableStore::node*, nSubnodes>& nodes, const typename VariableStore::node::index& position)
{
	return nodes[0]->getValue(position) / nodes[1]->getValue(position);
}

template<class VariableStore>
double power(Function<VariableStore>*, const typename VariableStore::node::variable* v, const typename VariableStore::node::index& shape, const std::array<const typename VariableStore::node*, nSubnodes>& nodes, const typename VariableStore::node::index& position)
{
	return std::pow(nodes[0]->getValue(position), nodes[1]->getValue(position));
}

template<class VariableStore>
double nodePassthrough(Function<VariableStore>*, const typename VariableStore::node::variable* v, const typename VariableStore::node::index& shape, const std::array<const typename VariableStore::node*, nSubnodes>& nodes, const typename VariableStore::node::index& position)
{
	return nodes[0]->getValue(position);
}

template<class VariableStore>
double constantRead(Function<VariableStore>*, const typename VariableStore::node::variable* v, const typename VariableStore::node::index& shape, const std::array<const typename VariableStore::node*, nSubnodes>&, const typename VariableStore::node::index& position)
{
	return (*v)[position[0] % shape[0]][position[1] % shape[1]];
}

template<class VariableStore>
double runFunction(Function<VariableStore>* function, const typename VariableStore::node::variable* v, const typename VariableStore::node::index& shape, const std::array<const typename VariableStore::node*, nSubnodes>&, const typename VariableStore::node::index& position)
{
	return function->getValue(position);
}

template<class VariableStore>
double nullOperator(Function<VariableStore>* function, const typename VariableStore::node::variable* v, const typename VariableStore::node::index& shape, const std::array<const typename VariableStore::node*, nSubnodes>&, const typename VariableStore::node::index& position)
{
	//this is basically an error if it ever gets called
	return std::numeric_limits<double>::quiet_NaN();
}


template<class VariableStore>
auto getOp(const sci::string &opString)
{
	if (opString == sU("+"))
		return &add<VariableStore>;
	if (opString == sU("-"))
		return &subtract<VariableStore>;
	if (opString == sU("*"))
		return &multiply<VariableStore>;
	if (opString == sU("/"))
		return &divide<VariableStore>;
	if (opString == sU("^"))
		return &power<VariableStore>;
	return &nullOperator<VariableStore>;
}
