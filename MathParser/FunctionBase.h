#pragma once

#include"types.h"
#include<array>

template<int nDims>
class VariableStore;
template<class VariableStore>
class FunctionMultiFactory;
template<int nDims>
class Node;

//A function is written in C++ and is very flexible
//It has 3 virtual methods
//makeInstance will be called to create an instance
//of your function. Return it cast as a Function*.
//
//setInputs will be called when the calling Node is
//being parsed. The function could parse the strings
//into Nodes, or could choose not to. Any nodes
//it creates are owned by the function and should
//be destroyed when the function object is destroyed.
//If an input cannot be parsed, put it in badExpression
// and return false. If there is some other error
//put it in bad expression and return false. If all is
//well, return true. You may want to grab variables
//from the variableStore.
//Get shape will be called after setNodes as part of
//parsing. The Function must return the shape of the
//output, which can be based on the shape and content
//of the nodes passed in earlier.
//CacheIfNeeded will be called just before a burst of
//getValue calls. This allows the Function to do
//calculations that are not element by element, e.g.
//a fourier transform. The whole input can be processed
//in one go, cached and then regurgitated as requested.
//Don't do this unless you need to. It is usually slower
//due to the increased iteration over memory.

template<class VariableStore>
class Function
{
public:
	using index = typename VariableStore::node::index;
	virtual bool setInputs(const sci::string inputs, sci::string& badExpression, sci::string &error, const VariableStore& variables, const FunctionMultiFactory<VariableStore>& functions) = 0;
	virtual index getShapeAndCacheIfNeeded(sci::string& error) const = 0;
	virtual double getValue(const index& position) const = 0;
	virtual bool contains(const typename VariableStore::node* node) const = 0;
	virtual ~Function()
	{}
};

//a FunctionFactory makes instances of your
//Function classes. Make a matching one for
//each function you write. The Function Factory
//must be copyable.
template<class VariableStore>
class FunctionFactory
{
public:
	virtual Function<VariableStore>* makeInstance() const = 0;
};

//A FunctionMultifactory holds unique pointers to each FunctionFactory with the key being the name
// of the functon. You use it to grab the FunctionFactory to make a function instance from the function
//name. 
template<class VariableStore>
class FunctionMultiFactory
{
public:
	bool set(const sci::string& name, std::unique_ptr<FunctionFactory<VariableStore>> functionFactory)
	{
		m_factories[name] = std::move(functionFactory);
		return true;
	}
	std::unique_ptr<Function<VariableStore>> getFunction(sci::string functionName) const
	{
		auto factoryIter = m_factories.find(functionName);
		if (factoryIter == m_factories.end())
			return nullptr;
		else
			return std::unique_ptr<Function<VariableStore>>(factoryIter->second->makeInstance());
		return nullptr;
	}
	bool addStandardFunctions();
private:
	std::unordered_map<sci::string, std::unique_ptr<FunctionFactory<VariableStore>>> m_factories;
};
