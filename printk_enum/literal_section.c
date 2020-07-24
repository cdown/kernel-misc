#include <stdio.h>

int main(int argc, char *argv[])
{
	static const char c[] __attribute__((section("foo"))) = "hi there";
	printf("%s\n", c);
}
