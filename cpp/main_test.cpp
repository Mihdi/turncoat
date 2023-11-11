#include "test/gamestate_test.hxx"
#include "test/gamehandler_test.hxx"

void test_all(void)
{
	test_gamestate();
	test_gamehandler();
}

int main(int argc, char const *argv[])
{
	test_all();
	return 0;
}