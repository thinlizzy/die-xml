#ifndef AUTOMATA_H_MEL_CROUCHER_jdf328942j493
#define AUTOMATA_H_MEL_CROUCHER_jdf328942j493

#include <map>
#include <unordered_map>
#include <string>
#include <utility>
#include <functional>
#include <cassert>
// debug
//#include <iostream>

namespace automata {

template<typename C, typename TR>
class Node;

template<typename C>
struct BasicTransition {
	Node<C,BasicTransition> * node;
	inline void operator()(C s) const {}
};

template<typename C>
struct MealyTransition {
	Node<C,MealyTransition> * node;
	typedef std::function<void(C)> output_type;
	output_type output;
	inline void operator()(C s) const { output(s); }
	MealyTransition() 
	{
		output = [](C s){};
	}
};

template<typename C, typename TR>
struct map_traits {
	typedef std::map<C,TR> map_type;
	static typename map_type::const_iterator find(map_type const & map, C const & key) { return map.find(key); }
	static TR const & getValue(typename map_type::const_iterator it) { return it->second; }
};

template<typename TR>
struct map_traits<char,TR> {
	typedef std::unordered_map<char,TR> map_type;
	static typename map_type::const_iterator find(map_type const & map, char const & key) { return map.find(key); }
	static TR const & getValue(typename map_type::const_iterator it) { return it->second; }
};

template<typename C, typename TR>
class Node {
public:
	typedef std::string KeyType;
private:
	KeyType const name;
public:
	typename map_traits<C,TR>::map_type transitions;
	std::function<Node *(C)> defaultTransition;
	bool final;

	Node(KeyType const & name): 
		name(name),
		final(false)
	{
		defaultTransition = [](C s) -> Node * { return 0; };
	}

	KeyType const & getName() const { return name; }

	Node const * transit(C s) const
	{
//		auto t = transitions.find(s);
		auto t = map_traits<C,TR>::find(transitions,s);
		if( t == transitions.end() ) return defaultTransition(s);
//		auto & transition = t->second;
		auto & transition = map_traits<C,TR>::getValue(t);
		transition(s);
		return transition.node;
	}

	TR & setTrans(C s, Node & node)
	{
		auto & result = transitions[s];
		result.node = &node;
		return result;
	}
};

template<typename C = char, typename TR = BasicTransition<C>, typename N = Node<C,TR>>
class FiniteAutomata {
public:
	typedef N NodeType;
	typedef typename N::KeyType KeyType;
private:
	std::unordered_map<KeyType,N> nodes;
	N const * start;

	// TODO add move semantics
	N & createNode(KeyType const & name)
	{
		auto it = nodes.insert(std::make_pair(name,N(name)));
		return it.first->second;
	}
public:
	N & getNode(KeyType const & name)
	{
		auto it = nodes.find(name);
		if( it == nodes.end() ) return createNode(name);
			
		return it->second;
	}

	void setStart(KeyType const & name)
	{
		start = &getNode(name);
	}

	TR & setTrans(KeyType const & src, C s, KeyType const & dest)
	{
		return getNode(src).setTrans(s,getNode(dest));
	}

	template<typename it>
	bool consume(it begin, it end) const
	{
		auto node = start;
		for( it s = begin; s != end; ++s ) {
			node = node->transit(*s);
			if( ! node ) return false;
		}
		return node->final;
	}
	
	template<typename U>
	bool consume(U const & container) const
	{
		return consume(container.begin(),container.end());
	}

	class Consumer {
		N const * node;
	public:
		Consumer(): node(0) {}
		Consumer(N const * node): node(node) {}

		bool consume(C s)
		{
			//std::cout << "consumindo ch " << s << " node = " << (node ? node->getName() : "nil") << std::endl;
			if( ! node ) return false;
			node = node->transit(s);
			//std::cout << "transited to " << (node ? node->getName() : "nil") << std::endl;
			return node;
		}

		bool fail() const { return ! node; }

		bool final() const { return node && node->final; }
	};

	class output_iterator: public std::iterator<std::output_iterator_tag,C> {
		struct RefProxy {
			Consumer consumer;
			RefProxy(N const * node): consumer(node) {}
			RefProxy & operator=(C s) { consumer.consume(s); return *this; }
		};

		RefProxy refProxy;
	public:
		output_iterator(N const * node): refProxy(node) {}
		output_iterator & operator++() { return *this; }
		RefProxy & operator*() { return refProxy; }
		// explicit operator bool() const { return ! consumer.fail(); }
		operator void const *() const { return refProxy.consumer.fail() ? 0 : this; }
	};

	Consumer getConsumer()	
	{
		return Consumer(start);
	}

	output_iterator output()
	{
		return output_iterator(start);
	}
};


template<typename C>
struct Range {
	C a,b;
	Range(C v): a(v), b(v) {}
	Range(C a, C b): a(a), b(b) {}
	operator C() { assert(a==b); return a; }
};

template<typename C>
bool operator<(Range<C> const & r1, Range<C> const & r2) { return r1.b < r2.a; }
//bool operator<(Range<C> const & r1, Range<C> const & r2) { return r1.a < r2.a && r1.b < r2.b; }
//bool operator<(Range<C> const & r1, Range<C> const & r2) { return r1.a < r2.a && r2.b < r1.b; }

template<typename C, template<class> class TR>
struct TTraits {
	typedef typename TR<Range<C>>::output_type output_type;
	static void setOutput(TR<Range<C>> & tr, output_type const * output)
	{
		if( output != 0 ) {
			tr.output = *output;
		}
	}
};

template<typename C>
struct TTraits<C,BasicTransition> {
	typedef void * output_type;
	static void setOutput(BasicTransition<Range<C>> & tr, output_type const * output) {}
};

template<typename C, template<class> class TR = BasicTransition>
class RangeSetter {
	typedef TR<Range<C>> Transition;
	typedef FiniteAutomata<Range<C>,Transition> Automata;
	Automata & automata;
	char ch1;
	FiniteAutomata<char,MealyTransition<char>> parser;

	void addTrans(typename Automata::KeyType const & src, Range<C> range, typename Automata::KeyType const & dest, typename TTraits<C,TR>::output_type const * output)
	{
		auto & tr = this->automata.setTrans(src,range,dest);
		TTraits<C,TR>::setOutput(tr,output);
	}

	void buildParser(typename Automata::KeyType const & src, typename Automata::KeyType const & dest, typename TTraits<C,TR>::output_type const * output)
	{
		parser.getNode("start").defaultTransition = [this](char ch) -> FiniteAutomata<char,MealyTransition<char>>::NodeType * {
			this->ch1 = ch;
			return &this->parser.getNode("ch1");
		};

		parser.setTrans("ch1",'-',"ch2");
		parser.getNode("ch1").defaultTransition = [=,this](char ch) -> FiniteAutomata<char,MealyTransition<char>>::NodeType * {
			this->addTrans(src,this->ch1,dest,output);
			this->ch1 = ch;
			return &this->parser.getNode("ch1");
		};

		parser.getNode("ch2").defaultTransition = [=,this](char ch) -> FiniteAutomata<char,MealyTransition<char>>::NodeType * {
			this->addTrans(src,Range<C>(this->ch1,ch),dest,output);
			this->ch1 = 0;
			return &this->parser.getNode("start");
		};

		parser.setStart("start");
		parser.getNode("start").final = true;
		parser.getNode("ch1").final = true;
	}

	void do_setTrans(typename Automata::KeyType const & src, std::string const & ranges, typename Automata::KeyType const & dest, typename TTraits<C,TR>::output_type const * output)
	{
		ch1 = 0;
		buildParser(src,dest,output);
		bool consumed = parser.consume(ranges.begin(),ranges.end());	// had to use a separate var due to damned VC++
		assert(consumed);
		
		if( ch1 != 0 ) {	// add empilhado que sobrou
			addTrans(src,ch1,dest,output);
		}
	}
public:
	RangeSetter(Automata & automata): 
		automata(automata)
	{
	}

	void setTrans(typename Automata::KeyType const & src, std::string const & ranges, typename Automata::KeyType const & dest)
	{
		do_setTrans(src,ranges,dest,0);
	}

	void setTrans(typename Automata::KeyType const & src, std::string const & ranges, typename Automata::KeyType const & dest, typename TTraits<C,TR>::output_type const & output)
	{
		do_setTrans(src,ranges,dest,&output);
	}
};

}

#endif
