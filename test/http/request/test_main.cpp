// Copyright 2025 Ideal Broccoli

#include "../../../toolbox/stepmark.hpp"

void requestLineTest();
void fieldTest();

int main(void) {
    toolbox::logger::StepMark::setLogFile("field_test.log");
    toolbox::logger::StepMark::setLevel(toolbox::logger::DEBUG);
    requestLineTest();
    fieldTest();

    return 0;
}
