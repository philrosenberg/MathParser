#pragma once

#include"types.h"
#include<array>

class VariableStore;
class FunctionMultiFactory;
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
class Function
{
public:
	virtual bool setInputs(const sci::string inputs, sci::string& badExpression, const VariableStore& variables, const FunctionMultiFactory& functions) = 0;
	virtual index getShapeAndCacheIfNeeded() const = 0;
	virtual double getValue(const index& position) const = 0;
	virtual ~Function()
	{}
};

//a FunctionFactory makes instances of your
//Function classes. Make a matching one for
//each function you write. The Function Factory
//must be copyable.
class FunctionFactory
{
public:
	virtual Function* makeInstance() const = 0;
};

//A FunctionMultifactory holds unique pointers to each FunctionFactory with the key being the name
// of the functon. You use it to grab the FunctionFactory to make a function instance from the function
//name. 
class FunctionMultiFactory
{
public:
	bool set(const sci::string& name, std::unique_ptr<FunctionFactory> functionFactory)
	{
		m_factories[name] = std::move(functionFactory);
		return true;
	}
	std::unique_ptr<Function> getFunction(sci::string functionName) const
	{
		auto factoryIter = m_factories.find(functionName);
		if (factoryIter == m_factories.end())
			return nullptr;
		else
			return std::unique_ptr<Function>(factoryIter->second->makeInstance());
		return nullptr;
	}
	bool addStandardFunctions();
private:
	std::unordered_map<sci::string, std::unique_ptr<FunctionFactory>> m_factories;
};
