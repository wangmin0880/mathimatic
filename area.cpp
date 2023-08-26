/*
* We will focus on the area topic this part
*                   ----wangmin0880@163.com
*/
/*
* circle area
* y = pi * r^2
*/
#define m_pi 3.1415926536
#define m_circle_area(r) (m_pi * (r * r))

double math_circle_area(double r)
{
    return m_circle_area(r);
}
/*
* annulus area
* y = pi * (R^2 - r^2)
* y = pi * (R + r) * (R-r)
*/
#define m_annulus_area(R,r) (m_pi * ((R + r) * (R - r)))
#define m_annulus_areaA(R,r) (m_pi * (R*R - r*r))

double math_annulus_area(double R, double r)
{
    return m_annulus_areaA(R,r);
}
