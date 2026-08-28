#include"Operations.h"
#include"FunctionBase.h"
#include"Node.h"

double at(const Node* n, const index& position)
{
	return n->getValue(position);
}

double add(Function*, const variable*, const index&, const std::array<const Node*, nSubnodes>& nodes, const index& position)
{
	return at(nodes[0], position) + at(nodes[1], position);
}
double subtract(Function*, const variable*, const index&, const std::array<const Node*, nSubnodes>& nodes, const index& position)
{
	return at(nodes[0], position) - at(nodes[1], position);
}
double multiply(Function*, const variable*, const index&, const std::array<const Node*, nSubnodes>& nodes, const index& position)
{
	return at(nodes[0], position) * at(nodes[1], position);
}
double divide(Function*, const variable*, const index&, const std::array<const Node*, nSubnodes>& nodes, const index& position)
{
	return at(nodes[0], position) / at(nodes[1], position);
}
double power(Function*, const variable*, const index&, const std::array<const Node*, nSubnodes>& nodes, const index& position)
{
	return std::pow(at(nodes[0], position), at(nodes[1], position));
}
double nodePassthrough(Function*, const variable*, const index&, const std::array<const Node*, nSubnodes>& nodes, const index& position)
{
	return at(nodes[0], position);
}
double constantRead(Function*, const variable* v, const index& shape, const std::array<const Node*, nSubnodes>&, const index& position)
{
	return (*v)[position[0] % shape[0]][position[1] % shape[1]];
}
double runFunction(Function* function, const variable*, const index&, const std::array<const Node*, nSubnodes>&, const index& position)
{
	return function->getValue(position);
}

op getOp(sci::string opString)
{
	if (opString == sU("+"))
		return add;
	if (opString == sU("-"))
		return subtract;
	if (opString == sU("*"))
		return multiply;
	if (opString == sU("/"))
		return divide;
	if (opString == sU("^"))
		return power;
	return nullptr;
}