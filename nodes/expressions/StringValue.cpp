#include "StringValue.h"
#include <sstream>
#include "../types/StringType.h"
#include "../../Context.h"

StringValue::StringValue(std::string *v){
    std::stringstream ss{""};

    for (int i = 1; i < v->length() - 1; i++) {
      if (v->at(i) == '\\') {
        switch (v->at(i + 1)) {
        case 'n':
          ss << '\n';
          i++;
          break;
        case '"':
          ss << '\"';
          i++;
          break;
        default:
          ss << '\\';
        }
      } else {
        ss << v->at(i);
      }
    }

    this->val = new std::string(ss.str());
    delete v;
  }

StringValue::~StringValue(){
    delete val;
  }

llvm::Value *StringValue::exprGen(Context *cc){
  return cc->builder->CreateGlobalStringPtr(*this->val);
}

llvm::Value *StringValue::getAlloca(Context *cc){}

void StringValue::compile(Context *cc){
  resolveType(cc);
  // TODO
}

Type *StringValue::resolveType(Context *cc){
  if(exprType)
    return exprType;
  exprType = new StringType();
  return exprType;
}

