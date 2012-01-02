#include "SaxParser.h"

namespace xml {
namespace sax {

// http://www.w3.org/TR/REC-xml/

void Parser::tagNameAdd(char s)
{ 
	tagName.push_back(s); 
};

static std::string const S = " \t\n\r";
static std::string const NameStartChar = "a-zA-Z_:";

Parser::Parser():
	startDocFn([](TagType const & name, AttributeIterator & it){}),
	endDocFn([](TagType const & name){}),
	startElementFn([](TagType const & name, AttributeIterator & it){}),
	charactersFn([](CharIterator & it){}),
	endElementFn([](TagType const & name){}),
	procInstrFn([](TagType const & processingInstruction, std::string const & arguments){}),
	elementFn([](TagType const & processingInstruction, std::string const & arguments){})
{
	auto limpaTag = [this](char s){ tagName.clear(); };
	auto limpaBuffer = [this](char s){ buffer.clear(); };
	auto geraStartTag = [this](char s){ 
		event_state = start_tag; 
		buffer.clear();
	};
	auto geraEndTag = [this](char s){ 
		event_state = end_tag; 
	};
	using namespace std;
	auto addTagName = bind1st(mem_fun(&Parser::tagNameAdd),this);

	using namespace automata;

	RangeSetter<char,MealyTransition> rs(parserAut);
	typedef decltype(parserAut) Automata;

	rs.setTrans("start",S,"start");

	parserAut.setTrans("start",'<',"tagStart").output = limpaTag;

	// <?
	parserAut.setTrans("tagStart",'?',"pi");
	rs.setTrans("pi",S,"piS",limpaBuffer);
	parserAut.getNode("pi").defaultTransition = [this](char ch) -> Automata::NodeType * { 
		tagNameAdd(ch);
		return &parserAut.getNode("pi");
	};
	rs.setTrans("piS",S,"piS");
	parserAut.setTrans("piS",'?',"fimPi1");
	parserAut.getNode("piS").defaultTransition = [this](char ch) -> Automata::NodeType * { 
		buffer.push_back(ch);
		return &parserAut.getNode("piChars");
	};
	parserAut.getNode("piChars").defaultTransition = parserAut.getNode("piS").defaultTransition;
	parserAut.setTrans("piChars",'?',"fimPi1");
	parserAut.setTrans("fimPi1",'>',"start").output = [this](char s){ event_state = processing_instruction; };

	// <! elements (doctype) e cdata
	parserAut.setTrans("tagStart",'!',"!");
	// comentario
	parserAut.setTrans("!",'-',"commentS");
	parserAut.setTrans("commentS",'-',"comment");
	parserAut.getNode("comment").defaultTransition = [this](char ch) -> Automata::NodeType * { 
		return &parserAut.getNode("comment");
	};
	parserAut.setTrans("comment",'-',"commentHyphen");
	parserAut.getNode("commentHyphen").defaultTransition = parserAut.getNode("comment").defaultTransition;
	parserAut.setTrans("commentHyphen",'-',"commentEnd");
	parserAut.setTrans("commentEnd",'>',"start");
	// element
	rs.setTrans("!",NameStartChar,"element",[this](char s){ tagName = s; });
	parserAut.getNode("element").defaultTransition = [this](char ch) -> Automata::NodeType * { 
		tagNameAdd(ch);
		return &parserAut.getNode("element");
	};
	rs.setTrans("element",S,"elementText",limpaBuffer);
	parserAut.getNode("elementText").defaultTransition = [this](char ch) -> Automata::NodeType * { 
		buffer.push_back(ch);
		return &parserAut.getNode("elementText");
	};
	parserAut.setTrans("elementText",'>',"start").output = [this](char s){ event_state = element_notation; };
	// cdata like
	parserAut.setTrans("!",'[',"![");
	parserAut.setTrans("![",'[',"element[").output = [this](char s){ 
		event_state = special_element; 
		buffer.clear();
	};
	parserAut.getNode("![").defaultTransition = [this](char ch) -> Automata::NodeType * { 
		tagNameAdd(ch);
		return &parserAut.getNode("![");
	};
	parserAut.getNode("element[").defaultTransition = [this](char ch) -> Automata::NodeType * { 
		return &parserAut.getNode("element[");
	};
	auto specialElementEndingFn = [this](char s){ 
		event_state = special_element_ending; 
		buffer.push_back(']');
	};
	parserAut.setTrans("element[",']',"endElement]").output = specialElementEndingFn;
	parserAut.setTrans("endElement]",']',"endElement]]").output = specialElementEndingFn;
	parserAut.setTrans("endElement]]",']',"endElement]]").output = specialElementEndingFn;
	parserAut.setTrans("endElement]]",'>',"start").output = [this](char s){ 
		event_state = special_element_end;
		buffer.resize(buffer.size()-2);
	};
	parserAut.getNode("endElement]").defaultTransition = [this](char ch) -> Automata::NodeType * { 
		buffer.push_back(ch);
		event_state = no_event;
		return &parserAut.getNode("element[");
	};
	parserAut.getNode("endElement]]").defaultTransition = parserAut.getNode("endElement]").defaultTransition;

	// xml tag
	rs.setTrans("tagStart",NameStartChar,"tagName",addTagName);
	parserAut.getNode("tagName").defaultTransition = [this](char ch) -> Automata::NodeType * { 
		tagNameAdd(ch);
		return &parserAut.getNode("tagName");
	};
	parserAut.setTrans("tagName",'/',"emptyTag");
	parserAut.setTrans("tagName",'>',"start").output = geraStartTag;

	rs.setTrans("tagName",S,"tagAttrSpace");
	rs.setTrans("tagAttrSpace",S,"tagAttrSpace");
	parserAut.setTrans("tagAttrSpace",'/',"emptyTag");

	parserAut.setTrans("emptyTag",'>',"start").output = [this](char s){ 
		event_state = empty_tag;
		buffer.clear();
	};

	// TODO deixar o automato mais rigido nos atributos para gerar os malformed mais vezes

	rs.setTrans("tagAttrSpace",NameStartChar,"tagAttrName",[this](char s){ 
		event_state = start_attribute; 
		buffer.clear();
		buffer.push_back(s);
	});
	parserAut.getNode("tagAttrName").defaultTransition = [this](char s) -> Automata::NodeType * { 
		buffer.push_back(s);
		return &parserAut.getNode("tagAttrName");
	};

	auto attrNameFn = [this](char s){ event_state = attr_name; };
	auto attrValueFn = [this](char s){ event_state = attr_value; };

	rs.setTrans("tagAttrName",S,"tagAttrNameS");
	rs.setTrans("tagAttrNameS",S,"tagAttrNameS");
	parserAut.setTrans("tagAttrNameS",'=',"tagAttrValueStart").output = attrNameFn;
	parserAut.setTrans("tagAttrName",'=',"tagAttrValueStart").output = attrNameFn;
	rs.setTrans("tagAttrValueStart",S,"tagAttrValueStart");

	parserAut.setTrans("tagAttrValueStart",'\'',"tagAttrValue1").output = limpaBuffer;
	parserAut.setTrans("tagAttrValueStart",'"',"tagAttrValue2").output = limpaBuffer;
	parserAut.setTrans("tagAttrValue1",'\'',"tagAttrSpace").output = attrValueFn;
	parserAut.setTrans("tagAttrValue2",'"',"tagAttrSpace").output = attrValueFn;

	parserAut.getNode("tagAttrValue1").defaultTransition = [this](char s) -> Automata::NodeType * { 
		buffer.push_back(s);
		return &parserAut.getNode("tagAttrValue1");
	};
	parserAut.getNode("tagAttrValue2").defaultTransition = [this](char s) -> Automata::NodeType * { 
		buffer.push_back(s);
		return &parserAut.getNode("tagAttrValue2");
	};

	// sem atributos. só espacos gays
	parserAut.setTrans("tagAttrSpace",'>',"start").output = geraStartTag;

	// nova tag, ignora buffer vazio
	parserAut.setTrans("start",'<',"tagStart").output = limpaTag;

	// qualquer outra coisa? gera evento de chars e o pullador vai usar o buffer
	parserAut.getNode("start").defaultTransition = [this](char s) -> Automata::NodeType * { 
		buffer.push_back(s);
		event_state = start_chars;
		return &parserAut.getNode("TextNode");
	};

	parserAut.setTrans("TextNode",'<',"tagStart").output = [=](char s){ 
		event_state = end_chars; 
		limpaTag(s);
	};
	parserAut.getNode("TextNode").defaultTransition = [this](char s) -> Automata::NodeType * { 
		return &parserAut.getNode("TextNode");
	};

	parserAut.setTrans("tagStart",'/',"tagEnd");
	rs.setTrans("tagEnd",NameStartChar,"tagEndName",addTagName);
	parserAut.getNode("tagEndName").defaultTransition = [this](char ch) -> Automata::NodeType * { 
		tagNameAdd(ch);
		return &parserAut.getNode("tagEndName");
	};

	rs.setTrans("tagEndName",S,"tagEndSpace");
	rs.setTrans("tagEndSpace",S,"tagEndSpace");
	parserAut.setTrans("tagEndName",'>',"start").output = geraEndTag;
	parserAut.setTrans("tagEndSpace",'>',"start").output = geraEndTag;

	parserAut.setStart("start");
	parserAut.getNode("start").final = true;
}

#define SR(var) var = fn; return *this;

Parser & Parser::startDocument(start_tag_event fn) { SR(startDocFn) }
Parser & Parser::startTag(start_tag_event fn) { SR(startElementFn) }
Parser & Parser::characters(char_event fn) { SR(charactersFn) }
Parser & Parser::endTag(end_tag_event fn) { SR(endElementFn) }
Parser & Parser::endDocument(end_tag_event fn) { SR(endDocFn) }
Parser & Parser::processingInstruction(processing_instruction_event fn) { SR(procInstrFn) }
Parser & Parser::element(processing_instruction_event fn) { SR(elementFn) }

void Parser::parse(std::istream & is)
{
	parser_state = prologue;
	event_state = no_event;
	buffer.clear();
	TagStack().swap(tags);		// only vs 2010 supports swap to rvalues on std::stack

	auto consumer = parserAut.getConsumer();
	IteratorHelper ih(buffer,event_state,consumer,is);
	for(;;) {
		int ch = is.get();
		if( ch == std::char_traits<char>::eof() ) {
			if( consumer.final() && parser_state == end_document ) return;
			throw PREMATURE_EOF;
		}
		if( ! consumer.consume(ch) ) throw MALFORMED;

		while( event_state != no_event ) {
			if( parser_state == end_document ) throw EXTRA;

			switch(event_state) {
			case empty_tag:
				// o mesmo que start_tag, mas depois fará end_tag
			case start_tag:
			case start_attribute:
				tags.push(tagName);
				{	AttributeIterator attrIt(ih);
					if( parser_state <= start_document ) {
						startDocFn(tagName,attrIt);
						parser_state = inside_document;
					} else {
						startElementFn(tagName,attrIt);
					}
				}

				break;
			case end_tag:
				if( tags.empty() || tags.top() != tagName ) throw TAG_MISMATCH;
				tags.pop();
				if( tags.empty() ) {
					endDocFn(tagName);
					parser_state = end_document;
				} else {
					endElementFn(tagName);
				}
				break;
			case start_chars:
				{	CharIterator charIt(ih);
					charactersFn(charIt);
				}
				break;
			case special_element:
				if( tagName == "CDATA" ) {
					CharIterator charIt(ih);
					charactersFn(charIt);
				} else {
					throw UNSUPPORTED;				// TODO improve this to support conditionals
				}
				break;

			case processing_instruction:
				if( tagName == "xml" ) {
					if( parser_state != prologue ) throw MALFORMED;
					parser_state = doctype;
					// TODO interpretar encoding e criar decoders
				}
				procInstrFn(tagName,buffer);
				break;

			case element_notation:
				if( tagName == "DOCTYPE" ) {
					if( parser_state != doctype ) throw MALFORMED;
					parser_state = start_document;
					// TODO interpretar DOCTYPE
				}
				elementFn(tagName,buffer);
				break;
			}

			if( event_state == empty_tag ) {
				event_state = end_tag;
			} else {
				event_state = no_event;
			}
				
		}
	}
}


}
}

