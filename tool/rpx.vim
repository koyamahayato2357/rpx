syntax match Comment /;.*/
syntax match Delimiter /,/
syntax match Identifier /^:/
syntax match Special /^:./hs=s+1
syntax match Operator "\V\[-+/*=^><%]"
syntax match Constant "\\."
syntax match Identifier "\$."
syntax match Identifier "&."
syntax match Function "!."
syntax match Number "\d"
syntax match Number "\."
syntax match Function /\d*s\d*/
syntax keyword Function s c t as ac at hs hc ht
syntax keyword Function l2 lc le m i
