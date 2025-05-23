// Copyright 2025 Ideal Broccoli

#include "../../../toolbox/stepmark.hpp"

void requestLineTest();
void fieldTest();
void testRequestParser();

int main(void) {
    toolbox::logger::StepMark::setLogFile("request_test.log");
    toolbox::logger::StepMark::setLevel(toolbox::logger::DEBUG);
    requestLineTest();
    fieldTest();
    testRequestParser();

    return 0;
}
