
SHELL=/bin/sh
GLCC=../../build/glcc
ROMS=v5a v6 dev7


all: FORCE
	while read n opt ; do\
		for rom in ${ROMS} ; do\
			cmd="${GLCC} -rom=$$rom $$opt hello.c -o hello-$$n-$$rom.gt1" ;\
			${SHELL} -c "$$cmd" && len=`wc -c < "hello-$$n-$$rom.gt1"` || len=err ;\
			echo "$$cmd" "-->" "$$len" ;\
		done || break ;\
	done < options.txt

clean: FORCE
	rm *.gt1

FORCE:
