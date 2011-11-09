#include <tut.h>

#include "../../util/automata.h"

#include <string>
#include <iostream>
#include <sstream>
#include <algorithm>

namespace {
    struct setup {
    };
}

namespace tut { 
    typedef test_group<setup> tg;
    tg automata_test_group("automata");
    
    typedef tg::object testobject;
    
    template<> 
    template<> 
    void testobject::test<1>() 
    {
        set_test_name("finite aaabbbb");

		automata::FiniteAutomata<char> automata;

		automata.setTrans("a",'a',"a");
		automata.setTrans("a",'b',"b");
		automata.setTrans("b",'b',"b");
		automata.setStart("a");
		automata.getNode("b").final = true;

		std::string input = "aaabbbb";
		ensure(automata.consume(input.begin(),input.end()));
		ensure(automata.consume(std::string("aaaaab")));
		ensure(! automata.consume(std::string("aaaaa")));
    }

    template<> 
    template<> 
    void testobject::test<2>() 
    {
        set_test_name("mealy machine");

		automata::FiniteAutomata<char,automata::MealyTransition<char>> mealy;

		std::string out;
		mealy.setTrans("start",'a',"start").output = [&](char s){ out += char('b'-s+'a'); };
		mealy.setTrans("start",'b',"2nd").output = [&](char s){ out += char('b'-s+'a'); };
		mealy.setTrans("2nd",'a',"start").output = [&](char s){ out += char('b'-s+'a' + 2); };
		mealy.setTrans("2nd",'b',"start").output = [&](char s){ out += char('b'-s+'a' + 2); };

		mealy.setStart("start");
		mealy.getNode("2nd").final = true;
		ensure(mealy.consume(std::string("aaaaabaab")));
		ensure_equals(out,"bbbbbadba");
		out.clear();
		ensure(! mealy.consume(std::string("aaaaaba")));
		ensure_equals(out,"bbbbbad");
	}

    template<> 
    template<> 
    void testobject::test<3>() 
    {
        set_test_name("ranges");

		using namespace automata;

		FiniteAutomata<Range<char>> automata;

		automata.setTrans("a",Range<char>('a','b'),"a");
		automata.setTrans("a",'c',"b");
		automata.setTrans("b",'b',"b");
		automata.setStart("a");
		automata.getNode("b").final = true;

		ensure(automata.consume(std::string("aabbaaabcbbbb")));
		ensure(! automata.consume(std::string("aaaaaba")));
		ensure(automata.consume(std::string("aaaaabac")));
	}

    template<> 
    template<> 
    void testobject::test<4>() 
    {
        set_test_name("Consumers and streams");

		using namespace automata;

		automata::FiniteAutomata<char> automata;

		automata.setTrans("a",'a',"a");
		automata.setTrans("a",'b',"b");
		automata.setTrans("b",'b',"b");
		automata.setStart("a");
		automata.getNode("b").final = true;

		std::istringstream iss("aaabbbb");
		auto consumer = automata.getConsumer();
		char ch;
		while( iss.get(ch) ) {
			ensure(consumer.consume(ch));
		}
		ensure(consumer.final());
	}

    template<> 
    template<> 
    void testobject::test<5>() 
    {
        set_test_name("output iterator");

		using namespace automata;

		automata::FiniteAutomata<char> automata;

		automata.setTrans("a",'a',"a");
		automata.setTrans("a",'b',"b");
		automata.setTrans("b",'b',"b");
		automata.setStart("a");
		automata.getNode("b").final = true;

		std::string input = "aaabbbb";
		auto outIt = automata.output();
		std::copy(input.begin(),input.end(),outIt);
		ensure(outIt);
	}

    template<> 
    template<> 
    void testobject::test<6>() 
    {
        set_test_name("default transition");

		using namespace automata;

		FiniteAutomata<char> automata1;

		automata1.setTrans("start",'<',"tagStart");
		automata1.setTrans("tagStart",'>',"end");
		typedef decltype(automata1) t;
		automata1.getNode("tagStart").defaultTransition = [&automata1](char ch) -> t::NodeType * { 
			return ch >= 'a' && ch <= 'z' ? &automata1.getNode("tagStart") : 0; 
		};

		automata1.setStart("start");
		automata1.getNode("end").final = true;

		std::string input = "<teste>";

		auto outIt = automata1.output();
		outIt = std::copy(input.begin(),input.end(),outIt);
		ensure(outIt);

		input = "<te3s31te>";
		outIt = automata1.output();
		outIt = std::copy(input.begin(),input.end(),outIt);
		ensure(! outIt);
	}

    template<> 
    template<> 
    void testobject::test<7>() 
    {
        set_test_name("range setter");

		using namespace automata;

		FiniteAutomata<Range<char>> automata1;
		RangeSetter<char> rs(automata1);
		rs.setTrans("start","abc","start");
		rs.setTrans("start","1-9","end");
		rs.setTrans("end","1-9","end");

		automata1.setStart("start");
		automata1.getNode("end").final = true;

		std::string input = "acacab1323";
		auto outIt = automata1.output();
		outIt = std::copy(input.begin(),input.end(),outIt);
		ensure(outIt);
	}

    template<> 
    template<> 
    void testobject::test<8>() 
    {
        set_test_name("range setter with output");

		using namespace automata;

		int antes = 0;
		int depois = 0;

		FiniteAutomata<Range<char>,MealyTransition<Range<char>>> automata1;
		RangeSetter<char,MealyTransition> rs(automata1);
		rs.setTrans("start","abc","start",[&](char ch){ ++antes; });
		rs.setTrans("start","1-9","num");
		rs.setTrans("num","1-9","num");
		rs.setTrans("num","abc","end",[&](char ch){ ++depois; });
		rs.setTrans("end","abc","end",[&](char ch){ ++depois; });

		automata1.setStart("start");
		automata1.getNode("end").final = true;

		std::string input = "acacab132abcab";
		auto outIt = automata1.output();
		outIt = std::copy(input.begin(),input.end(),outIt);
		ensure(outIt);
		ensure_equals(antes,6);
		ensure_equals(depois,5);
	}

}
