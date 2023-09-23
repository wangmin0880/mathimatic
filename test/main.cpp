#include "stdio.h"

#include "math.h"
#include "area.h"
#include "taylor.h"

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

class Math_ecc_test: public Math, public Test
{
  public:
  void test()
  {
    printf("ecc y^2 = x^3 + a*x + b calculate x = 8, a = 5, b= 5, result : %f , time cost : %f nsec\n",
        math_ecc(8,5,5),
        calculate_time_spend_nsec());
  }
  int mathimatic_call()
  {
    return math_ecc(8,5,5);
  }
};

class Math_square_root_test: public Math, public Test
{
  public:
  void test()
  {
    printf("square root y = sqrt x calculate x = 2, result : %f , time cost : %f nsec\n",
        math_square_root(2),
        calculate_time_spend_nsec());
  }
  int mathimatic_call()
  {
    return math_square_root(2);
  }
};

class Taylor_sin_test: public Taylor, public Test
{
  public:
  void test()
  {
    printf("Taylor sin y = sin(x) calculate x = 45, result : %f , time cost : %f nsec\n",
        math_sin(45),
        calculate_time_spend_nsec());
  }
  void full_test()
  {
    double dDegree = 30;
    for(int i = 0; i<= 360; i++ ){
      dDegree = i;
      printf("sin(%f) = %.6lf \n", dDegree, ja_sin(dDegree));
    }
    printf("Taylor sin y = sin(x) calculate x from 0 to 360, time cost : %f nsec\n",
        calculate_time_spend_nsec());
  }
  int mathimatic_call()
  {
    double dDegree = 30;
    for(int i = 0; i<= 360; i++ ){
      dDegree = i;
      ja_sin(dDegree);
      //printf("sin(%f) = %.6lf \n", dDegree, ja_sin(dDegree));
    }
    return 0;
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

  Math_ecc_test math_ecc;
  math_ecc.test();

  Math_square_root_test square_root;
  square_root.test();

  Taylor_sin_test sin;
  sin.test();
  sin.full_test();

  return 0;
}
