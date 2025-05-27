#include <string>
#include <vector>
#include <fstream>
#include <iostream>

#include <sys/stat.h>

#include "../string_utils.hpp"
#include "post_method.hpp"

namespace http {
namespace {

const char* MULTIPART_FORM_DATA = "multipart/form-data";
const char* URL_ENCODED = "application/x-www-form-urlencoded";
const char* CONTENT_DISPOSITION = "Content-Disposition: form-data;";
const char* const BOUNDARY_PREFIX = "boundary=";
const size_t BOUNDARY_PREFIX_LEN = 9;
const char* const FILENAME_PREFIX = "filename=\"";
const size_t FILENAME_PREFIX_LEN = 10;

bool startsWith(const std::string& str, const std::string& prefix) {
    return str.find(prefix) == 0;
}

bool isFileExists(const std::string& name) {
    struct stat buffer;
    return (stat(name.c_str(), &buffer) == 0 && S_ISREG(buffer.st_mode));
}

std::string getTimestamp() {
    return toolbox::to_string(time(NULL));
}

void saveToFile(const std::string& filepath, const std::string& content) {
    std::ofstream ofs(filepath.c_str(), std::ios::binary);
    if (!ofs) {
        toolbox::logger::StepMark::error("runPost: saveToFile failed: " + filepath);
        throw HttpStatus::INTERNAL_SERVER_ERROR;
    }
    ofs.write(content.data(), content.size());
    if (ofs.fail()) {
        toolbox::logger::StepMark::error("runPost: saveToFile failed: " + filepath);
        throw HttpStatus::INTERNAL_SERVER_ERROR;
    }
    toolbox::logger::StepMark::info("runPost: file created: " + filepath);
}

bool isMultipartFormData(HTTPFields::FieldValue& contentType) {
    return !contentType.empty() && startsWith(contentType[0], MULTIPART_FORM_DATA);
}

bool isUrlEncoded(HTTPFields::FieldValue& contentType) {
    return !contentType.empty() && startsWith(contentType[0], URL_ENCODED);
}

std::string getBoundary(HTTPFields& fields) {
    HTTPFields::FieldValue contentType = fields.getFieldValue(fields::CONTENT_TYPE);
    if (contentType.empty()) {
        toolbox::logger::StepMark::error("runPost: getBoundary failed: contentType is empty");
        return "";
    }
    std::string contentTypeStr = contentType[0];
    std::size_t boundaryPos = contentTypeStr.find(BOUNDARY_PREFIX);
    if (boundaryPos == std::string::npos) {
        toolbox::logger::StepMark::error("runPost: getBoundary failed: boundary is not found");
        return "";
    }
    std::string boundaryStr = contentTypeStr.substr(boundaryPos + BOUNDARY_PREFIX_LEN);
    return "--" + boundaryStr;
}

std::string generateUniqueFilename(const std::string& uploadPath, const std::string& baseFilename) {
    std::string filename = baseFilename;
    std::string name, extension;
    std::size_t dotPos = filename.find_last_of('.');

    if (dotPos != std::string::npos) {
        name = filename.substr(0, dotPos);
        extension = filename.substr(dotPos);
    } else {
        name = filename;
        extension = "";
    }

    std::size_t counter = 1;
    while (isFileExists(joinPath(uploadPath, filename))) {
        filename = name + "(" + toolbox::to_string(counter) + ")" + extension;
        counter++;
    }
    return filename;
}

void handleCreateFile(const std::string& uploadPath, std::string filename, const std::string& content, HTTPFields& fields) {
    if (filename.empty()) {
        filename = getTimestamp();
    }
    if (filename.find('.') == std::string::npos) {
        HTTPFields::FieldValue contentType = fields.getFieldValue(fields::CONTENT_TYPE);
        if (!contentType.empty()) {
            std::string extension = ContentTypeManager::getInstance().getExtension(contentType[0]);
            if (!extension.empty()) {
                filename += "." + extension;
            }
        }
    }
    std::string filepath = joinPath(uploadPath, filename);
    if (isFileExists(filepath)) {
        filename = generateUniqueFilename(uploadPath, filename);
        filepath = joinPath(uploadPath, filename);
    }

    saveToFile(filepath, content);
}

void extractMultipartSegments(std::vector<std::string>& bodyParts, const std::string& recvBody, const std::string& boundary) {
    std::string endBoundary = symbols::CRLF + boundary + "--";
    std::size_t boundaryPos = 0;

    boundaryPos = recvBody.find(boundary, boundaryPos);
    boundaryPos += boundary.length();
    while (boundaryPos < recvBody.length()) {
        std::size_t nextBoundaryPos = recvBody.find(symbols::CRLF + boundary, boundaryPos);
        if (nextBoundaryPos == std::string::npos) {
            return;
        }

        std::string part = recvBody.substr(boundaryPos + std::string(symbols::CRLF).length(), nextBoundaryPos - boundaryPos);
        bodyParts.push_back(part);

        if (recvBody.find(endBoundary, nextBoundaryPos) == nextBoundaryPos) {
            break;
        }
        boundaryPos = nextBoundaryPos + std::string(symbols::CRLF).length() + boundary.length();
    }
}

void parseContentDisposition(std::string& bodyPart, FormDataField& formData) {
    std::string attributes = bodyPart.substr(std::string(CONTENT_DISPOSITION).length());
    while (true) {
        std::string keyValue = toolbox::trim(&attributes, symbols::SEMICOLON);
        utils::skipSpace(&keyValue);
        if (keyValue.empty()) {
            break;
        }
        if (keyValue.find(FILENAME_PREFIX) == 0) {
            std::string filename = keyValue.substr(FILENAME_PREFIX_LEN);
            std::size_t pos = filename.find("\"");
            if (pos != std::string::npos) {
                filename.erase(pos);
                formData.filename = filename;
            } else {
                toolbox::logger::StepMark::error("runPost: parseContentDisposition failed: filename is not closed");
            }
        }
    }
}

void extractPartData(std::string& currentLine, FormDataField& formData) {
    while (!currentLine.empty()) {
        std::size_t crlfPos = currentLine.find(symbols::CRLF);
        if (crlfPos == std::string::npos) {
            toolbox::logger::StepMark::error("runPost: handleMultipartFormData failed: unexpected line");
            return;
        }
        std::string line = toolbox::trim(&currentLine, symbols::CRLF);
        if (line.empty()) {
            break;
        }

        if (line.find(CONTENT_DISPOSITION) == 0) {
            parseContentDisposition(line, formData);
        }
    }
}

void parseMultipartFormData(std::string& recvBody, const std::string& boundary, std::vector<FormDataField>& formDataVec) {
    std::vector<std::string> bodyParts;
    extractMultipartSegments(bodyParts, recvBody, boundary);
    formDataVec.reserve(bodyParts.size());
    for (std::size_t i = 0; i < bodyParts.size(); i++) {
        std::string currentLine = bodyParts[i];
        FormDataField formData;

        extractPartData(currentLine, formData);
        formData.content = currentLine.substr(0, currentLine.length() - std::string(symbols::CRLF).length());
        formDataVec.push_back(formData);
    }
}

void connectFormData(std::vector<FormDataField>& formDataVec, std::map<std::string, std::string>& formDataMap) {
    for (std::vector<FormDataField>::iterator it = formDataVec.begin(); it != formDataVec.end(); it++) {
        if (!it->filename.empty()) {
            formDataMap[it->filename] += it->content;
        }
    }
}

void handleMultipartFormData(const std::string& uploadPath, std::string& recvBody, HTTPFields& fields) {
    std::string boundary = getBoundary(fields);
    if (boundary.empty()) {
        toolbox::logger::StepMark::error("runPost: handleMultipartFormData failed: boundary is empty");
        throw HttpStatus::BAD_REQUEST;
    }

    std::vector<FormDataField> formDataVec;
    parseMultipartFormData(recvBody, boundary, formDataVec);

    std::map<std::string, std::string> formDataMap;
    connectFormData(formDataVec, formDataMap);
    for (std::map<std::string, std::string>::iterator it = formDataMap.begin(); it != formDataMap.end(); it++) {
        handleCreateFile(uploadPath, it->first, it->second, fields);
    }
}

void handleUrlEncoded(const std::string& uploadPath, std::string& recvBody, HTTPFields& fields) {
    std::string body;
    while (!recvBody.empty()) {
        std::string keyValue = toolbox::trim(&recvBody, symbols::AMPERSAND);
        std::size_t equalPos = keyValue.find(symbols::EQUAL);
        if (equalPos == std::string::npos) {
            if (!utils::percentDecode(keyValue, &body)) {
                toolbox::logger::StepMark::error("runPost: handleUrlEncoded failed: " + keyValue);
                throw HttpStatus::BAD_REQUEST;
            }
            body += symbols::EQUAL;
            body += symbols::LF;
        } else {
            std::string key = keyValue.substr(0, equalPos);
            std::string value = keyValue.substr(equalPos + 1);
            if (!utils::percentDecode(key, &body)) {
                toolbox::logger::StepMark::error("runPost: handleUrlEncoded failed: " + key);
                throw HttpStatus::BAD_REQUEST;
            }
            body += symbols::EQUAL;
            if (!utils::percentDecode(value, &body)) {
                toolbox::logger::StepMark::error("runPost: handleUrlEncoded failed: " + value);
                throw HttpStatus::BAD_REQUEST;
            }
        }
        body += symbols::LF;
    }
    handleCreateFile(uploadPath, "", body, fields);
}

}  // namespace

void runPost(const std::string& uploadPath, std::string& recvBody, HTTPFields& fields, Response& response) {
    try {
        if (uploadPath.empty()) {
            toolbox::logger::StepMark::error("runPost: uploadPath is empty");
            throw HttpStatus::NOT_IMPLEMENTED;
        }

        HTTPFields::FieldValue contentType = fields.getFieldValue(fields::CONTENT_TYPE);
        if (isMultipartFormData(contentType)) {
            handleMultipartFormData(uploadPath, recvBody, fields);
        } else if (isUrlEncoded(contentType)) {
            handleUrlEncoded(uploadPath, recvBody, fields);
        } else {
            handleCreateFile(uploadPath, "", recvBody, fields);
        }
        response.setStatus(HttpStatus::CREATED);
    } catch (const HttpStatus::EHttpStatus& e) {
        toolbox::logger::StepMark::error("runPost: setStatus " + toolbox::to_string(static_cast<int>(e)));
        response.setStatus(e);
    }
}

}  // namespace http
