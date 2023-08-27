/*
* We will focus on the basic mathimatic implementation this part
*                                                    ---wangmin0880@163.com
*                                                    ---phone:+8618611106826
*/

#include "math.h"

/* y = a + b */
#define m_add(x, y) (x + y)
/* y = x^2 */
#define m_square(x) x*x

/* y^2 = x^3 + a*x +b
#define m_ecc(x, a, b) sqrt(x*x*x + a*x + b)
*/

int Math::math_add(int x, int y)
{
  return m_add(x,y);
}

double Math::math_add(double x, double y)
{
  return m_add(x,y);
}

int Math::math_square(int x)
{
  return m_square(x);
}

double Math::math_square(double x)
{
  return m_square(x);
}

double Math::math_ecc(double x, double a, double b)
{
  double y_square = x*x*x + a*x +b;
  double y = math_square_root(y_square);
  return y;
}

double Math::math_square_root(double x)
{
  if(x == 0)
  {
    return 0;
  }
  double xk = 1, xk1 = 0;
  double err = 1e-6;

  while(math_abs(xk - xk1) > err)
  {
    xk1 = xk;
    xk = ( xk + x/xk ) / 2; //niuton
  }
  return xk;
}

double Math::math_abs(double x)
{
  return (x > 0) ? x : -x;
}
