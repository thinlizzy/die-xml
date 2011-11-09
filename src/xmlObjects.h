#ifndef XML_OBJECTSewjhiorj5584jr08j453
#define XML_OBJECTSewjhiorj5584jr08j453

#include <string>
#include "../util/automata.h"

namespace xml {

struct Attribute {
	std::string name;
	std::string value;
	void swap(Attribute && other) throw()
	{
		name.swap(std::move(other.name));
		value.swap(std::move(other.value));
	}
};

typedef automata::FiniteAutomata<automata::Range<char>,automata::MealyTransition<automata::Range<char>>> ParserAutomata;

enum EventState {
	no_event, start_tag, end_tag, empty_tag, 
	start_attribute, attr_name, attr_value, 
	start_chars, end_chars,
	processing_instruction, element_notation, 
	special_element, special_element_ending, special_element_end,
};

enum Exception { ABORTED, PREMATURE_EOF, MALFORMED, EXTRA, TAG_MISMATCH, UNSUPPORTED, };

}

#endif
