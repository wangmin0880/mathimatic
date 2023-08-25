#include "stdio.h"

#include "math.h"
#include "area.h"

void test_math()
{
    printf("F(x, y) = x + y when x = 5, y = 6, result : %d \n", math_add(5,6));
    printf("F(x) = x^2 when x = 5, result : %d \n", math_square(5));
    return;
}

void test_area()
{
    printf("circle area S = pi * r^2 when r = 5, result : %f \n", m_circle_area(5));
}

int main()
{
    //test_math();
    test_area();
    return 0;
}
