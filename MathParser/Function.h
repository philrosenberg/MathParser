#pragma once
#include"FunctionBase.h"
#include"Node.h"
#include"Parse.h"

//this is a function which accepts N expressions, each separated by a comma. Each expression will be converted to a node
template<size_t N>
class CommaSeparatedInputsFunction : public Function
{
public:
	virtual bool setInputs(const sci::string inputs, sci::string& badExpression, const VariableStore& variables, const FunctionMultiFactory& functions)
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
			m_ownedNodes[i].reset(new Node());
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
	std::array<std::unique_ptr<Node>, N>m_ownedNodes;
	sci::GridData<const Node*, 1>m_nodes;
};

template<size_t N>
class ElementwiseCommaSeparatedInputsFunction : public CommaSeparatedInputsFunction<N>
{
public:
	virtual index getShapeAndCacheIfNeeded() const override
	{
		index result;
		for (auto& r : result)
			r = 0;
		for (auto n : CommaSeparatedInputsFunction<N>::m_nodes)
		{
			index thisShape = n->getShape();
			for (size_t i = 0; i < nDims; ++i)
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
template<size_t N>
using CacheFuncNParam = void(*)(variable&, std::array<const Node*, N>);


//this class creates a function that will apply a scalar function accepting one parameter
//to every element of an input.
template<ElementwiseFunc1Param FUNC>
class EasyElementwiseFunction1Param
{
public:
	class Function : public ElementwiseCommaSeparatedInputsFunction<1>
	{
	public:
		double getValue(const index& position) const override
		{
			return FUNC(CommaSeparatedInputsFunction<1>::m_nodes[0]->getValue(position));
		}
	private:
	};
	class Factory : public FunctionFactory
	{
		Function* makeInstance() const override
		{
			return new EasyElementwiseFunction1Param<FUNC>::Function();
		}
	};
};

//this class creates a function that will apply a scalar function accepting N_PARAMS
//parameters inside a std::array to every element of a set of imputs to give the elements
//of an output.
template<size_t N_PARAMS, ElementwiseFuncNParam<N_PARAMS> FUNC>
class EasyElementwiseFunction
{
public:
	class Function : public ElementwiseCommaSeparatedInputsFunction<N_PARAMS>
	{
	public:
		double getValue(const index& position) const override
		{
			std::array<double, N_PARAMS> inputs;
			for (size_t i = 0; i < N_PARAMS; ++i)
				inputs[i] = CommaSeparatedInputsFunction<N_PARAMS>::m_nodes[i]->getValue(position);
			return FUNC(inputs);
		}
	private:

	};
	class Factory : public FunctionFactory
	{
		Function* makeInstance() const override
		{
			return new EasyElementwiseFunction<N_PARAMS, FUNC>::Function();
		}
	};
};

template<size_t N_PARAMS, CacheFuncNParam<N_PARAMS> FUNC>
class CachingFunction
{
public:
	class Function : public CommaSeparatedInputsFunction<N_PARAMS>
	{
	public:
		index getShapeAndCacheIfNeeded() const override
		{
			std::array<const Node*, N_PARAMS> inputs;
			for (size_t i = 0; i < N_PARAMS; ++i)
				inputs[i] = CommaSeparatedInputsFunction<N_PARAMS>::m_nodes[i];
			FUNC(m_cache, inputs);
			return m_cache.shape();
		}
		double getValue(const index& position) const override
		{
			index p = position;
			index shape = m_cache.shape();
			for (size_t i = 0; i < p.size(); ++i)
				p[i] = p[i] % shape[i];
			return m_cache [p];
		}
	protected:
		mutable variable m_cache;
	};
	class Factory : public FunctionFactory
	{
		Function* makeInstance() const override
		{
			return new CachingFunction<N_PARAMS, FUNC>::Function();
		}
	};
};

class FreezeFunction
{
public:
	class Function : public CommaSeparatedInputsFunction<1>
	{
	public:
		bool setInputs(const sci::string inputs, sci::string& badExpression, const VariableStore& variables, const FunctionMultiFactory& functions) override
		{
			bool result = CommaSeparatedInputsFunction<1>::setInputs(inputs, badExpression, variables, functions);
			if(result)
				m_cache = CommaSeparatedInputsFunction<1>::m_nodes[0]->calculate();
			return result;
		}
		index getShapeAndCacheIfNeeded() const override
		{
			return m_cache.shape();
		}
		double getValue(const index& position) const override
		{
			index p = position;
			index shape = m_cache.shape();
			for (size_t i = 0; i < p.size(); ++i)
				p[i] = p[i] % shape[i];
			return m_cache[p];
		}
	private:
		mutable variable m_cache;
	};
	class Factory : public FunctionFactory
	{
		Function* makeInstance() const override
		{
			return new FreezeFunction::Function();
		}
	};
};