#pragma once

#include "Context.h"

Context *codegen(std::vector<Statement *> *statements,
                        std::string outfile, bool print, bool ofile=true);

