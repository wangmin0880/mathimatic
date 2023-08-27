#include "stdio.h"

#include "math.h"
#include "area.h"

#include "test.h"

class Math_test: public Math, public Test
{
  public:
    void test()
    {
      printf("F(x, y) = x + y when x = 5, y = 6, result : %d \n", math_add(5,6));
      printf("F(x) = x^2 when x = 5, result : %d \n", math_square(5));
    }
    int mathimatic_call()
    {
      return 0;
    }
};

class Area_circle_test: public Area, public Test
{
  public:
    void test()
    {
      printf("circle area S = pi * r^2 calculate r = 5, result : %f , time cost : %f nsec\n",
          math_circle_area(5),
          calculate_time_spend_nsec());
    }
    int mathimatic_call()
    {
       return math_circle_area(5);
    }
};

class Area_annulus_test: public Area, public Test
{
  public:
    void test()
    {
      printf("annulus area S = pi * (R^2 - r^2) calculate R=8, r = 5, result : %f , time cost : %f nsec\n",
          math_annulus_area(8,5),
          calculate_time_spend_nsec());
    }
    int mathimatic_call()
    {
      return math_annulus_area(8,5);
    }
};

class Area_ellipse_test: public Area, public Test
{
  public:
  void test()
  {
    printf("ellipse area S = pi * a * b calculate a=8, b = 5, result : %f , time cost : %f nsec\n",
        math_ellipse_area(8,5),
        calculate_time_spend_nsec());
  }
  int mathimatic_call()
  {
    return math_ellipse_area(8,5);
  }
};

int main()
{
  Math_test math;
  math.test();

  Area_circle_test area_circle;
  area_circle.test();

  Area_annulus_test area_annulus;
  area_annulus.test();

  Area_ellipse_test area_ellipse;
  area_ellipse.test();

  return 0;
}
