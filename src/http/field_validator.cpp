

void BaseParser::validateFieldLine(std::string& line) {
    if (hasCtlChar(line) || line.size() > fields::MAX_FIELDLINE_SIZE ||
        line.find(symbols::COLON) == std::string::npos) {
        return handleInvalidFieldLine();
    }
    return true;
}