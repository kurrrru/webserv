#include <string>

#include "delete_method.hpp"
namespace http {

namespace {
HttpStatus::EHttpStatus handleFile(const std::string& path) {
    if (std::remove(path.c_str()) != 0) {
        toolbox::logger::StepMark::error("runDelete: remove fail "
            + path + " " + toolbox::to_string(HttpStatus::INTERNAL_SERVER_ERROR));
        return HttpStatus::INTERNAL_SERVER_ERROR;
    }
    toolbox::logger::StepMark::info("runDelete: remove success "
        + path + " " + toolbox::to_string(HttpStatus::NO_CONTENT));
    return HttpStatus::NO_CONTENT;
}
}  // namespace

void runDelete(const std::string& path, Response& response) {
    struct stat st;

    HttpStatus::EHttpStatus status = checkFileAccess(path, st);
    if (status != HttpStatus::OK) {
        toolbox::logger::StepMark::error("runDelete: checkFileAccess fail "
            + path + " " + toolbox::to_string(status));
        response.setStatus(status);
    } else {
        try {
            if (isDirectory(st)) {
                toolbox::logger::StepMark::error("runDelete: isDirectory "
                    + path + " " + toolbox::to_string(HttpStatus::FORBIDDEN));
                response.setStatus(HttpStatus::FORBIDDEN);
            } else if (isRegularFile(st)) {
                response.setStatus(handleFile(path));
            } else {
                toolbox::logger::StepMark::error("runDelete: not a regularfile "
                    + path + " " + toolbox::to_string(HttpStatus::INTERNAL_SERVER_ERROR));
                response.setStatus(HttpStatus::INTERNAL_SERVER_ERROR);
            }
        } catch (const std::exception& e) {
            response.setStatus(HttpStatus::INTERNAL_SERVER_ERROR);
            toolbox::logger::StepMark::error("runDelete: exception "
                + std::string(e.what()) + " "
                + toolbox::to_string(HttpStatus::INTERNAL_SERVER_ERROR));
        }
    }
}

}  // namespace http
