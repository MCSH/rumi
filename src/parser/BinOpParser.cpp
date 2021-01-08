#include "BinOpParser.h"
#include "Symbols.h"
#include "../Source.h"

ParseResult BinOpParser::scheme(CC *cc, Source *s, int pos){
  ParseState *ps = s->resolveState(pos);
  if(ps->binOp) return ps->binToken; // recursive
  ps->binOp = true;

  ps->binToken = innerscheme(cc, s, pos).token;
  return ps->binToken;
}

ParseResult BinOpParser::innerscheme(CC *cc, Source *s, int pos){
  // exp op exp
  auto v1 = vp.parse(cc, s, pos);

  // op = +?
  auto tmp = v1 >> addp;
  if(tmp) return tmp >> vp;

  // op = -?
  tmp = v1 >> minusp;
  if(tmp) return tmp >> vp;

  // op = *?
  tmp = v1 >> multp;
  if(tmp) return tmp >> vp;

  // op = /?
  tmp = v1 >> divp;
  if(tmp) return tmp >> vp;

  // op = %?
  tmp = v1 >> remp;
  if(tmp) return tmp >> vp;

  return 0;
}


BinOpParser::BinOpParser()
  : addp(s_plus)
  , minusp(s_minus)
  , multp(s_mult)
  , divp(s_div)
  , remp(s_rem)
{}
