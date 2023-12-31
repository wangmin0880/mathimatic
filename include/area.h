/*
* We will focus on the area topic this part
*/

class Area
{
  public:
    /*
    * circle area
    * y = pi * r^2
    */
    double math_circle_area(double r);

    /*
    * annulus area
    * y = pi * (R^2 - r^2)
    * y = pi * (R + r) * (R-r)
    */
    double math_annulus_area(double R, double r);

    /*
    * ellipse area
    * y = pi * a * b
    */
    double math_ellipse_area(double a, double b);
};
