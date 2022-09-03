#include "doctest/doctest.h"
#include "Parser.hpp"

HTMLParser::HTMLParser()
{}

HTMLParser::HTMLParser(const std::vector<seed::string> markdown, std::string title)
:markdown_data(markdown), page_title(title)
{}

std::string HTMLParser::parse() const
{
	seed::string html = R"~~(
<html>
	<head>
		<title>{{[TITLE]}}</title>
	</head>
	<body>
{{[BODY]}}
	</body>
</html>
	)~~";

	html = html.replace("{{[TITLE]}}", page_title);

	std::string body = "";
	for (int i = 0; i < markdown_data.size(); i++)
	{
		body += parse_line(markdown_data[i]).data() + "\n";
	}
	html = html.replace("{{[BODY]}}", body);

	return html.data();
}

seed::string HTMLParser::parse_line(seed::string line) const
{
	int char_pos = 0;
	bool ready;

	do
	{
		ready = true;

		/* ------------- */
		/* Parse headers */
		/* ------------- */
		if (line.data()[0] == '#')
		{
			int header_size;
			int max_header_size = 7;
			for (header_size = 1; header_size < line.size(); header_size++)
			{
				if (line.data()[header_size] != '#'
						|| header_size > max_header_size - 1)
					break;
			}

			std::string opening_tag = "<h" + std::to_string(header_size) + ">";
			std::string closing_tag = "</h" + std::to_string(header_size) + ">";

			std::string html_line = opening_tag + line.trim_until(' ').data() + closing_tag;
			line = html_line;

			ready = false;
		}

		/* ----------- */
		/* Parse links */
		/* ----------- */
		char_pos = line.find_char('[', char_pos);
		if (char_pos != -1)
		{
			int opening_square_bracket = char_pos;

			/* We may have found a link. Lets investigate a bit further */
			int closing_square_bracket = line.find_char(']', char_pos + 2);

			int opening_link_bracket = line.find_char('(', closing_square_bracket);

			if (opening_link_bracket != -1)
				char_pos = opening_link_bracket;

			/* The link part isn't starting directly after the link name, so this isn't a link */
			if (opening_link_bracket != closing_square_bracket + 1)
				continue;

			int closing_link_bracket = line.find_char(')', opening_link_bracket + 2);

			/* The end of the link wasn't found */
			if (closing_link_bracket == -1)
				continue;
			else
				char_pos = closing_link_bracket;

			/* We found a link! */
			seed::string link_md_text = line.substr(opening_square_bracket, closing_link_bracket);
			seed::string link_name = line.substr(opening_square_bracket+ 1, closing_square_bracket - 1);
			seed::string link_url = line.substr(opening_link_bracket + 1, closing_link_bracket - 1);

			seed::string link = "<a href=\"" + link_url.data() + "\">" + link_name.data() + "</a>";

			/* Do a search and replace */
			line = line.replace(link_md_text, link);

			ready = false;
		}

	} while (!ready);

	return line;
}

TEST_CASE("Markdown line parsing")
{
	HTMLParser parser;
	SUBCASE("Headers")
	{
		CHECK(parser.parse_line("# some cool header") == "<h1>some cool header</h1>");
		CHECK(parser.parse_line("## some cool header") == "<h2>some cool header</h2>");
		CHECK(parser.parse_line("### some cool header") == "<h3>some cool header</h3>");
		CHECK(parser.parse_line("#### some cool header") == "<h4>some cool header</h4>");
		CHECK(parser.parse_line("##### some cool header") == "<h5>some cool header</h5>");
		CHECK(parser.parse_line("###### some cool header") == "<h6>some cool header</h6>");
		CHECK(parser.parse_line("####### some cool header") == "<h7>some cool header</h7>");
		CHECK(parser.parse_line("######## some cool header") == "<h7>some cool header</h7>");

		CHECK(parser.parse_line("Not # a header") == "Not # a header");
		CHECK(parser.parse_line(" # Title with a space") == " # Title with a space");
	}

	SUBCASE("Links")
	{
		SUBCASE("A basic link")
		{
			CHECK(parser.parse_line("Some text [and](https://alink.something) more text") ==
					"Some text <a href=\"https://alink.something\">and</a> more text");
			CHECK(parser.parse_line("[some link](example.com)") == "<a href=\"example.com\">some link</a>");
		}

		SUBCASE("Link in a header")
		{
			CHECK(parser.parse_line("## [link in a title](example.com)") == "<h2><a href=\"example.com\">link in a title</a></h2>");
		}

		SUBCASE("Two links in the same line")
		{
			CHECK(parser.parse_line("Two [links](example.com) on the [same](some.link.com) line") ==
					"Two <a href=\"example.com\">links</a> on the <a href=\"some.link.com\">same</a> line");
		}

		SUBCASE("Really short links")
		{
			CHECK(parser.parse_line("[a](a)") == "<a href=\"a\">a</a>");
			CHECK(parser.parse_line("[a](a) [a](a) [a](a)") == "<a href=\"a\">a</a> <a href=\"a\">a</a> <a href=\"a\">a</a>");
		}

		SUBCASE("Shouldn't be links")
		{
			CHECK(parser.parse_line("This [link] is not a [link]") == "This [link] is not a [link]");
			CHECK(parser.parse_line("Invalid [](link)") == "Invalid [](link)");
			CHECK(parser.parse_line("[]]()") == "[]]()");
			CHECK(parser.parse_line("[](()") == "[](()");
			CHECK(parser.parse_line("[]())") == "[]())");
		}
	}
}
