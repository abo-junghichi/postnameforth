[
 postpone ]
 0x c3 literal c, ( x86 "ret" )
 create
]
0x c3 c,
create ]= immediate
( [ body... ]= new_word )

[ postpone 0x c, ]= ,c0x

,c0x b8 ,c0x 01 ,c0x  00 ,c0x 00 ,c0x 00 ( mov $0x1,%eax )
,c0x 31 ,c0x db ( xor %ebx,%ebx )
,c0x cd ,c0x 80 ( int $0x80 )
]= bye

,c0x 03 ,c0x 1e ( add (%esi),%ebx )
,c0x 8d ,c0x 76 ,c0x 04 ( lea 0x4(%esi),%esi )
]= add

,c0x 8b ,c0x 06 ( mov (%esi),%eax )
,c0x 8d ,c0x 76 ,c0x 04 ( lea 0x4(%esi),%esi )
,c0x 29 ,c0x d8 ( sub %ebx,%eax )
,c0x 89 ,c0x c3 ( mov %eax,%ebx )
]= sub

[ postpone 0x postpone literal ]= lit0x immediate

[ lit0x 6 allot ]= codefield
[ lit0x 6 add ]= >body
[ postpone ' postpone literal postpone does ]= does> immediate
[ [ postpone get_body ]= [action

[action ]= variable_code
[ codefield align does> variable_code lit0x 4 allot create ]= variable

variable var

var @ .
.
0x beef var !
.
' var >body @ .
.
0x cafe ' var >body !
.
var @ .
.

variable counter_tmp
[action counter_tmp ! counter_tmp @ @ lit0x 1 add counter_tmp @ !
  counter_tmp @ @ . 
]= counter_code
[ codefield align does> counter_code lit0x 4 allot create ]= counter

counter co

co . co . co . co .
0x 0 0x 3 sub ' co >body !
co . co . co . co . co . co .

bye
