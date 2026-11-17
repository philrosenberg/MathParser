#pragma once
#include"FunctionBase.h"
#include"Node.h"

//this is a function which accepts N expressions, each separated by a comma. Each expression will be converted to a node
template<size_t N, class VariableStore>
class CommaSeparatedInputsFunction : public Function<VariableStore>
{
public:
	virtual bool setInputs(const sci::string inputs, sci::string& badExpression, const VariableStore& variables, const FunctionMultiFactory<VariableStore>& functions)
	{
		//split the inputs via commas
		size_t comma = -1;
		std::vector<sci::string> expressions;
		int curlyBracketLevel = 0;
		for (size_t i = 0; i < inputs.length(); ++i)
		{
			if (inputs[i] == sU('{'))
				++curlyBracketLevel;
			else if(inputs[i] == sU('}'))
				--curlyBracketLevel;
			else if (curlyBracketLevel == 0 && inputs[i] == sU(','))
			{
				size_t begin = comma + 1;
				comma = i;
				expressions.push_back(inputs.substr(begin, comma - begin));
			}
		}
		size_t begin = comma + 1;
		comma = inputs.length();
		expressions.push_back(inputs.substr(begin, comma - begin));
		
		if (expressions.size() != N)
		{
			badExpression = inputs;
			return false;
		}
		//turn the text expressions to nodes
		for (size_t i = 0; i < m_ownedNodes.size(); ++i)
		{
			m_ownedNodes[i].reset(new typename VariableStore::node());
			bool success = parseExpression(expressions[i], badExpression, variables, functions, *m_ownedNodes[i].get());
			if (!success)
				return false;
			m_nodes.push_back(m_ownedNodes[i].get());
		}
		return true;
	}
	//virtual double getValue(const index& position) const = 0;
	virtual ~CommaSeparatedInputsFunction()
	{}
protected:
	std::array<std::unique_ptr<typename VariableStore::node>, N>m_ownedNodes;
	sci::GridData<const typename VariableStore::node*, 1>m_nodes;
};

template<size_t N, class VariableStore>
class ElementwiseCommaSeparatedInputsFunction : public CommaSeparatedInputsFunction<N, VariableStore>
{
public:
	virtual typename VariableStore::node::index getShapeAndCacheIfNeeded() const override
	{
		typename VariableStore::node::index result;
		for (auto& r : result)
			r = 0;
		for (auto n : CommaSeparatedInputsFunction<N, VariableStore>::m_nodes)
		{
			typename VariableStore::node::index thisShape = n->getShape();
			for (size_t i = 0; i < thisShape.size(); ++i)
				result[i] = std::max(result[i], thisShape[i]);
		}
		return result;
	}
	virtual ~ElementwiseCommaSeparatedInputsFunction()
	{}
};

using ElementwiseFunc1Param = double(*)(double);
using ElementwiseFunc2Param = double(*)(double, double);
template<size_t N>
using ElementwiseFuncNParam = double(*)(std::array<double, N>);
template<size_t N, class VariableStore>
using CacheFuncNParam = void(*)(typename VariableStore::node::variable&, std::array<const typename VariableStore::node*, N>);


//this class creates a function that will apply a scalar function accepting one parameter
//to every element of an input.
template<ElementwiseFunc1Param FUNC, class VariableStore>
class EasyElementwiseFunction1Param
{
public:
	class Function : public ElementwiseCommaSeparatedInputsFunction<1, VariableStore>
	{
	public:
		double getValue(const typename VariableStore::node::index& position) const override
		{
			return FUNC(CommaSeparatedInputsFunction<1, VariableStore>::m_nodes[0]->getValue(position));
		}
	private:
	};
	class Factory : public FunctionFactory<VariableStore>
	{
		Function* makeInstance() const override
		{
			return new EasyElementwiseFunction1Param<FUNC, VariableStore>::Function();
		}
	};
};

//this class creates a function that will apply a scalar function accepting N_PARAMS
//parameters inside a std::array to every element of a set of imputs to give the elements
//of an output.
template<size_t N_PARAMS, ElementwiseFuncNParam<N_PARAMS> FUNC, class VariableStore>
class EasyElementwiseFunction
{
public:
	class Function : public ElementwiseCommaSeparatedInputsFunction<N_PARAMS, VariableStore>
	{
	public:
		double getValue(const typename VariableStore::node::index& position) const override
		{
			std::array<double, N_PARAMS> inputs;
			for (size_t i = 0; i < N_PARAMS; ++i)
				inputs[i] = CommaSeparatedInputsFunction<N_PARAMS, VariableStore>::m_nodes[i]->getValue(position);
			return FUNC(inputs);
		}
	private:

	};
	class Factory : public FunctionFactory<VariableStore>
	{
		Function* makeInstance() const override
		{
			return new EasyElementwiseFunction<N_PARAMS, FUNC, VariableStore>::Function();
		}
	};
};

template<size_t N_PARAMS, class VariableStore, CacheFuncNParam<N_PARAMS, VariableStore> FUNC>
//template<size_t N_PARAMS, class VariableStore, auto FUNC>
class CachingFunction
{
public:
	class Function : public CommaSeparatedInputsFunction<N_PARAMS, VariableStore>
	{
	public:
		VariableStore::node::index getShapeAndCacheIfNeeded() const override
		{
			std::array<const typename VariableStore::node*, N_PARAMS> inputs;
			for (size_t i = 0; i < N_PARAMS; ++i)
				inputs[i] = CommaSeparatedInputsFunction<N_PARAMS, VariableStore>::m_nodes[i];
			FUNC(m_cache, inputs);
			return m_cache.shape();
		}
		double getValue(const typename VariableStore::node::index& position) const override
		{
			typename VariableStore::node::index p = position;
			typename VariableStore::node::index shape = m_cache.shape();
			for (size_t i = 0; i < p.size(); ++i)
				p[i] = p[i] % shape[i];
			return m_cache [p];
		}
	protected:
		mutable VariableStore::node::variable m_cache;
	};
	class Factory : public FunctionFactory<VariableStore>
	{
		Function* makeInstance() const override
		{
			return new CachingFunction<N_PARAMS, VariableStore, FUNC>::Function();
		}
	};
};

template<class VariableStore>
class FreezeFunction
{
public:
	class Function : public CommaSeparatedInputsFunction<1, VariableStore>
	{
	public:
		bool setInputs(const sci::string inputs, sci::string& badExpression, const VariableStore& variables, const FunctionMultiFactory<VariableStore>& functions) override
		{
			bool result = CommaSeparatedInputsFunction<1, VariableStore>::setInputs(inputs, badExpression, variables, functions);
			if(result)
				m_cache = CommaSeparatedInputsFunction<1, VariableStore>::m_nodes[0]->calculate();
			return result;
		}
		typename VariableStore::node::index getShapeAndCacheIfNeeded() const override
		{
			return m_cache.shape();
		}
		double getValue(const typename VariableStore::node::index& position) const override
		{
			typename VariableStore::node::index p = position;
			typename VariableStore::node::index shape = m_cache.shape();
			for (size_t i = 0; i < p.size(); ++i)
				p[i] = p[i] % shape[i];
			return m_cache[p];
		}
	private:
		mutable VariableStore::node::variable m_cache;
	};
	class Factory : public FunctionFactory<VariableStore>
	{
		Function* makeInstance() const override
		{
			return new FreezeFunction::Function();
		}
	};
};


inline double atan2(std::array<double, 2> inputs)
{
	return std::atan2(inputs[0], inputs[1]);
}

inline double hypot(std::array<double, 2> inputs)
{
	return std::hypot(inputs[0], inputs[1]);
}

inline double min(std::array<double, 2> inputs)
{
	return std::min(inputs[0], inputs[1]);
}

inline double max(std::array<double, 2> inputs)
{
	return std::max(inputs[0], inputs[1]);
}

template<class Node>
inline void rand1d(typename Node::variable& result, std::array<const Node*, 2> input)
{
	auto input0Shape = input[0]->getShape();
	auto input1Shape = input[1]->getShape();
	result.reshape({ 0,0 });
	if (Node::getSizeFromShape(input0Shape) == 0)
		return;
	if (Node::getSizeFromShape(input1Shape) == 0)
		return;
	size_t size = size_t(input[0]->getValue({ 0,0 }));
	result.reshape({ size, 1 });
	srand((unsigned int)(input[1]->getValue({ 0,0 })));
	for (auto& r : result)
		r = rand() / (double)(RAND_MAX);
}

template<class Node>
inline void rand2d(typename Node::variable& result, std::array<const Node*, 3> input)
{
	auto input0Shape = input[0]->getShape();
	auto input1Shape = input[1]->getShape();
	auto input2Shape = input[2]->getShape();
	result.reshape({ 0,0 });
	if (Node::getSizeFromShape(input0Shape) == 0)
		return;
	if (Node::getSizeFromShape(input1Shape) == 0)
		return;
	if (Node::getSizeFromShape(input2Shape) == 0)
		return;
	size_t size0 = size_t(input[0]->getValue({ 0,0 }));
	size_t size1 = size_t(input[1]->getValue({ 0,0 }));
	result.reshape({ size0, size1 });
	srand((unsigned int)(input[2]->getValue({ 0,0 })));
	for (auto& r : result)
		r = rand() / (double)(RAND_MAX);
}

template<class Node>
void sum(typename Node::variable& result, std::array<const Node*, 1> input)
{
	result.reshape({ 1,1 });
	result[0][0] = 0.0;
	typename Node::index shape = input[0]->getShape();
	for (size_t i = 0; i < shape[0]; ++i)
		for (size_t j = 0; j < shape[1]; ++j)
			result += input[0]->getValue({ i,j });
}

template<class VariableStore>
bool FunctionMultiFactory<VariableStore>::addStandardFunctions()
{
	return set(sU("cos"), std::make_unique<EasyElementwiseFunction1Param<std::cos, VariableStore>::Factory>()) &&
		set(sU("sin"), std::make_unique<EasyElementwiseFunction1Param<std::sin, VariableStore>::Factory>()) &&
		set(sU("tan"), std::make_unique<EasyElementwiseFunction1Param<std::tan, VariableStore>::Factory>()) &&
		set(sU("acos"), std::make_unique<EasyElementwiseFunction1Param<std::acos, VariableStore>::Factory>()) &&
		set(sU("asin"), std::make_unique<EasyElementwiseFunction1Param<std::asin, VariableStore>::Factory>()) &&
		set(sU("atan"), std::make_unique<EasyElementwiseFunction1Param<std::atan, VariableStore>::Factory>()) &&
		set(sU("atan2"), std::make_unique<EasyElementwiseFunction<2, atan2, VariableStore>::Factory>()) &&
		set(sU("cosh"), std::make_unique<EasyElementwiseFunction1Param<std::cosh, VariableStore>::Factory>()) &&
		set(sU("sinh"), std::make_unique<EasyElementwiseFunction1Param<std::sinh, VariableStore>::Factory>()) &&
		set(sU("tanh"), std::make_unique<EasyElementwiseFunction1Param<std::tanh, VariableStore>::Factory>()) &&
		set(sU("acosh"), std::make_unique<EasyElementwiseFunction1Param<std::acosh, VariableStore>::Factory>()) &&
		set(sU("asinh"), std::make_unique<EasyElementwiseFunction1Param<std::asinh, VariableStore>::Factory>()) &&
		set(sU("atanh"), std::make_unique<EasyElementwiseFunction1Param<std::atanh, VariableStore>::Factory>()) &&
		set(sU("exp"), std::make_unique<EasyElementwiseFunction1Param<std::exp, VariableStore>::Factory>()) &&
		set(sU("log"), std::make_unique<EasyElementwiseFunction1Param<std::log, VariableStore>::Factory>()) &&
		set(sU("log10"), std::make_unique<EasyElementwiseFunction1Param<std::log10, VariableStore>::Factory>()) &&
		set(sU("ln"), std::make_unique<EasyElementwiseFunction1Param<std::log, VariableStore>::Factory>()) &&
		set(sU("sqrt"), std::make_unique<EasyElementwiseFunction1Param<std::sqrt, VariableStore>::Factory>()) &&
		set(sU("cbrt"), std::make_unique<EasyElementwiseFunction1Param<std::cbrt, VariableStore>::Factory>()) &&
		set(sU("hypot"), std::make_unique<EasyElementwiseFunction<2, hypot, VariableStore>::Factory>()) &&
		set(sU("erf"), std::make_unique<EasyElementwiseFunction1Param<std::erf, VariableStore>::Factory>()) &&
		set(sU("erfc"), std::make_unique<EasyElementwiseFunction1Param<std::erfc, VariableStore>::Factory>()) &&
		set(sU("tgamma"), std::make_unique<EasyElementwiseFunction1Param<std::tgamma, VariableStore>::Factory>()) &&
		set(sU("lgamma"), std::make_unique<EasyElementwiseFunction1Param<std::lgamma, VariableStore>::Factory>()) &&
		set(sU("ceil"), std::make_unique<EasyElementwiseFunction1Param<std::ceil, VariableStore>::Factory>()) &&
		set(sU("floor"), std::make_unique<EasyElementwiseFunction1Param<std::floor, VariableStore>::Factory>()) &&
		set(sU("round"), std::make_unique<EasyElementwiseFunction1Param<std::round, VariableStore>::Factory>()) &&
		set(sU("abs"), std::make_unique<EasyElementwiseFunction1Param<std::abs, VariableStore>::Factory>()) &&
		set(sU("min"), std::make_unique<EasyElementwiseFunction<2, min, VariableStore>::Factory>()) &&
		set(sU("max"), std::make_unique<EasyElementwiseFunction<2, max, VariableStore>::Factory>()) &&
		set(sU("rand1d"), std::make_unique<CachingFunction<2, VariableStore, rand1d<VariableStore::node>>::Factory>()) &&
		set(sU("rand2d"), std::make_unique<CachingFunction<3, VariableStore, rand2d<VariableStore::node>>::Factory>()) &&
		set(sU("sum"), std::make_unique<CachingFunction<1, VariableStore, sum<VariableStore::node>>::Factory>()) &&
		set(sU("freeze"), std::make_unique<FreezeFunction<VariableStore>::Factory>());
}