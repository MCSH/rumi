if exists("b:current_syntax")
  finish
endif

syn keyword rumiKeyword return import enum struct interface sizeof malloc
syn keyword rumiConditional if else while
syn keyword rumiType int s8 s16 s32 s64 u8 u16 u32 u64 bool string f32 f64 unit any void
syn keyword rumiConstant true false
syn match rumiString '\v"([^"]|\"){-}[^\\]"|""'
syn match rumiNumber '\v<\d+(\.\d+)?'
syn match rumiComment '\v//.*$'
syn match rumiDirective '\v\@\w+'
syn region rumiComment start=!/\*! end=!\*/! fold
syn region rumiBlock start="{" end="}" transparent fold

hi default link rumiKeyword Keyword
hi default link rumiConditional Conditional
hi default link rumiType Type
hi default link rumiConstant Constant
hi default link rumiString String
hi default link rumiNumber Number
hi default link rumiComment Comment
hi default link rumiDirective PreProc

let b:current_syntax = "rumi"

