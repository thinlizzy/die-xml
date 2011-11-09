#ifndef SAX_PARSER_H_j342dfssssdr3242
#define SAX_PARSER_H_j342dfssssdr3242

#include <istream>
#include <functional>
#include <string>
#include <stack>
#include "xmlObjects.h"
#include "SaxIterators.h"

namespace xml {

namespace sax {

class Parser {
public:
	typedef std::string TagType;
	typedef std::function<void(TagType const & name, AttributeIterator & it)> start_tag_event;
	typedef std::function<void(CharIterator & it)> char_event;
	typedef std::function<void(TagType const & name)> end_tag_event;
	typedef std::function<void(TagType const & processingInstruction, std::string const & arguments)> processing_instruction_event;
private:
	start_tag_event startDocFn;
	end_tag_event endDocFn;
	start_tag_event startElementFn;
	char_event charactersFn;
	end_tag_event endElementFn;

	processing_instruction_event procInstrFn;
	processing_instruction_event elementFn;

	ParserAutomata parserAut;
	EventState event_state;
	std::string buffer;
	TagType tagName;
	typedef std::stack<TagType> TagStack;
	TagStack tags;
	enum {prologue, doctype, start_document, inside_document, end_document} parser_state;

	void Parser::tagNameAdd(char s);
public:
	Parser();
	Parser & startDocument(start_tag_event fn);
	Parser & startTag(start_tag_event fn);
	Parser & characters(char_event fn);
	Parser & endTag(end_tag_event fn);
	Parser & endDocument(end_tag_event fn);

	Parser & processingInstruction(processing_instruction_event fn);
	Parser & element(processing_instruction_event fn);

	void parse(std::istream & is);
};


}
}

#endif
