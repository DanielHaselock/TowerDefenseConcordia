#include "Color.h"

Math::Color::Color() : r(0), g(0), b(0), a(0)
{
}

Math::Color::Color(int p_r, int p_g, int p_b, int p_a) : r(p_r), g(p_g), b(p_b), a(p_a)
{
}

Math::Color::Color(Color& other) : r(other.r), g(other.g), b(other.b), a(other.a)
{
}


void Math::Color::setColor(int p_r, int p_g, int p_b, int p_a)
{
	r = p_r;
	g = p_g;
	b = p_b;
	a = p_a;
}