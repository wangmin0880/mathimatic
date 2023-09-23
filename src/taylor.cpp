/*
* We will focus on the taylor mathimatic implementation this part
* 1. sin(x)
*                                                    ---wangmin0880@163.com
*                                                    ---phone:+8618611106826
*/

#include "taylor.h"

/* x belong to [0,360] */
/*
 * sin : sin value
 * term : a_k
 * degree : input x is degree value  [0, 360]
 * arc : [0 2pi]
 */
#define ja_abs(x) (x > 0 ? x : -x)

double ja_sin(double x)
{
  /*
  * pi = 3.141592653589793238
  */
  double pi = 3.1415926536, delta_min = 0.0000001;

  double sin = 0.0;
  double term = 1.0;

  double degree = x;
  double x_arc = degree * pi / 180;
  int k = 1;

  term = x_arc;
  while(ja_abs(term) > delta_min){
    sin += term;

    k += 2;
    term = (-1) * term * x_arc * x_arc / (k * (k - 1)) ;
  }

  return sin;
}

double Taylor::math_sin(double x)
{
  return ja_sin(x);
}
