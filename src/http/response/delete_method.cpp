#include <string>

#include "delete_method.hpp"
namespace http {

namespace {
void handleFile(const std::string& path) {
    if (std::remove(path.c_str()) != 0) {
        toolbox::logger::StepMark::error("runDelete: remove fail "
            + path + " " + toolbox::to_string(HttpStatus::INTERNAL_SERVER_ERROR));
        throw HttpStatus::INTERNAL_SERVER_ERROR;
    }
    toolbox::logger::StepMark::info("runDelete: remove success "
        + path + " " + toolbox::to_string(HttpStatus::NO_CONTENT));
}
}  // namespace

void runDelete(const std::string& path, Response& response) {
    struct stat st;

    try {
        HttpStatus::EHttpStatus status = checkFileAccess(path, st);
        if (status != HttpStatus::OK) {
            toolbox::logger::StepMark::error("runDelete: checkFileAccess fail "
                + path + " " + toolbox::to_string(status));
            throw status;
        }

        if (isDirectory(st)) {
            toolbox::logger::StepMark::error("runDelete: isDirectory "
                + path + " " + toolbox::to_string(HttpStatus::FORBIDDEN));
            throw HttpStatus::FORBIDDEN;
        } else if (isRegularFile(st)) {
            handleFile(path);
        } else {
            toolbox::logger::StepMark::error("runDelete: not a regularfile "
                + path + " " + toolbox::to_string(HttpStatus::INTERNAL_SERVER_ERROR));
            throw HttpStatus::INTERNAL_SERVER_ERROR;
        }
        response.setStatus(HttpStatus::NO_CONTENT);
    } catch (const HttpStatus::EHttpStatus& e) {
        toolbox::logger::StepMark::error("runDelete: set status "
            + toolbox::to_string(e));
        response.setStatus(e);
    }
}

}  // namespace http
