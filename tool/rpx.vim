syntax iskeyword a-z

highlight link RpxCmdStart Keyword
syntax match RpxCmdStart /^:/ contained

highlight link RpxTglCmd Special
highlight link RpxTglItem Identifier
syntax match RpxTglCmd /t/ contained
syntax match RpxTgl /^:t/ contained contains=RpxCmdStart,RpxTglCmd
syntax match RpxTglItem /\l/ contained
syntax match RpxToggle /^:t\l/ contains=RpxTgl,RpxTglItem

highlight link RpxStgCmd Special
highlight link RpxStgItem Identifier
syntax match RpxStgCmd /s/ contained
syntax match RpxStg /^:s/ contained contains=RpxCmdStart,RpxStgCmd
syntax match RpxStgItem /\l/ contained
syntax match RpxSetting /^:s\l\l/ contains=RpxStg,RpxStgItem,RpxStgItem

highlight link RpxFormulaFollowsCmd Special
syntax match RpxFormulaFollowsCmd /[op]/ contained
syntax match RpxFormulaFollows /^:[op]/ contains=RpxCmdStart,RpxFormulaFollowsCmd

syntax match Comment /;.*/
syntax match Delimiter /,/
syntax match Operator "\V\[-+/*=^><%]"
syntax match Constant "\\."
syntax match Identifier "\$."
syntax match Identifier "&."
syntax match Function "!"
syntax match SpecialChar "@."
syntax match Number "\d"
syntax match Number "\."
syntax match Keyword "s"
syntax match Keyword "c"
syntax match Keyword "t"
syntax match Keyword "as"
syntax match Keyword "as"
syntax match Keyword "as"
syntax match Keyword "hs"
syntax match Keyword "hc"
syntax match Keyword "ht"
syntax match Keyword "l2"
syntax match Keyword "lc"
syntax match Keyword "le"
syntax match Keyword "m"
syntax match Keyword "ig"
syntax match Keyword "il"
syntax match Keyword "ic"
syntax match Keyword "ip"
syntax match Keyword "A"
syntax match Keyword "C"
syntax match Keyword "F"
syntax match Keyword "R"
syntax match Keyword "d"
syntax match Keyword "r"
syntax match Keyword "L"
