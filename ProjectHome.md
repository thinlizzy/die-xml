# Diego's XML Parser #
a simple XML parser that uses finite automata as backend. see FiniteAutomaton

## Features ##

  * parse all kinds of XML tags
  * C++0x API
  * SAX event based interface generating the following events:
    * start document
    * end document
    * start tag
    * end tag
    * characters (including CDATA)
    * processing instruction
    * notation/element definition
  * AttributeIterator class for start document and start tag events. This allows the application to pull attribute values during the event and avoids unnecessary buffering for undesired tags
  * CharIterator class for characters event. This allows the application to pull chars during the event and avoids unnecessary buffering for undesired text nodes

### Bonus Track ###

  * FiniteAutomata class template that can be extended to a Mealy Machine
  * a lightweight implementation of boost::optional<> with storage policy support

## TBD ##

  * entities decoding
  * UTF-8 support (and other encodings too)
  * support configuration for ignorable white space on text nodes

## Missing ##

  * DOM parser (can be easily built over SAX)
  * DOCTYPE validation
  * namespaces
  * text nodes outside tags
  * does not check duplicate notation and element definitions. These are handled by application events only