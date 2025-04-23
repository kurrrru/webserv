// Copyright 2025 Ideal Broccoli

#include "../../../toolbox/stepmark.hpp"

namespace http {
void fieldTest();
}

int main(void) {
    toolbox::logger::StepMark::setLogFile("field_test.log");
    toolbox::logger::StepMark::setLevel(toolbox::logger::DEBUG);
    http::fieldTest();
    return 0;
}
