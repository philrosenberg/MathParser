#pragma once
#include"FunctionBase.h"
#include"VariableStore.h"

bool parseLine(const sci::string& line, bool skipOutputCalculation, sci::string& badLine, VariableStore& variables, const FunctionMultiFactory& functions, sci::string& output, bool &isAssignment);
bool parseExpression(const sci::string& line, sci::string& badLine, const VariableStore& variables, const FunctionMultiFactory& functions, Node& output);
std::string basicAscii(const sci::string& str);
sci::string basicUnicode(const std::string& str);
void beginConsoleLoop(VariableStore& variables, FunctionMultiFactory& functionFactories);