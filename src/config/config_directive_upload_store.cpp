// Copyright 2025 Ideal Broccoli

#include "config_directive_handler.hpp"
#include "config_namespace.hpp"

#include "../../toolbox/stepmark.hpp"

namespace config {

bool DirectiveParser::parseUploadStoreDirective(const std::vector<std::string>& tokens, size_t* pos, std::string* upload_store) {
    toolbox::logger::StepMark::info("parseUploadStoreDirective called");
    
    if (!upload_store) {
        toolbox::logger::StepMark::error("Invalid upload_store pointer");
        return false;
    }
    
    if (*pos >= tokens.size()) {
        toolbox::logger::StepMark::error("Unexpected end of tokens while parsing upload_store directive");
        return false;
    }
    
    // アップロードディレクトリのパスを読み取る
    std::string path = tokens[(*pos)++];
    
    // パスの検証
    if (path.empty()) {
        toolbox::logger::StepMark::error("Upload store path cannot be empty");
        return false;
    }
    
    // 無効な文字チェック
    if (path.find('\0') != std::string::npos) {
        toolbox::logger::StepMark::error("Upload store path contains null character");
        return false;
    }
    
    // セミコロンチェック
    if (*pos >= tokens.size() || tokens[*pos] != ";") {
        toolbox::logger::StepMark::error("Expected semicolon after upload_store directive");
        return false;
    }
    (*pos)++; // セミコロンをスキップ
    
    // パスを設定
    *upload_store = path;
    toolbox::logger::StepMark::debug("Upload store path set to " + path);
    
    return true;
}

}  // namespace config
