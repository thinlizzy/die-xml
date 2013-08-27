#include <tut.h>
#include <sstream>

#include "../../src/SaxParser.h"

namespace {
    struct setup {
		xml::sax::Parser parser;
    };
}

namespace tut { 
    typedef test_group<setup> tg;
    tg saxparser_test_group("sax parser");
    
    typedef tg::object testobject;
    
    template<> 
    template<> 
    void testobject::test<1>() 
    {
        set_test_name("well formed");
		std::istringstream ss("<root></root>  ");
		parser.parse(ss);

		ss.str("<root>");
		ss.clear();
		try {
			parser.parse(ss);
		} catch(xml::Exception ex) {
			ensure_equals(ex,xml::PREMATURE_EOF);
		}

		ss.str("<root></ruut>");
		ss.clear();
		try {
			parser.parse(ss);
		} catch(xml::Exception ex) {
			ensure_equals(ex,xml::TAG_MISMATCH);
		}

		ss.str("<root><0sub></0sub></root>");
		ss.clear();
		try {
			parser.parse(ss);
		} catch(xml::Exception ex) {
			ensure_equals(ex,xml::MALFORMED);
		}

    }

    template<> 
    template<> 
    void testobject::test<2>() 
    {
        set_test_name("well formed with text");
		std::istringstream ss("<root><sub>text</sub></root>");
		parser.parse(ss);
    }

    template<> 
    template<> 
    void testobject::test<3>() 
    {
        set_test_name("well formed with text and attributes");
		std::istringstream ss("<root naosei='20'><sub attr=\"10\" other='dsfsdfs'>text</sub></root>");
		parser.parse(ss);
    }

    template<> 
    template<> 
    void testobject::test<4>() 
    {
        set_test_name("well formed with empty tags");
		std::istringstream ss("<root naosei='20'><sub attr=\"10\" other='dsfsdfs' /></root>");
		parser.parse(ss);
		ss.str("<root><sub/></root>");
		ss.clear();
		parser.parse(ss);
    }

    template<> 
    template<> 
    void testobject::test<5>() 
    {
		using namespace xml::sax;
        set_test_name("events start and end");
		std::istringstream ss("<root naosei='20'>\n<sub attr=\"10\" other='dsfsdfs' />    <tag2   >texto</tag2>   </root>");
		Parser::TagType startDoc;
		Parser::TagType tags,tagsEnd;
		parser.startDocument([&](Parser::TagType const & name, AttributeIterator & it) { startDoc = name; });
		parser.endDocument([&](Parser::TagType const & name) { ensure_equals(name,startDoc); });
		parser.startTag([&](Parser::TagType const & name, AttributeIterator & it) { tags += name; });
		parser.endTag([&](Parser::TagType const & name) { tagsEnd += name; });
		parser.parse(ss);
		ensure_equals(startDoc,"root");
		ensure_equals(tags,"subtag2");
		ensure_equals(tagsEnd,tags);
	}

    template<> 
    template<> 
    void testobject::test<6>() 
    {
		using namespace xml::sax;
        set_test_name("event start using attributes");
		std::istringstream ss(
			"<root naosei='20'>\n"
				"<sub attr=\"10\" cost='BBR' other='dsfsdfs' />    "
				"<tag2   >texto</tag2>   "
			"</root    >");
		parser.startTag([](Parser::TagType const & name, AttributeIterator & it) {
			if( name == "sub" ) {
				int found = 0;
				auto attribute = it.getNext();
				while( attribute && found < 2 ) {
					if( (attribute->name == "attr" && attribute->value == "10") ||
						(attribute->name == "cost" && attribute->value == "BBR") ) ++found;
					attribute = it.getNext();
				}
				if( found != 2 ) throw xml::ABORTED;
			}
		});
		parser.parse(ss);
	}

    template<> 
    template<> 
    void testobject::test<7>() 
    {
		using namespace xml::sax;
        set_test_name("character data");
		std::istringstream ss(
			"<root naosei='20'>\n"
				"<tag1>texto inicial</tag1>"
				"<sub attr=\"10\" cost='BBR' other='dsfsdfs' />    "
				"<tag2   >texto</tag2>   "
			"</root    >");
		std::string texto;
		parser.characters([&](CharIterator & it) {
			texto += it.getText();
		});
		parser.parse(ss);
		ensure_equals(texto,"texto inicialtexto");
	}

    template<> 
    template<> 
    void testobject::test<8>() 
    {
		using namespace xml::sax;
        set_test_name("processing instructions");
		std::istringstream ss(
			"<?xml encoding=\"ISO-8859-1\"?>"
			"<root naosei='20'>\n"
				"<tag1>texto inicial</tag1>"
				"<?teucu instrucao?>"
				"<sub attr=\"10\" cost='BBR' other='dsfsdfs' />    "
				"<tag2   >texto</tag2>   "
			"</root    >");
		std::string pi;
		std::string args;
		parser.processingInstruction([&](Parser::TagType const & name, std::string const & arguments) {
			pi += name;
			args += arguments;
		});
		parser.parse(ss);
		ensure_equals(pi,"xmlteucu");
		ensure_equals(args,"encoding=\"ISO-8859-1\"instrucao");
	}

    template<> 
    template<> 
    void testobject::test<9>() 
    {
        set_test_name("well formed with prologue");
		std::istringstream ss(
			"<?xml version=\"1.0\" encoding=\"ISO-8859-1\"?>\n\r"
			"<root><sub></sub></root>");
		parser.parse(ss);

		ss.str("<root><sub></sub><?xml version=\"1.0\" encoding=\"ISO-8859-1\"?></root>");
		ss.clear();
		try {
			parser.parse(ss);
		} catch(xml::Exception ex) {
			ensure_equals(ex,xml::MALFORMED);
		}
	}

    template<> 
    template<> 
    void testobject::test<10>() 
    {
        set_test_name("comments");
		std::istringstream ss("<root><sub a='1'>alasksf</sub><!-- this game sucks --><a>dd</a></root>");
		parser.parse(ss);
	}

    template<> 
    template<> 
    void testobject::test<11>() 
    {
		using namespace xml::sax;
        set_test_name("elements");
		std::istringstream ss(
			"<?xml encoding=\"ISO-8859-1\"?>"
			"<!DOCTYPE greeting SYSTEM \"hello.dtd\">"
			"<root naosei='20'>\n"
				"<tag1>texto inicial</tag1>"
				"<!ELEMENT br EMPTY>"
				"<sub attr=\"10\" cost='BBR' other='dsfsdfs' />    "
				"<tag2   >texto</tag2>   "
				"<!NOTATION usdruvs PUBLIC argh>"
			"</root    >");
		std::string pi;
		std::string args;
		parser.element([&](Parser::TagType const & name, std::string const & arguments) {
			pi += name;
			args += arguments;
		});
		parser.parse(ss);
		ensure_equals(pi,"DOCTYPE""ELEMENT""NOTATION");
		ensure_equals(args,"greeting SYSTEM \"hello.dtd\"""br EMPTY""usdruvs PUBLIC argh");
	}

    template<> 
    template<> 
    void testobject::test<12>() 
    {
		using namespace xml::sax;
        set_test_name("cdata 1");
		std::istringstream ss(
			"<?xml encoding=\"ISO-8859-1\"?>"
			"<!DOCTYPE greeting SYSTEM \"hello.dtd\">"
			"<root naosei='20'>\n"
				"<tag1>antes</tag1>"
				"<tagsafada><![CDATA[123456]]></tagsafada>"
				"<!ELEMENT br EMPTY>"
				"<tag2   >meio</tag2>   "
				"<tagsafada2><![CDATA[ ai [dede] ]]></tagsafada2>"
				"<tag3   >depois</tag3>   "
			"</root    >");
		std::string texto;
		parser.characters([&](CharIterator & it) {
			texto += it.getText();
		});
		parser.parse(ss);
		ensure_equals(texto,"antes""123456""meio"" ai [dede] ""depois");
	}

    template<> 
    template<> 
    void testobject::test<13>() 
    {
		using namespace xml::sax;
        set_test_name("cdata 2");
		std::istringstream ss(
			"<root naosei='20'>\n"
				"<tagsafada3><![CDATA[ai [[didi]]]]></tagsafada3>"
				"<tag3   >depois</tag3>   "
			"</root    >");
		std::string texto;
		parser.characters([&](CharIterator & it) {
			texto += it.getText();
		});
		parser.parse(ss);
		ensure_equals(texto,"ai [[didi]]""depois");
	}

    template<> 
    template<> 
    void testobject::test<14>() 
    {
        set_test_name("not aborted");
		std::istringstream ss("<root></root>  ");
		ensure_not(parser.parse(ss));
	}

    template<> 
    template<> 
    void testobject::test<15>() 
    {
		using namespace xml::sax;
        set_test_name("abort and continue");
		std::istringstream ss(
			"<root naosei='20'>\n"
				"<tag1>aah</tag1>"
				"<tagdef tagName='superTag' other='dsfsdfs' another='xxx' />    "
				"<otherTag>irrelevant text</otherTag>   "
				"<superTag>this is the answer</superTag>"
				"<tag2>bah</tag2>"
			"</root>");
		std::string tagName;
		parser.startTag([&](Parser::TagType const & name, AttributeIterator & it) {
			if( name == "tagdef" ) {
				for( auto attribute = it.getNext(); attribute; attribute = it.getNext() ) {
					if( attribute->name == "tagName" ) {
						tagName = attribute->value;
						throw xml::ABORTED;
					}
				}
			}
		});

		if( parser.parse(ss) ) {
			bool getText = false;
			std::string texto;
			parser.startTag([&](Parser::TagType const & name, AttributeIterator & it) {
				if( name == tagName ) {
					getText = true;
				}
			});
			parser.endTag([&](Parser::TagType const & name) {
				if( name == tagName ) throw xml::ABORTED;
			});
			parser.characters([&](CharIterator & it) {
				if( getText ) {
					texto += it.getText();
				}
			});
			if( parser.parseContinue(ss) ) {
				ensure_equals(texto,"this is the answer");
			} else {
				fail("parsing was not aborted on second time");
			}
		} else {
			fail("parsing was not aborted on first time");
		}
	}

    template<> 
    template<> 
    void testobject::test<16>() 
    {
		using namespace xml::sax;
        set_test_name("abort and continue");
		std::istringstream ss(
			"<root>"
				"<fieldtag>aah1</fieldtag>"
				"<fieldtag>aah2</fieldtag>"
			"</root>");
		parser.endTag([&](Parser::TagType const & name) {
			if( name == "fieldtag" ) throw xml::ABORTED;
		});

		ensure_equals( parser.parse(ss), true );
		ensure_equals( parser.parseContinue(ss), true );
		ensure_equals( parser.parseContinue(ss), false );
	}


}
