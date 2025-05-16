#include <string>
#include <vector>
#include <fstream>
#include <iostream>

#include <sys/stat.h>

#include "../string_utils.hpp"
#include "post_method.hpp"

namespace http {
namespace {
bool startsWith(const std::string& str, const std::string& prefix) {
    return str.find(prefix) == 0;
}

bool isMultipartFormData(HTTPFields& fields) {
    HTTPFields::FieldValue contentType = fields.getFieldValue(fields::CONTENT_TYPE);
    return !contentType.empty() && startsWith(contentType[0], "multipart/form-data");
}

bool isUrlEncoded(HTTPFields& fields) {
    HTTPFields::FieldValue contentType = fields.getFieldValue(fields::CONTENT_TYPE);
    return !contentType.empty() && startsWith(contentType[0], "application/x-www-form-urlencoded");
}

std::string getBoundary(HTTPFields& fields) {
    std::string boundary = fields.getFieldValue(fields::CONTENT_TYPE)[0];
    std::size_t pos = boundary.find("boundary=");
    if (pos == std::string::npos) {
        return "";
    }
    std::string boundaryStr = boundary.substr(pos + 9);  // "boundary=" len = 9
    return "--" + boundaryStr;
}

// void writeToFile(const std::string& filename, const std::string& bodyPart) {
//     std::ofstream ofs(filename.c_str(), std::ios_base::binary | std::ios_base::app);
//     if (!ofs) {
//         toolbox::logger::StepMark::error("runPost: writeToFile failed: " + filename);
//         throw HttpStatus::INTERNAL_SERVER_ERROR;
//     }
//     ofs.write(reinterpret_cast<const char*>(bodyPart.c_str()), bodyPart.size());
// }

std::string getTimestamp() {
    return toolbox::to_string(time(NULL));
}

void splitBodyByBoundary(std::vector<std::string>& bodyParts,
                        std::string& recvBody,
                        const std::string& boundary) {
    std::string endBoundary = boundary + "--";
    std::size_t pos = 0;
    
    while (pos < recvBody.length()) {
        // Find the next boundary
        std::size_t boundaryPos = recvBody.find(boundary, pos);
        if (boundaryPos == std::string::npos) {
            break;
        }
        
        // Skip the boundary and any following newlines
        pos = boundaryPos + boundary.length();
        while (pos < recvBody.length() && 
               (recvBody[pos] == '\r' || recvBody[pos] == '\n')) {
            pos++;
        }
        
        // Find the next boundary or end boundary
        std::size_t nextBoundaryPos = recvBody.find(boundary, pos);
        if (nextBoundaryPos == std::string::npos) {
            nextBoundaryPos = recvBody.find(endBoundary, pos);
            if (nextBoundaryPos == std::string::npos) {
                break;
            }
        }
        
        // Extract the part between boundaries
        std::string part = recvBody.substr(pos, nextBoundaryPos - pos);
        if (!part.empty()) {
            bodyParts.push_back(part);
        }
        
        pos = nextBoundaryPos;
    }
}

}  // namespace

void handleCreateFile(const std::string& uploadPath, std::string filename, const std::string& body) {
    std::cout << "handleCreateFile: " << filename << std::endl;
    if (filename.empty()) {
        filename = getTimestamp();
    }
    std::string filepath = joinPath(uploadPath, filename);
    std::ofstream ofs(filepath.c_str(), std::ios::binary);
    if (!ofs) {
        toolbox::logger::StepMark::error("runPost: handleCreateFile failed: " + filepath);
        throw HttpStatus::INTERNAL_SERVER_ERROR;
    }
    ofs.write(body.data(), body.size());
}

void handleNonExistUploadPath(Response& response) {
    response.setBody("Upload path does not exist");
    response.setStatus(HttpStatus::NOT_IMPLEMENTED);
}

void parseContentDisposition(std::string& bodyPart, FormDataField& formData) {
    std::size_t pos = bodyPart.find("Content-Disposition: form-data;");
    if (pos == std::string::npos) {
        return;
    }
    std::string attributes = bodyPart.substr(pos + 31);  // "Content-Disposition: form-data;" の長さ
    while (true) {
        std::string keyValue = toolbox::trim(&attributes, symbols::SEMICOLON);
        utils::skipSpace(&keyValue);
        if (keyValue.empty()) {
            break;
        }
        if (keyValue.find("filename=\"") == 0) {
            std::string filename = keyValue.substr(10);  // "filename=\"" の長さ
            std::size_t pos = filename.find("\"");
            if (pos != std::string::npos) {
                filename.erase(pos);
                formData.filename = filename;
            } else {
                toolbox::logger::StepMark::error("runPost: parseContentDisposition failed: filename is not closed");
            }
        } else if (keyValue.find("name=\"") == 0) {
            std::string name = keyValue.substr(6);  // "name=\"" の長さ
            std::size_t pos = name.find("\"");
            if (pos != std::string::npos) {
                name.erase(pos);
                formData.name = name;
            } else {
                toolbox::logger::StepMark::error("runPost: parseContentDisposition failed: name is not closed");
            }
        }
    }
}

void parseContentType(std::string& bodyPart, FormDataField& formData) {
    std::size_t pos = bodyPart.find(symbols::COLON);
    std::string contentType = bodyPart.substr(pos + 1);
    if (contentType.empty()) {
        toolbox::logger::StepMark::error("runPost: parseContentType failed: content type is empty");
    } else {
        formData.content_type = contentType;
    }
}

void processMultipartBody(const std::string& uploadPath, std::vector<std::string>& bodyParts, const std::string& boundary) {
    std::string endBoundary = boundary + "--";
    std::size_t index = 0;
    std::map<std::string, std::string> formDataMap;

    while(index < bodyParts.size()) {
        std::string currentLine = bodyParts[index];
        FormDataField formData;

        while(!currentLine.empty()) {
            std::size_t crlfPos = currentLine.find(symbols::CRLF);
            std::size_t lfPos = currentLine.find(symbols::LF);
            std::size_t pos = std::min(crlfPos, lfPos);

            if (pos == std::string::npos) {
                toolbox::logger::StepMark::error("runPost: handleMultipartFormData failed: unexpected line");
                return;
            }

            std::string line = currentLine.substr(0, pos + utils::getLineEndLen(currentLine, pos));
            currentLine = currentLine.substr(pos + utils::getLineEndLen(currentLine, pos));
            // std::cout << "line: " << line << std::endl;
            // std::cout << "currentLine: " << currentLine << std::endl;
            if (line.find("Content-Disposition: form-data;") == 0) {
                parseContentDisposition(line, formData);
            } else if (line.find("Content-Type:") == 0) {
                parseContentType(line, formData);
            } else {  // ""
                if (line == symbols::CRLF || line == symbols::LF) {
                    std::cout << "line is empty" << std::endl;
                    break;
                }
                // error
                toolbox::logger::StepMark::error("runPost: handleMultipartFormData failed: unexpected line");
                return;
            }
        }
        if (formData.filename.empty()) {
            // filenameがない場合にはその塊を無視する? 一旦エラー
            toolbox::logger::StepMark::error("runPost: handleMultipartFormData failed: filename is empty");
            throw HttpStatus::BAD_REQUEST;
        }

        if (!formData.filename.empty()) {
            // std::cout << "formData.filename: " << formData.filename << std::endl;
            // std::cout << "currentLine: " << currentLine << std::endl;
            std::size_t lastLfPos = currentLine.find_last_of(symbols::LF);
            std::size_t lastCrLfPos = currentLine.find_last_of(symbols::CRLF);
            std::size_t lastNewLinePos = std::max(lastLfPos, lastCrLfPos);
            std::string content = currentLine.substr(0,lastNewLinePos - 1);
            // std::cout << "content: " << content << std::endl;
            formDataMap[formData.filename] += content;
        }
        index++;
    }
    for (std::map<std::string, std::string>::iterator it = formDataMap.begin(); it != formDataMap.end(); it++) {
        handleCreateFile(uploadPath, it->first, it->second);
    }
}
 
void handleMultipartFormData(const std::string& uploadPath, std::string& recvBody, HTTPFields& fields) {
    std::string boundary = getBoundary(fields);
    if (boundary.empty()) {
        toolbox::logger::StepMark::error("runPost: handleMultipartFormData failed: boundary is empty");
        throw HttpStatus::BAD_REQUEST;
    }
    std::vector<std::string> bodyParts;
    splitBodyByBoundary(bodyParts, recvBody, boundary);

    processMultipartBody(uploadPath, bodyParts, boundary);
}

void handleUrlEncoded(const std::string& uploadPath, std::string& recvBody) {
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
    handleCreateFile(uploadPath, "", body);
}

void runPost(const std::string& uploadPath, std::string& recvBody, HTTPFields& fields, Response& response) {
    if (uploadPath.empty()) {
        handleNonExistUploadPath(response);
        return;
    }
    try {
        if (isMultipartFormData(fields)) {
            handleMultipartFormData(uploadPath, recvBody, fields);
        } else if (isUrlEncoded(fields)) {
            handleUrlEncoded(uploadPath, recvBody);
        } else {
            handleCreateFile(uploadPath, "", recvBody);
        }
    } catch (const HttpStatus::EHttpStatus& e) {
        toolbox::logger::StepMark::error("runPost: setStatus " + toolbox::to_string(static_cast<int>(e)));
        response.setStatus(e);
    }
}

}  // namespace http
