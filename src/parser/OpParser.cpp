#include "OpParser.h"
#include "Symbols.h"

ParseResult OpParser::scheme(CC *cc, Source *s, int pos){
  auto tmp = addp.parse(cc, s, pos);
  if(tmp) return tmp;

  tmp = minusp.parse(cc, s, pos);
  if(tmp) return tmp;

  tmp = multp.parse(cc, s, pos);
  if(tmp) return tmp;

  tmp = divp.parse(cc, s, pos);
  if(tmp) return tmp;

  tmp = remp.parse(cc, s, pos);
  if(tmp) return tmp;

  tmp = eqp.parse(cc, s, pos);
  if(tmp) return tmp;

  tmp = neqp.parse(cc, s, pos);
  if(tmp) return tmp;

  tmp = gtp.parse(cc, s, pos);
  if(tmp) return tmp;

  tmp = gtep.parse(cc, s, pos);
  if(tmp) return tmp;

  tmp = ltp.parse(cc, s, pos);
  if(tmp) return tmp;

  tmp = ltep.parse(cc, s, pos);
  if(tmp) return tmp;

  tmp = andp.parse(cc, s, pos);
  if(tmp) return tmp;

  tmp = carP.parse(cc, s, pos);
  if(tmp) return tmp;

  tmp = pipP.parse(cc, s, pos);
  if(tmp) return tmp;

  tmp = ampP.parse(cc, s, pos);
  if(tmp) return tmp;

  tmp = lshiftP.parse(cc, s, pos);
  if(tmp) return tmp;

  tmp = rshiftP.parse(cc, s, pos);
  if(tmp) return tmp;

  tmp = orp.parse(cc, s, pos);
  return tmp;
}

OpParser::OpParser()
  : addp(s_plus)
  , minusp(s_minus)
  , multp(s_mult)
  , divp(s_div)
  , remp(s_rem)
  , eqp(s_eqeq)
  , neqp(s_neq)
  , gtp(s_gt)
  , gtep(s_gte)
  , ltp(s_lt)
  , ltep(s_lte)
  , andp(s_andand)
  , orp(s_pipepipe)
  , carP(s_caret)
  , pipP(s_pipe)
  , ampP(s_and)
  , lshiftP(s_ltlt)
  , rshiftP(s_gtgt)
{}
