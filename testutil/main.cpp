#include "stdio.h"

/*1. target lib */
#include "math.h"
#include "area.h"

/*2. util & test*/
#include "test.h"
#include "ja_list.h"

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
    printf("square root y = sqrt x calculate x = 16, result : %f , time cost : %f nsec\n",
        math_square_root(16),
        calculate_time_spend_nsec());
  }
  int mathimatic_call()
  {
    return math_square_root(16);
  }
};

int main()
{
  ja_list_head ja_head;
  printf("hello, util!\n");
  return 0;
}
