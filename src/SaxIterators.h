#ifndef SAX_ITERATORS_H_4893uutfguuuj436cvkew32
#define SAX_ITERATORS_H_4893uutfguuuj436cvkew32

#include "xmlObjects.h"
#include <istream>
#include <vector>
#include "../util/optional.h"

namespace xml {

namespace sax {

class IteratorHelper {
public:
	std::string & buffer;
	EventState const & event_state;
private:
	ParserAutomata::Consumer & consumer;
	std::istream & is;
	EventState endEvt1;
	EventState endEvt2;
public:
	IteratorHelper(std::string & buffer, EventState const & event_state, ParserAutomata::Consumer & consumer, std::istream & is);
	void drain();
	void setEndStates(EventState e1, EventState e2);
	bool isDone() const;
	char consume();
};

class AttributeIterator {
	IteratorHelper & ih;
	bool pull(EventState evt);
public:
	AttributeIterator(IteratorHelper & ih);
	~AttributeIterator();
	optional<Attribute,AutomaticStoragePolicy> getNext();
	std::vector<Attribute> getAttributes();
};

class CharIterator {
	IteratorHelper & ih;
	char (CharIterator::*ptrGetChar)();
	char getCharFromBuffer();
	char getCharFromStream();
	char processEndCData();
public:
	CharIterator(IteratorHelper & ih);
	~CharIterator();
	char getChar();
	std::string getText();
};


}
}

#endif

