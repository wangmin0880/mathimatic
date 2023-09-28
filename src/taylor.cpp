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

/* x belong to [0,360] */
/*
 * cos : cos value
 * term : a_k
 * degree : input x is degree value  [0, 360]
 * arc : [0 2pi]
 */
double ja_cos(double x)
{
  /*
  * pi = 3.141592653589793238
  */
  double pi = 3.1415926536, delta_min = 0.0000001;

  double cos = 0.0;
  double term = 1.0;

  double degree = x;
  double x_arc = degree * pi / 180;
  int k = 1;

  while(ja_abs(term) > delta_min){
    cos += term;

    k += 2;
    term = (-1) * term * x_arc * x_arc / ((k-1) * (k - 2)) ;
  }

  return cos;

}

double Taylor::math_cos(double x)
{
  return ja_cos(x);
}

/* x belong to [0,360] */
/*
 * e : e value
 * term : a_k
 * x : input x is limited,  x > 0
 */
#define ja_abs(x) (x > 0 ? x : -x)

double ja_e(double x)
{
  double delta_min = 0.0000001;

  double e = 0.0;
  double term = 1.0;

  int k = 1;

  while(ja_abs(term) > delta_min){
    e += term;

    k += 1;
    term = term * x / (k-1);
  }

  return e;
}

double Taylor::math_e(double x)
{
  return ja_e(x);
}

/*
 * ln : ln value
 * term : a_k
 * x : input x shoule biger then zero,  x > 0
 */

double ja_ln(double u)
{
  int count = 100;
  double ln = 0.0;
  double term = 1.0;

  double x = (u-1) / (u+1);
  int k = 1;
  double xx = x*x;

  term = 1;

  for(int i = 0; i < count; i++){
    ln += term;

    k += 1;
    term = term * (2*k-3)  * xx  / (2*k-1) ;
  }

  return 2*x * ln;
}

double Taylor::math_ln(double x)
{
  return ja_ln(x);
}

/* x belong to [0,360] */
double ja_pi(double n)
{
  //pi is 3.1415926 535898
  double x = 0, pi = 0, sum = 0;
  double num_slipt = n;
  double step = 1.0/(double)num_slipt;
  for(double i = 0; i < num_slipt; i+=1){
    x = (i + 0.5)*step;
    sum += 4.0/(1.0 + x*x) * step;
  }
  pi = sum;
  return pi;
}

double Taylor::math_pi(double n){
  return ja_pi(n);
}
