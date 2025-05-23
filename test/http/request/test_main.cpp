// Copyright 2025 Ideal Broccoli

#include "../../../toolbox/stepmark.hpp"

int main(void) {
    toolbox::logger::StepMark::setLogFile("request_test.log");
    toolbox::logger::StepMark::setLevel(toolbox::logger::DEBUG);
    return 0;
}
