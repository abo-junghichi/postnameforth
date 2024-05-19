#!/bin/sh
for f in forth
do
	gcc -Os -Wall -pedantic -Wextra -Wstrict-aliasing=1 \
		-static -nostdlib -fno-builtin -fno-pie -S \
		-DSHEBANG $f.c -o /dev/stdout | \
	sed 's/.rodata/.text/g' >$f.s
done
sh elfheader.sh 0 0 0 0 > elfheader.s
assemble(){
	gcc -static -nostdlib -fno-builtin -fno-pie \
		elfheader.s forth.s -o forth.out
}
assemble
TEXT=$(readelf -l forth.out | grep 'LOAD .* R E')
BSS=$(readelf -l forth.out | grep 'LOAD .* RW ')
ph_cut(){
        cut -f$1 -dx | cut -f1 -d\ 
}
TEXT_ADDR=$(echo $TEXT | ph_cut 3)
TEXT_SIZE=$(echo $TEXT | ph_cut 6)
BSS_ADDR=$(echo $BSS | ph_cut 3)
BSS_SIZE=$(echo $BSS | ph_cut 6)
sh elfheader.sh $TEXT_ADDR $TEXT_SIZE $BSS_ADDR $BSS_SIZE > elfheader.s
assemble
LENGTH=$(echo $TEXT_SIZE | tr 'abcdef' 'ABCDEF')
SIZE=$(echo "ibase=16 ; $LENGTH" | bc )
dd if=forth.out of=forth.out.trunc bs=4096 skip=1
truncate --size=$SIZE forth.out.trunc
chmod +x forth.out.trunc
