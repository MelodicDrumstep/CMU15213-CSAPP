#include <stdio.h>

int conditional(int x, int y, int z) {
  x = !!x;
  int flag = (~(x)) + 1;
  return (flag & y) | (~flag & z);
}

int equal(int x, int y)
{
  return !((x) ^ (y));
}

int negate(int x)
{
    return ~x + 1;
}

int isLessOrEqual(int x, int y) {
  int sign_x = x & (1 << 31);
  int sign_y = y & (1 << 31);
  printf("sign of x is 0x%x\n", sign_x);
  printf("sign of y is 0x%x\n", sign_y);
  printf("equal is 0x%x\n", equal(x, y));
  printf("contional is 0x%x\n", conditional(sign_x ^ sign_y, sign_x >> 31, (!!((x + negate(y)) & (1 << 31))) ));
  return equal(x, y) | conditional(sign_x ^ sign_y, sign_x >> 31, (!!((x + negate(y)) & (1 << 31))) );
}

int main()
{
    isLessOrEqual(0x80000000, 0x7fffffff);
    return 0;
}