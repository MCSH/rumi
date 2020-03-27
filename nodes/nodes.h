#pragma once

#include "Expression.h"
#include "Node.h"
#include "Statement.h"
#include "Type.h"

#include "types/AnyType.h"
#include "types/ArrayType.h"
#include "types/FloatType.h"
#include "types/FunctionType.h"
#include "types/IntType.h"
#include "types/InterfaceType.h"
#include "types/PointerType.h"
#include "types/StringType.h"
#include "types/StructType.h"
#include "types/VoidType.h"

#include "expressions/IntValue.h"
#include "expressions/StringValue.h"
#include "expressions/VariableExpr.h"
#include "expressions/FunctionCallExpr.h"
#include "expressions/BinaryOperation.h"
#include "expressions/MemberExpr.h"
#include "expressions/ArrayExpr.h"
#include "expressions/CastExpr.h"
#include "expressions/PointerExpr.h"
#include "expressions/PointerAccessExpr.h"
#include "expressions/SizeofExpr.h"
#include "expressions/TypeNode.h"
#include "expressions/MethodCall.h"

#include "statements/ArgDecl.h"
#include "statements/CodeBlock.h"
#include "statements/FunctionDefine.h"
#include "statements/FunctionSignature.h"
#include "statements/ReturnStatement.h"
#include "statements/VariableDecl.h"
#include "statements/VariableAssign.h"
#include "statements/IfStatement.h"
#include "statements/WhileStatement.h"
#include "statements/DeferStatement.h"
#include "statements/StructStatement.h"
#include "statements/ImportStatement.h"
#include "statements/CompileStatement.h"
#include "statements/MemberStatement.h"
#include "statements/InterfaceStatement.h"
