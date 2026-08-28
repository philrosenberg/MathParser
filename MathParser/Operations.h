#pragma once
#include"types.h"
class Function;
class Node;
typedef double (*op)(Function*, const variable* v, const index& shape, const std::array<const Node*, nSubnodes>&, const index& position);

double add(Function*, const variable*, const index&, const std::array<const Node*, nSubnodes>& nodes, const index& position);
double subtract(Function*, const variable*, const index&, const std::array<const Node*, nSubnodes>& nodes, const index& position);
double multiply(Function*, const variable*, const index&, const std::array<const Node*, nSubnodes>& nodes, const index& position);
double divide(Function*, const variable*, const index&, const std::array<const Node*, nSubnodes>& nodes, const index& position);
double power(Function*, const variable*, const index&, const std::array<const Node*, nSubnodes>& nodes, const index& position);
double nodePassthrough(Function*, const variable*, const index&, const std::array<const Node*, nSubnodes>& nodes, const index& position);
double constantRead(Function*, const variable* v, const index& shape, const std::array<const Node*, nSubnodes>&, const index& position);
double runFunction(Function* function, const variable*, const index&, const std::array<const Node*, nSubnodes>&, const index& position);
op getOp(sci::string opString);
