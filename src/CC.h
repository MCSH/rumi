#pragma once
#include <string>

class CompileContext;
using CC = CompileContext;
class Source;
class DebugInfo;

enum Keyword: short;
enum Symbol : short;

void graceFulExit(DebugInfo *d, std::string error="");
