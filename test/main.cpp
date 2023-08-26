#include "stdio.h"

#include "math.h"
#include "area.h"

#include "test.h"
void test_math()
{
    printf("F(x, y) = x + y when x = 5, y = 6, result : %d \n", math_add(5,6));
    printf("F(x) = x^2 when x = 5, result : %d \n", math_square(5));
    return;
}

void test_area_timespend()
{
    printf("circle area S = pi * r^2 calculate r = 5, result : %f , time cost : %f nsec\n",
            math_circle_area(5),
            calculate_time_spend_nsec(CIRCLE));
}

void test_annulus_timespend()
{
    printf("annulus area S = pi * (R^2 - r^2) calculate R=8, r = 5, result : %f , time cost : %f nsec\n",
            math_annulus_area(8,5),
            calculate_time_spend_nsec(ANNULUS));
}

void test_ellipse_timespend()
{
    printf("ellipse area S = pi * a * b calculate a=8, b = 5, result : %f , time cost : %f nsec\n",
            math_ellipse_area(8,5),
            calculate_time_spend_nsec(ELLIPSE));
}

int main()
{
    test_math();
    test_area_timespend();
    test_annulus_timespend();
    test_ellipse_timespend();
    return 0;
}
