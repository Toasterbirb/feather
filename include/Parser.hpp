#pragma once
#include "seed/seed.hpp"

class HTMLParser
{
public:
	HTMLParser();
	HTMLParser(const std::vector<seed::string> markdown, std::string title);

	std::string parse() const;
	seed::string parse_line(seed::string line) const;

private:
	std::vector<seed::string> markdown_data;
	std::string page_title;
};
