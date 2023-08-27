#include "math.h"

/* y = a + b */
#define m_add(x, y) (x + y)
/* y = x^2 */
#define m_square(x) x*x

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
