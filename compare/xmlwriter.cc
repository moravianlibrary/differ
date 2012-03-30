#include <stack>
#include <string>
#include <iostream>
#include <fstream>

#include "xmlwriter.h"

using namespace std;

xml_writer_t* xml_writer_t::push(string element, string text) {
   *out << "<" << element << ">" << text;
   elements.push(element);
   return this;
}

xml_writer_t* xml_writer_t::pop() { 
   *out << "</" << elements.top() << ">" << endl;
   elements.pop();
   return this;
}

xml_writer_t* xml_writer_t::push(string element, string attrs[][2], int count, string text) {
   *out << "<" << element;
   for (int i = 0; i <= count; i++) {
      string attr[2] = attrs[i];
      *out << " " << attr[0] << "=\"" << attr[1] << "\"";
   }
   *out << ">" << text;
   elements.push(element);
   return this;
}

xml_writer_t* xml_writer_t::xsl_instruction(string href) {
   *out << "<?xml-stylesheet type=\"text/xsl\" href=\"" << href << "\"?>" << endl;
}

#ifdef DEBUG
int main(int argc, char* argv[]) {
   ofstream os(argv[1]);
   xml_writer_t* writer = new xml_writer_t(&os);
   writer->push("root", "hello, world!");
   string foo[2][2] = { {string("one"), string("two")}, {string("one"), string("two")} };
   writer->push("test", foo, 1)->pop();
   writer->pop();
}
#endif
