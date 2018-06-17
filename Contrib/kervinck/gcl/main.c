
#include <stdio.h>

#include "gcl.h"

int main(void)
{
  compileBegin();

  while (1) {
    size_t len;
    char *line = fgetln(stdin, &len);
    if (!line)
      break;
    if (line[len-1] != '\n') {
      fprintf(stderr, "Incomplete line\n");
      break;
    }
    if (compileLine(line)) {
      fprintf(stderr, "Compile error\n");
      break;
    }
  }

  if (compileEnd())
    fprintf(stderr, "Compile error\n");

  if (compileError)
    fprintf(stderr, "Error: %s\n", compileError);

  return 0;
}

/*
         V=
        <V.
        \V.
        \V
         V.

        \buttonA

        if (scan("{")) { }
        if (scan("}")) { }
        if (scan("[")) { }
        if (scan("]")) { }
        if (scan("I")) { }
        if (scan("I&")) { }
        if (scan("I|")) { }
        if (scan("I^")) { }
        if (scan("I+")) { }
        if (scan("I-")) { }
        if (scan("I.")) { }
        if (scan("I!")) { }
        if (scan("V")) { }
        if (scan("V&")) { }
        if (scan("V|")) { }
        if (scan("V^")) { }
        if (scan("V+")) { }
        if (scan("V-")) { }
        if (scan("V=")) { }
        if (scan("V!")) { }
        if (scan("V.")) { }
        if (scan("V,")) { }
        if (scan("<V++")) { }
        if (scan("<V.")) { }
        if (scan("<V,")) { }
        if (scan(">V++")) { }
        if (scan(">V.")) { }
        if (scan(">V,")) { }
        if (scan("if=0")) { }
        if (scan("if>0")) { }
        if (scan("if<0")) { }
        if (scan("if>=0")) { }
        if (scan("if<=0")) { }
        if (scan("if<>0")) { }
        if (scan("if=0loop")) { }
        if (scan("if>0loop")) { }
        if (scan("if<0loop")) { }
        if (scan("if>=0loop")) { }
        if (scan("if<=0loop")) { }
        if (scan("if<>0loop")) { }
        if (scan("do")) { }
        if (scan("loop")) { }
        if (scan("push")) { }
        if (scan("def")) { }
        if (scan("peek")) { }
        if (scan("push")) { }
        if (scan("pop")) { }
        if (scan("call")) { }
        if (scan("ret")) { }
        if (scan("else")) { }
        if (scan("gcl1")) { }
*/
