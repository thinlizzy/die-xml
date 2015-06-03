Welcome to die-xml, which stands for Diego's XML Parser.

This project is a simple XML SAX parser that uses a finite automaton as the backend.

*** HOW TO BUILD ***

This project uses CMake as the meta-build system to support a variety of
target platforms.  You will first need to download and install CMake for your
platform from http://www.cmake.org/.  For Ubuntu/Debian users, you can type:
    sudo apt-get install cmake

The recommended use of CMake is to create out-of-source builds.  First, 
launch cmake-gui, select this source directory, select a new build directory and
then click the "configure" button.  Select the desired build system and click
"ok".  Once this completes successfully, click "configure" again.  Finally,
click "generate" to create your build system.  You now have a working build
system for your platform!

For linux systems, you can now make the build directory.  For Windows systems,
you can open the die_xml.sln file in the build directory.

Testcases with some usage examples are presented in the die-xml-test directory, which can be built in the same way.

Enjoy!
