
#include <ctype.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>

#include "bindings.h"
#include "gcl.h"

#define null ((void*) 0)

/*----------------------------------------------------------------------+
 |      Definitions                                                     |
 +----------------------------------------------------------------------*/

typedef enum {
  endType, openType, closeType, wordType, condType, loopType
} WordType_t;

// Disected word 
struct word {
  char prefix;      // optional
  char *name;       // null in case of a number
  int number;       // length in case of a name
  char operator[3]; // or a conditional
};

/*----------------------------------------------------------------------+
 |      Variables                                                       |
 +----------------------------------------------------------------------*/

char *cursor;
char *compileError;
int blockDepth;

int base, offset, size;
unsigned char page[256];

/*----------------------------------------------------------------------+
 |      Internal functions                                              |
 +----------------------------------------------------------------------*/

/*
 *  Get the next word skipping whitespace and nested comments.
 *  Classify the word according to its structure.
 *  Generate an error message if malformed.
 *  Most work is done here already.
 */
static WordType_t nextWord(struct word *word)
{
  #define hasMore\
    (isprint(*cursor) && !strchr("[{", *cursor))

  #define error(s)\
    ((compileError = (s)), -1)

  memset(word, 0, sizeof *word);

  // Whitespace
  while (1) {
    // Spaces or newlines
    if (strchr(" \n", *cursor) || strncmp("\r\n", cursor, 2)==0) {
      cursor++;
      continue;
    }
    // Comments
    if (*cursor == '{') {
      cursor++;
      int depth = 1;
      do {
        if (*cursor == 0)      return error("Comment without end");
        if (!isprint(*cursor)) return error("Invalid byte");
        if (*cursor == '{')    depth++;
        if (*cursor == '}')    depth--;
        cursor++;
      } while (depth > 0);
      continue;
    }
    break;
  }

  // End of file/line
  if (*cursor == 0) return endType;

  // Blocks
  if (*cursor == '[') return cursor++, openType;
  if (*cursor == ']') return cursor++, closeType;

  // Prefix
  if (strchr("<>\\%", *cursor))
    word->prefix = *cursor++;

  // Word or number
  char *s = strchr("-+", *cursor);
  char sign = s ? *cursor++ : 0;
  if (isdigit(*cursor)) {
    // Decimal number (ignore overflows)
    int n = 0;
    do {
      n = 10 * n + (*cursor - '0');
      cursor++;
    } while (isdigit(*cursor));
    word->number = (sign == '-') ? -n : n;
  } else if (*cursor == '$') {
    // Hexadecimal number (ignore overflows)
    int n = 0;
    s = ++cursor;
    while (1) {
      if (isdigit(*cursor))
        n = 16 * n + (*cursor - '0');
      else if ('a' <= *cursor && *cursor <= 'f')
        n = 16 * n + (*cursor - 'a');
      else if ('A' <= *cursor && *cursor <= 'F')
        n = 16 * n + (*cursor - 'A');
      else
        break;
      cursor++;
    }
    if (cursor == s)
      return error("Missing hex");
    word->number = (sign == '-') ? -n : n;
  } else if (sign)
    return error("Sign without number");
  else if (isalpha(*cursor)) {
    // Name
    word->name = cursor++;
    while (isalnum(*cursor) || *cursor == '_')
      cursor++;
    word->number = cursor - word->name;
  } else
    return error("Invalid word");

  // Conditionals
  if (strncmp("if", word->name, 2)==0 && word->number == 2 &&
      !word->prefix && !sign) {
    int i = 0;
    if (*cursor == '<')        word->operator[i++] = *cursor++;
    if (*cursor == '>')        word->operator[i++] = *cursor++;
    if (*cursor == '=' && i<2) word->operator[i++] = *cursor++;
    if (i > 0 && *cursor++ == '0') {
      if (strncmp(cursor, "loop", 4)==0)
        cursor += 4;
      if (!hasMore)
        return condType;
    }
    return error("Invalid conditional");
  }

  // Operator
  s = strchr("+-&|^=.,:;?!<", *cursor);
  if (s) {
    word->operator[0] = *cursor++;
    if (cursor[-1] == cursor[0] && strchr("+-<", *cursor))
      word->operator[1] = *cursor++; // ++ -- <<
  }

  if (hasMore)
    return error("Invalid word");
  return wordType;
}

static int compileWord(struct word *w)
{
  return 0;
}

static void emit(int byte)
{
  if (offset < size)
    page[offset] = byte;
  offset++;
}

static int emitIf(int cond)
{
  emit(BCC);
  emit(cond);
  return 0;
}

static int emitIfLoop(int cond)
{
  emit(BCC);
  emit(cond);
  return 0;
}

static int compileCond(struct word *w)
{
  if (strcmp(w->operator, "<")==0) return emitIf(GE);
  if (strcmp(w->operator, ">")==0) return emitIf(LE);
  if (strcmp(w->operator, "=")==0) return emitIf(NE);
  if (strcmp(w->operator, "<>")==0) return emitIf(EQ);
  if (strcmp(w->operator, "<=")==0) return emitIf(GT);
  if (strcmp(w->operator, ">=")==0) return emitIf(LT);
  return -1;
}

static int compileLoop(struct word *w)
{
  if (strcmp(w->operator, "<")==0) return emitIfLoop(LT);
  if (strcmp(w->operator, ">")==0) return emitIfLoop(GT);
  if (strcmp(w->operator, "=")==0) return emitIfLoop(EQ);
  if (strcmp(w->operator, "<>")==0) return emitIfLoop(NE);
  if (strcmp(w->operator, "<=")==0) return emitIfLoop(LE);
  if (strcmp(w->operator, ">=")==0) return emitIfLoop(GE);
  return -1;
}

/*----------------------------------------------------------------------+
 |      External functions                                              |
 +----------------------------------------------------------------------*/

void compileBegin(void)
{
  compileError = null;
  blockDepth = 0;

  base = 0x0200;
  offset = 0;
  size = 250;
}

int compileLine(char *line)
{
  cursor = line;
  struct word w;
  while (1) {
    switch (nextWord(&w)) {
    case endType:
      break;
    case openType:
      blockDepth++;
      continue;
    case closeType:
      if (blockDepth <= 0)
        return error("Close block without open");
      blockDepth--;
      continue;
    case wordType:
      if (compileWord(&w))
        return -1;
      continue;
    case condType:
      if (compileCond(&w))
        return -1;
      continue;
    case loopType:
      if (compileLoop(&w))
        return -1;
      continue;
    default:
      return -1;
    }
    break;
  }
  return 0;
}

int compileEnd(void)
{
  if (blockDepth > 0)
    return error("Open block without close");
  return 0;
}

/*----------------------------------------------------------------------+
 |                                                                      |
 +----------------------------------------------------------------------*/

