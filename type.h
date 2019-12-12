#pragma once

class Type{
public:
  virtual ~Type(){
  }
};

class NoType: public Type{
};

class IntType: public Type{
};
