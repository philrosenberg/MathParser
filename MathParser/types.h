#pragma once
#include<vector>
#include<scieng/string.h>
#include<scieng/grid.h>
#include<scieng/gridtransformview.h>

const size_t nDims = 2;
const size_t nSubnodes = 2;
using split = std::vector<sci::string>;
using schar = sci::string::value_type;
using variable = sci::GridData<double, nDims>;
using index = std::array<size_t, nDims>;

inline size_t variableSize(index shape)
{
	return(shape[0] * shape[1]);
}