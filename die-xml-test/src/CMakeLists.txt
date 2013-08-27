include_directories("${TUT_INCLUDE}")
add_executable(die-xml-test
    main.cpp
    testAutomata.cpp
    testSaxParser.cpp)
target_link_libraries(die-xml-test saxparser)
