/*
* We will focus on the taylor mathimatic implementation this part
* 1. sin(x)
* 2. cos(x)
* 3. e(x)
* 4. ln(x)
* 5. pi(n)
*/

double ja_sin(double x);
double ja_cos(double x);
double ja_e(double x);
double ja_ln(double u);
double ja_pi(double n);
class Taylor
{
  public:
    /* y = sin(x) */
    double math_sin(double x);
    double math_cos(double x);
    double math_e(double x);
    double math_ln(double u);
    double math_pi(double n);
};
