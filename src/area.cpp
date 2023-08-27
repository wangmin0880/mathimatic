/*
* We will focus on the area topic this part
*                   ----wangmin0880@163.com
*/

#include "area.h"

/*
* circle area
* y = pi * r^2
*/
#define m_pi 3.1415926536
#define m_circle_area(r) (m_pi * (r * r))

/*
* annulus area
* y = pi * (R^2 - r^2)
* y = pi * (R + r) * (R-r)
*/
#define m_annulus_area(R,r) (m_pi * ((R + r) * (R - r)))
#define m_annulus_areaA(R,r) (m_pi * (R*R - r*r))

/*
* ellipse area
* y = pi * a * b
*/
#define m_ellipse_area(a,b) (m_pi * a * b)

double Area::math_circle_area(double r)
{
  return m_circle_area(r);
}

double Area::math_annulus_area(double R, double r)
{
    return m_annulus_areaA(R,r);
}

double Area::math_ellipse_area(double a, double b)
{
  return m_ellipse_area(a,b);
}
