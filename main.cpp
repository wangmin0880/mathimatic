#include "math.h"
#include "stdio.h"
/*
int add(int a, int b)
{
    return a + b;
}
*/
void testF()
{
    printf("F(x, y) = x + y when x = 5, y = 6, result : %d \n", math_add(5,6));
    printf("F(x) = x^2 when x = 5, result : %d \n", math_square(5));
    return;
}
int main()
{
    /*
    int a = 0;
    int b = 0;
    int c = F(a, b);
    */
    testF();
    return 0;
}
