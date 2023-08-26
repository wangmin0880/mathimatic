/* y = a + b */
#define m_add(x, y) (x + y)
double math_add(double x, double y)
{
    return m_add(x,y);
}

/* y = x^2 */
#define m_square(x) x*x
double math_square(double x)
{
    return m_square(x);
}
