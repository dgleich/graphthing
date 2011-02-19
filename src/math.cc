//
//	math.cc
//

#include "math.h"


int Math::choose (int n, int m)
{
	static const int cache[] = {	// half of Pascal's triangle
		1,
		1,
		1, 2,
		1, 3,
		1, 4, 6,
		1, 5, 10,
		1, 6, 15, 20,
		1, 7, 21, 35,
		1, 8, 28, 56, 70,
		1, 9, 36, 84, 126,
		1, 10, 45, 120, 210, 252,
		1, 11, 55, 165, 330, 462,
		1, 12, 66, 220, 495, 792, 924,
		1, 13, 78, 286, 715, 1287, 1716,
		1, 14, 91, 364, 1001, 2002, 3003, 3432,
		1, 15, 105, 455, 1365, 3003, 5005, 6435,
	};
	static const int CACHE_MAX = 15;

	if (m > n - m)
		m = n - m;
	if (m < 0)
		return 0;
	if (n <= CACHE_MAX) {
		if (n & 1)
			return cache[(n + 1) * (n + 1) / 4 + m];
		else
			return cache[(n + 2) * n / 4 + m];
	}

	return (Math::choose (n - 1, m - 1) + Math::choose (n - 1, m));
}

int Math::factorial (int n)
{
	static const int cache[] = {
		1, 1, 2, 6, 24, 120,			// 0 - 5
		720, 5040, 40320, 362880, 3628800,	// 6 - 10
		39916800, 479001600			// 11, 12
	};
	static const int CACHE_MAX = 12;
	int i, tot;

	if (n < 2)
		return 1;
	if (n <= CACHE_MAX)
		return cache[n];

	// FIXME: Uh, oh! Overflow!
	for (i = 13, tot = cache[12]; i <= n; ++i)
		tot *= i;

	return tot;
}

int Math::stirling (int n, int k)
{
	if (n == k)
		return 1;
	if ((n <= 0) || (k <= 0))
		return 0;
	if (k == 1)
		return Math::factorial (n - 1);
	if (n == (k + 1))
		return Math::choose (n, 2);
	if (k > n)
		return 0;

	return ((n - 1) * Math::stirling (n - 1, k) +
			Math::stirling (n - 1, k - 1));
}
