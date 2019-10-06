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
ls -l tmp/*.bin tmp/*.lst

