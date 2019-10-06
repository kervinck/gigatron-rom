if [ ! -d tmp ]; then
	mkdir tmp
fi

#for i in cbmbasic1 cbmbasic2 kbdbasic osi kb9 applesoft microtan aim65 sym1; do
for i in cbmbasic2 gigatron; do

echo $i
ca65 -D $i msbasic.s -o tmp/$i.o -l tmp/$i.lst &&
ld65 -C $i.cfg tmp/$i.o -o tmp/$i.bin -Ln tmp/$i.lbl

done

# Gigatron

od -v -A n -t x1 tmp/gigatron.bin |\
fmt -1 |\
awk -v A=0x280 '{
 if(A%16==0)print""
 if(A%256==0)printf"\n*=$%x\n",A
 printf " #$%02x",$1
 A++}' > include.gcl

ls -l tmp/*.bin tmp/*.lst include.gcl
