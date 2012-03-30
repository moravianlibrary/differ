#include <stack>
#include <string>
#include <iostream>
#include <fstream>

namespace std {

class xml_writer_t {
   protected:
      ostream* out;
      stack<string> elements;
   public:
      xml_writer_t(ostream* os):out(os) { *os << "<?xml version=\"1.0\" encoding=\"UTF-8\" ?>" << endl; }
      xml_writer_t* xsl_instruction(string href);
      xml_writer_t* push(string element, string text="");
      xml_writer_t* push(string element, string attrs[][2], int count, string text="");
      xml_writer_t* pop();
};

}
