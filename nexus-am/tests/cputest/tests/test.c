#include "trap.h"

long long add(long long a, long long b) {
	long long c = a + b;
	return c;
}

int main()
{
    long long i = 0, j = 0;
    long long k = add(i, j);
    nemu_assert(k == 0L);
}