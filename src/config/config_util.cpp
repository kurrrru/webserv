#include <string>
#include <limits>
#include <algorithm>

#include <sys/types.h>

#include "config.hpp"
#include "config_util.hpp"
#include "config_namespace.hpp"

#include "../../toolbox/stepmark.hpp"

namespace config {

bool isCaseInsensitiveIdentical(const std::string& str1, const std::string& str2) {
    if (str1.size() != str2.size()) {
        return false;
    }
    for (std::size_t i = 0; i < str1.size(); ++i) {
        char c1 = std::tolower(static_cast<unsigned char>(str1[i]));
        char c2 = std::tolower(static_cast<unsigned char>(str2[i]));
        if (c1 != c2) {
            return false;
        }
    }
    return true;
}

bool stringToSizeT(const std::string& str, std::size_t* result) {
    if (!result) {
        toolbox::logger::StepMark::error("Invalid result pointer");
        return false;
    }
    if (str.empty()) {
        toolbox::logger::StepMark::error("Empty numeric value");
        return false;
    }
    // off_t is max
    const std::size_t max_value = std::numeric_limits<off_t>::max();
    const std::size_t cutoff = max_value / 10;
    const std::size_t cutlim = max_value % 10;
    std::size_t value = 0;
    for (std::size_t i = 0; i < str.size(); i++) {
        char c = str[i];
        if (!std::isdigit(c)) {
            toolbox::logger::StepMark::debug("Invalid character in numeric value: " + str);
            return false;
        }
        if (value > cutoff || (value == cutoff && static_cast<std::size_t>(c - '0') > cutlim)) {
            toolbox::logger::StepMark::error("Numeric value too large: " + str);
            return false;
        }
        value = value * 10 + (c - '0');
    }
    *result = value;
    return true;
}

int pathCmp(const std::string& s1, const std::string& s2) {
    if (s1.length() > s2.length()) {
        return 0;
    }
    std::size_t n = s1.length();
    for (std::size_t i = 0; i < n; ++i) {
        char c1 = s1[i];
        char c2 = s2[i];
        if (c1 == c2) {
            if (c1 == '\0') {
                return 0;
            }
            continue;
        }
        return static_cast<int>(c1) - static_cast<int>(c2);
    }
    return 0;
}

void throwConfigError(const std::string& message) {
    toolbox::logger::StepMark::error(message);
    throw ConfigException(message);
}

bool isInAllowedTokens(const std::string& token, const std::string allowedTokens[], std::size_t tokenCount) {
    for (std::size_t i = 0; i < tokenCount; i++) {
        if (token == allowedTokens[i]) {
            return true;
        }
    }
    return false;
}

bool isContextToken(const std::string& token) {
    const std::string contexts[] = {
        config::context::HTTP,
        config::context::SERVER,
        config::context::LOCATION
    };
    const std::size_t contextCount = sizeof(contexts) / sizeof(contexts[0]);
    return isInAllowedTokens(token, contexts, contextCount);
}

bool isDirectiveToken(const std::string& token) {
    const std::string directives[] = {
        config::directive::ALLOWED_METHODS,
        config::directive::AUTOINDEX,
        config::directive::CGI_EXTENSION,
        config::directive::CGI_PATH,
        config::directive::CLIENT_MAX_BODY_SIZE,
        config::directive::ERROR_PAGE,
        config::directive::INDEX,
        config::directive::LISTEN,
        config::directive::RETURN,
        config::directive::ROOT,
        config::directive::SERVER_NAME,
        config::directive::UPLOAD_STORE
    };
    const std::size_t directiveCount = sizeof(directives) / sizeof(directives[0]);
    return isInAllowedTokens(token, directives, directiveCount);
}

}  // namespace config
