//#include <stdio.h>

int main(int argc, char **argv)
{
	if (argc < 2) {
		return 1;
	}

    int a = 100;
    int b = argv[1][0];
    return a + b;
}

