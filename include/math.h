/*
* We will focus on the basic mathimatic implementation this part
*/

class Math
{
  public:
    /* y = a + b */
    int math_add(int x, int y);
    double math_add(double x, double y);

    /* y = x^2 */
    int math_square(int x);
    double math_square(double x);

    /* y^2 = x^3 + a*x +b */
    double math_ecc(double x, double a, double b);

    /* y = sqrt x */
    /*x_(k+1) = x_k - f(x_k) / (f`(x_k))*/
    double math_square_root(double x);
    double math_abs(double x);
};
