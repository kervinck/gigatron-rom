SHELL=/bin/sh
GIGAROM=../..
COMPILEGCL=${GIGAROM}/Core/compilegcl.py

PGMS=CardBoot.gt1

all:${PGMS}

CardBoot.gt1: CardBoot.gcl
	${COMPILEGCL} -s ${GIGAROM}/interface.json CardBoot.gcl

clean: FORCE
  ifdef OS   ## Windows
	-del ${PGMS}
  else
	-rm 2>/dev/null ${PGMS}
  endif


FORCE:

.PHONY: FORCE

