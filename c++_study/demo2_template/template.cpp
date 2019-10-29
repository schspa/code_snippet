#include <iostream>

// C++模版(泛型)demo

template <typename T>
void myswap(T & x, T & y)
{
	T tmp;
	tmp = x;
	x = y;
	y = tmp;
}

int main(int argc, char *argv[])
{
	using namespace std;
	
	int a = 2, b = 3;
	cout << "a, b = " << a << ", " << b << endl;
	myswap(a, b);
	cout << "a, b = " << a << ", " << b << endl;

	double c = 2.1, d = 3.3;
	cout << "c, d = " << c << ", " << d << endl;
	myswap(c, d);
	cout << "c, d = " << c << ", " << d << endl;

	return 0;
}
