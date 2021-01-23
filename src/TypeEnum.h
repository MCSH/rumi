#pragma once
#include <string>

enum TypeEnum{
  t_int,
  t_f32,
  t_f64,
  t_any,
  t_u8,
  t_u16,
  t_u32,
  t_u64,
  t_s8,
  t_s16,
  t_s32,
  t_s64,
  t_string,
  t_unit,
  t_void,
  t_bool,
};

std::string typeEnumToString(TypeEnum key);

TypeEnum typeEnumFromString(std::string key);
