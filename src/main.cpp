#define DOCTEST_CONFIG_IMPLEMENT
#include "doctest/doctest.h"
#include "seed/seed.hpp"
#include "Parser.hpp"

int main(int argc, char** argv)
{
	seed::ArgsParser arg_parser(argc, argv);

	if (!arg_parser.is_valid())
	{
		std::cout << "Invalid arguments detected, quitting..." << std::endl;
		return 1;
	}

	/* Run tests and quit */
	if (arg_parser.key()["test"] == "true")
	{
#ifdef TEST
		std::cout << "Running tests..." << std::endl;
		doctest::Context context;
		int res = context.run();
		if (context.shouldExit())
			return res;

		return 0;
#else
		std::cout << "Tests weren't compiled into feather. Recompile feather with -DTESTS CMake option to enable tests" << std::endl;
#endif /* TEST */
	}

	std::string md_input = arg_parser.key()["i"];
	if (md_input == "")
	{
		std::cout << "No input file was provided" << std::endl;
	}

	std::vector<seed::string> md_lines = seed::ReadFileLines(md_input);

	HTMLParser html_parser(md_lines, "Test site");
	std::string html = html_parser.parse();
	std::cout << html << std::endl;


	return 0;
}
