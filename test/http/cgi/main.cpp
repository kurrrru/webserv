#include "../../../src/http/cgi/cgi_response_parser.hpp"
#include "cgi_response_test.hpp"
#include "cgi_handle_test.hpp"
#include "cgi_execute_test.hpp"

void requestLineTest();
// void testCgiBufSize();
void handleTest();
void executeTest();

int main(void) {
    toolbox::logger::StepMark::setLogFile("cgi_test.log");
    toolbox::logger::StepMark::setLevel(toolbox::logger::DEBUG);
    requestLineTest();
    // testCgiBufSize();
    handleTest();
    executeTest();

    return 0;
}
