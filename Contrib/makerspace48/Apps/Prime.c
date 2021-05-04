#include<stdio.h>
int main()
{
  int n, i = 3, count, c;
while(1)
{
  printf("How many prime numbers?\n");
  scanf("%d",&n);
  if ( n >= 1 )
  {
    printf("First %d prime numbers:\n",n);
    printf("2 ");
  }
  for ( count = 2 ; count <= n ;  )
    {
    for ( c = 2 ; c <= i - 1 ; c++ )
    {
      if ( i%c == 0 ) break;
    }
    if ( c == i )
    {
      printf("%d ", i);
      count++;
    }
    i++;
  }
printf("\n");
}
  return 0;
}

