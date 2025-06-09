#include "../../../src/http/cgi/cgi_response_parser.hpp"
#include "cgi_response_test.hpp"

void requestLineTest();
void testCgiBufSize();

int main(void) {
    toolbox::logger::StepMark::setLogFile("cgi_test.log");
    toolbox::logger::StepMark::setLevel(toolbox::logger::DEBUG);
    requestLineTest();
    testCgiBufSize();

    return 0;
}
