#include "SaxIterators.h"

namespace xml {

namespace sax {

IteratorHelper::IteratorHelper(std::string & buffer, EventState const & event_state, ParserAutomata::Consumer & consumer, std::istream & is):
	buffer(buffer),
	event_state(event_state),
	consumer(consumer),
	is(is)
{}

void IteratorHelper::setEndStates(EventState e1, EventState e2)
{
	endEvt1 = e1;
	endEvt2 = e2;
}

bool IteratorHelper::isDone() const
{
	return event_state == endEvt1 || event_state == endEvt2 || is.fail();
}

char IteratorHelper::consume()
{
	int ch = is.get();
	if( ch == std::char_traits<char>::eof() ) return 0;
	consumer.consume(ch);
	return ch;
}

void IteratorHelper::drain()
{
	while( ! (isDone() || consumer.fail() ) ) {
		consume();
	}
}

// --

AttributeIterator::AttributeIterator(IteratorHelper & ih):
	ih(ih)
{
	ih.setEndStates(start_tag,empty_tag);
}

AttributeIterator::~AttributeIterator()
{
	ih.drain();
}

//* TODO VS ISSUE waiting for a new VS version
optional<Attribute,AutomaticStoragePolicy<Attribute>> AttributeIterator::getNext()
{
	Attribute result;
	if( ! pull(attr_name) ) return optional<Attribute,AutomaticStoragePolicy<Attribute>>();
	result.name = ih.buffer;
	if( ! pull(attr_value) ) return optional<Attribute,AutomaticStoragePolicy<Attribute>>();
	result.value = ih.buffer;
	return result;
}
/*/
optional<Attribute,AutomaticStoragePolicy> AttributeIterator::getNext()
{
	Attribute result;
	if( ! pull(attr_name) ) return optional<Attribute,AutomaticStoragePolicy>();
	result.name = ih.buffer;
	if( ! pull(attr_value) ) return optional<Attribute,AutomaticStoragePolicy>();
	result.value = ih.buffer;
	return result;
}
//*/

bool AttributeIterator::pull(EventState evt)
{
	while( ! ih.isDone() && ih.event_state != evt ) {
		ih.consume();
	}
	return ih.event_state == evt;
}

std::vector<Attribute> AttributeIterator::getAttributes()
{
	std::vector<Attribute> result;
	for(;;) {
		Attribute attr;
		if( ! pull(attr_name) ) break;
		attr.name = ih.buffer;
		if( ! pull(attr_value) ) break;
		attr.value = ih.buffer;
		result.push_back(attr);
	}
	return result;
}

// --

CharIterator::CharIterator(IteratorHelper & ih):
	ih(ih),
	ptrGetChar(ih.buffer.empty() ? &CharIterator::getCharFromStream : &CharIterator::getCharFromBuffer)
{
	ih.setEndStates(end_chars,special_element_end);
}

CharIterator::~CharIterator()
{
	ih.drain();
}

char CharIterator::getCharFromBuffer()
{
	char result = ih.buffer[0];
	ih.buffer.erase(ih.buffer.begin());
	if( ih.buffer.empty() ) {
		ptrGetChar = &CharIterator::getCharFromStream;
	}
	return result;
}

char CharIterator::getCharFromStream()
{
	if( ih.isDone() ) return 0;
	char ch = ih.consume();
	if( ih.event_state == special_element_ending ) return processEndCData();
	return ih.isDone() ? 0 : ch;
}

char CharIterator::processEndCData()
{
	do {
		ih.consume();
	} while( ih.event_state == special_element_ending );

	if( ih.buffer.empty() ) return 0;
	ptrGetChar = &CharIterator::getCharFromBuffer;
	return getCharFromBuffer();
}

char CharIterator::getChar()
{
	return (this->*ptrGetChar)();
}

std::string CharIterator::getText()
{
	std::string result;
	char ch;
	while( (ch = getChar()) != 0 ) {
		result.push_back(ch);
	}
	return result;
}


}
}