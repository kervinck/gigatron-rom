Attempting to bootstrap a standard C library

Known issues
------------

``
puts:
  XXX Linker crashes with 'const char *ss'
  //const char *ss = (const char *)s;
  const byte *ss = (const byte *)s;

LCC:
- Test for overflow/underfow with relational operators
- 'long' is 2 bytes instead of 4. Lets see how far we get with this

``
