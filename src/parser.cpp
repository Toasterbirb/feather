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
		int link_start = line.find_char('[', char_pos);
		if (link_start != -1)
		{
			char_pos = link_start;
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

		/* --------------- */
		/* Line separators */
		/* --------------- */
		if (line == "---")
			return "<hr>";

		/* -------------------- */
		/* Bold and italic text */
		/* -------------------- */
		int format_start = line.find_char('*', char_pos);
		if (format_start != -1)
		{
			char_pos = format_start;

			/* Start of a possibly bold or italic text was found.
			 * Check if there's also a second asterisk */
			bool is_bold = false;
			if (line.data()[char_pos + 1] == '*')
				is_bold = true;

			int closing_asterisk = line.find_char('*', char_pos + 2);
			if (closing_asterisk != -1)
			{
				seed::string text_to_format;
				std::string formatted_text;

				if (is_bold && line.data()[closing_asterisk + 1] == '*')
				{
					text_to_format = line.substr(format_start, closing_asterisk + 1);

					/* Check for a special case that would end in an infinite loop */
					if (text_to_format == "****")
						formatted_text = "";
					else
						formatted_text = "<b>" + text_to_format.substr(2, text_to_format.size() - 3).data() + "</b>";
				}
				else
				{
					text_to_format = line.substr(format_start, closing_asterisk);
					formatted_text = "<i>" + text_to_format.substr(1, text_to_format.size() - 2).data() + "</i>";
				}
				line = line.replace(text_to_format, formatted_text);

				char_pos = closing_asterisk + 1;
				ready = false;
			}
		}


		/* --------------- */
		/* Underlined text */
		/* --------------- */
		format_start = line.find_char('_', char_pos);
		if (format_start != -1)
		{
			char_pos = format_start;

			/* Possibly a start of an underlined textblock */
			int closing_underscore = line.find_char('_', char_pos + 1);
			if (closing_underscore != -1)
			{
				seed::string no_underscores = line.substr(format_start, closing_underscore).trim('_');
				std::string underlined_text = "<u>" + no_underscores.data() + "</u>";
				line = line.replace(line.substr(format_start, closing_underscore), underlined_text);

				char_pos = closing_underscore + 1;
				ready = false;
			}
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

	SUBCASE("Line separator")
	{
		CHECK(parser.parse_line("---") == "<hr>");
		CHECK(parser.parse_line("--") == "--");
		CHECK(parser.parse_line("----") == "----");
	}

	SUBCASE("Text formatting")
	{
		SUBCASE("Bold")
		{
			CHECK(parser.parse_line("Some **bold** text") == "Some <b>bold</b> text");
			CHECK(parser.parse_line("Some not **bold text") == "Some not **bold text");
			CHECK(parser.parse_line("**b**") == "<b>b</b>");
			CHECK(parser.parse_line("****") == "");
			CHECK(parser.parse_line("**aa") == "**aa");
		}

		SUBCASE("Italic")
		{
			CHECK(parser.parse_line("Some *italic* text") == "Some <i>italic</i> text");
			CHECK(parser.parse_line("Some not *italic text") == "Some not *italic text");
			CHECK(parser.parse_line("*b*") == "<i>b</i>");
			CHECK(parser.parse_line("**a*") == "<i>*a</i>");
			CHECK(parser.parse_line("**") == "**");
		}

		SUBCASE("Underlined")
		{
			CHECK(parser.parse_line("Some _underlined_ text") == "Some <u>underlined</u> text");
			CHECK(parser.parse_line("Some not _underlined text") == "Some not _underlined text");
			CHECK(parser.parse_line("__") == "<u></u>");
			CHECK(parser.parse_line("_a_") == "<u>a</u>");
		}
	}
}
