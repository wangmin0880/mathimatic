/* y = a + b */
#define m_add(x, y) (x + y)

int math_add(int x, int y)
{
    return m_add(x,y);
}

double math_add(double x, double y)
{
    return m_add(x,y);
}

/* y = x^2 */
#define m_square(x) x*x

int math_square(int x)
{
    return m_square(x);
}

double math_square(double x)
{
    return m_square(x);
}
