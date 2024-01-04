#include <stdio.h>

int test();

int main() {
	test();
}

int test() {
	int a = 0;
	int b = 1;
	const int *pa = &a;
	const int *pb = &b;

	const int ** pp = &pa;

	pp = &pb; /* A */
	*pp = &b; /* B */
	**pp = b; /* C */

	return 0;
}
