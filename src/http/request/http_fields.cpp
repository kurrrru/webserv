// Copyright 2025 Ideal Broccoli

#include <string>
#include <utility>
#include <map>
#include <vector>

#include "http_fields.hpp"
#include "request_parser.hpp"

bool hasWhiteSpace(const std::string& str) {
    for (std::size_t i = 0; i < str.size(); ++i) {
        if (std::isspace(str[i])) {
            return true;
        }
    }
    return false;
}


bool otherFieldLine() {
    /*
    対象のフィールドのリストに追加する
    */
}

std::string ngx_http_read_request_header() {
    /*
    bufを読み込んで、CRLFが存在する場所までを切り取りreturnする
    */
}

bool ngx_http_process_request_header() {
    /*
    ホストの確認
    content-lengthの数値化
    transfer-encodingの確認
    同時にcontent-lengthを送ってきた場合
    ログを残す
    レスポンスの数値を設定 400
    */
}

bool ngx_http_parse_request_line() {
    /*
    start:
    method:
    uri:
    slash
    schema
    sw_http_09
    */
}

void HTTPFields::initFieldsMap() {
    for (std::size_t i = 0; i < http::fields::FIELD_SIZE; ++i) {
        _fieldsMap.insert(
            std::make_pair(http::fields::FIELDS[i], FieldValue()));
        }
    }

bool HTTPFields::parse_header_line(const FieldPair& pair) {
    if (pair.first.empty() || hasWhiteSpace(pair.first)) {
        // response status 400 すぐに返す
        return false;
    }
    if (pair.first == http::fields::HOST) {
        hostFieldLine(pair);
    } else if (pair.first == http::fields::CONTENT_LENGTH ||
                pair.first == http::fields::TRANSFER_ENCODING) {
        uniqueFieldLine(pair);
    } else {
        nomalFieldLine();
    }
}

bool HTTPFields::hostFieldLine(const FieldPair& pair) {
/*
    ホストフィールドが存在するか確認
        存在する場合
            ログを残す
            レスポンスの数値を設定  400
        存在し無い場合
            値のバリデート
            フィールドに値を追加する
*/
    if (_fieldsMap.find(http::fields::HOST) != _fieldsMap.end()) {
        // ログ記録
        // レスポンス値設定
        return false;
    }
    if (!validateHost(pair.second)) {
        // ログ記録
        return false;
    }
    _fieldsMap[pair.first] = pair.second;
}

bool HTTPFields::validateHost(const FieldValue& values) {
    if (values.empty()) {
        // 値がないログ
        return false;
    }
    if (values.size() != 1) {
        // 値が多すぎるログ
        return false;
    }
    // 追加英数字以外を弾く
    if (http::hasWhiteSpace(values[0])) {
        // 無効な値ログ
        return false;
    }
    return true;
}

bool HTTPFields::uniqueFieldLine(const FieldPair& pair) {
    /*
        対象のフィールドが存在するか確認
            存在する場合
                ログを残す
                レスポンスの数値を設定  400
            存在し無い場合
                フィールドに値を追加する
    */
    FieldMap::iterator field = _fieldsMap.find(pair.first);
    // 対象のフィールドが存在しない場合
    if (field == _fieldsMap.end()) {
        // ログ記録
        return false;  // boolでは情報が足り無いかもしれ無いenum作る？
    }
    // 対象のフィールドが存在し、すでに値が設定されている
    if (!field->second.empty()) {
        // ログ記録
        // レスポンス数値設定 400
        return false;
    }
    field->second = pair.second;
    return true;
}

bool HTTPFields::nomalFieldLine(const FieldPair& pair) {
    /*
    対象のフィールドが存在するか確認
        存在する場合
            追加する
        存在し無い場合
            ログ記録
            無視する
    */
    FieldMap::iterator field = _fieldsMap.find(pair.first);
    // 対象のフィールドが存在しない場合
    if (field == _fieldsMap.end()) {
        // ログ記録
        return false;
    }
    for ()
}

bool HTTPFields::addField(const FieldPair& pair) {
    if (pair.first.empty() || hasWhiteSpace(pair.first)) {
        // response status 400
        return false;
    }
    for (FieldMap::iterator m_it =
            _fieldsMap.begin(); m_it != _fieldsMap.end(); ++m_it) {
        if (toolbox::isEqualIgnoreCase(m_it->first, pair.first)) {
            if ((m_it->first == http::fields::CONTENT_LENGTH ||
                    m_it->first == http::fields::HOST) &&
                    !m_it->second.empty()) {
                // response status 400
                return false;
            }
            if (pair.first == http::fields::HOST &&
                    (pair.second.size() != 1 ||
                        hasWhiteSpace(pair.second[0]))) {
                // response status 400
                return false;
            }
            for (std::size_t i = 0; i < pair.second.size(); ++i) {
                m_it->second.push_back(pair.second[i]);
            }
            if (m_it->first == http::fields::CONTENT_LENGTH) {
                for (std::size_t i = 0; i < m_it->second.size(); ++i) {
                    if (m_it->second[0] != m_it->second[i]) {
                        // response status 400
                        return false;
                    }
                }
            }
            break;
        }
    }
    return true;
}

bool HTTPFields::validateAllFields() {
    if (getFieldValue(http::fields::HOST).empty()) {
        return false;
    }
    if (!getFieldValue(http::fields::CONTENT_LENGTH).empty() &&
            !getFieldValue(http::fields::TRANSFER_ENCODING).empty()) {
                return false;
    }
    for (FieldMap::iterator it = _fieldsMap.begin();
            it != _fieldsMap.end(); ++it) {
        if (!it->second.empty() && !validateField(it->first, it->second)) {
            return false;
        }
    }
    return true;
}


bool HTTPFields::validateField(const std::string& key,
                const std::vector<std::string>& values) {
    if (!isValidFieldKey(key)) {
        // response status 400
        return false;
    }
    if (toolbox::isEqualIgnoreCase(key, http::fields::CONTENT_LENGTH)) {
        // response status 400, 413
        return validateContentLength(values);
    } else if (toolbox::isEqualIgnoreCase(key, http::fields::HOST)) {
        // response status 400
        return validateHost(values);
    }
    return true;
}

bool HTTPFields::validateContentLength(const std::vector<std::string>& values) {
    if (values.empty()) {
        return true;
    }
    for (std::size_t i = 0; i < values.size(); ++i) {
        if (values[0] != values[i]) {
            // response status 400
            return false;
        }
    }
    if (!http::isDigitStr(values[0])) {
        // response status 400
        return false;
    }
    int64_t size = std::strtol(values[0].c_str(), NULL, 10);
    if (size > http::fields::MAX_BODY_SIZE) {
        // response status 413
        return false;
    }
    return true;
}

bool HTTPFields::isValidFieldKey(const std::string& key) {
    if (key.empty()) {
        return false;
    }
    if (hasWhiteSpace(key)) {
        return false;
    }
    return true;
}

std::vector<std::string>& HTTPFields::getFieldValue(const std::string& key) {
    static std::vector<std::string> emptyVector;
    if (_fieldsMap.find(key) != _fieldsMap.end()) {
        return _fieldsMap[key];
    }
    return emptyVector;
}

HTTPFields::FieldMap& HTTPFields::get() {
    return _fieldsMap;
}
