#pragma once
#include "Node.h"

class Statement: public Node{
public:
  virtual void codegen(Context *cc)=0;
};
