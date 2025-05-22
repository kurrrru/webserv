
#include "../../../toolbox/stepmark.hpp"

int main() {
    toolbox::logger::StepMark::setLevel(toolbox::logger::DEBUG);
    toolbox::logger::StepMark::setLogFile("stepmark.log");

    toolbox::logger::StepMark::debug("This is a debug message.");
    toolbox::logger::StepMark::info("This is an info message.");
    toolbox::logger::StepMark::warning("This is a warning message.");
    toolbox::logger::StepMark::error("This is an error message.");
    toolbox::logger::StepMark::critical("This is a critical message.");

    toolbox::logger::StepMark::setLogFile("no_permission");

    return 0;
}