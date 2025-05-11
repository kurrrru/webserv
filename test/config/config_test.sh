#!/bin/bash

GREEN='\033[0;32m'
RED='\033[0;31m'
YELLOW='\033[0;33m'
NC='\033[0m'

EXECUTABLE="./config_test.out"

DIRECTIVE_DIR="../../conf/test/directives"
ALLOWED_METHODS_DIR="$DIRECTIVE_DIR/allowed_methods"
AUTOINDEX_DIR="$DIRECTIVE_DIR/autoindex"
CGI_EXTENSION_DIR="$DIRECTIVE_DIR/cgi_extension"
CGI_PASS_DIR="$DIRECTIVE_DIR/cgi_pass"
CLIENT_MAX_BODY_SIZE_DIR="$DIRECTIVE_DIR/client_max_body_size"
ERROR_PAGE_DIR="$DIRECTIVE_DIR/error_page"
INDEX_DIR="$DIRECTIVE_DIR/index"
LISTEN_DIR="$DIRECTIVE_DIR/listen"
RETURN_DIR="$DIRECTIVE_DIR/return"
ROOT_DIR="$DIRECTIVE_DIR/root"
SERVER_NAME_DIR="$DIRECTIVE_DIR/server_name"
UPLOAD_STORE_DIR="$DIRECTIVE_DIR/upload_store"
DIRECTIVE_LOG="config_test_directive.log"

SYNTAX_DIR="../../conf/test/syntax"
BLOCK_TEST_DIR="$SYNTAX_DIR/block"
BRACE_TEST_DIR="$SYNTAX_DIR/brace"
COMMENT_TEST_DIR="$SYNTAX_DIR/comment"
CONTEXT_TEST_DIR="$SYNTAX_DIR/context"
ESCAPE_SEQ_TEST_DIR="$SYNTAX_DIR/escape_sequence"
NEWLINE_TEST_DIR="$SYNTAX_DIR/newline"
QUOTE_TEST_DIR="$SYNTAX_DIR/quote"
SEMICOLON_TEST_DIR="$SYNTAX_DIR/semicolon"
WHITESPACE_TEST_DIR="$SYNTAX_DIR/whitespace"

DIRECTIVE_LOG="config_test_directive.log"
SYNTAX_LOG="config_test_syntax.log"

directives=("allowed_methods" "autoindex" "cgi_extension" "cgi_pass" "client_max_body_size" "error_page" "index" "listen" "return" "root" "server_name" "upload_store")

echo -e "${YELLOW}config test starting...${NC}"

run_test() {
    local file=$1
    local expected_result=$2
    local test_type=$3
    
    echo -n "- $(basename "$file") ... "
    
    if [[ " ${directives[@]} " =~ " $test_type " ]]; then
        echo "--- "$test_type" :$(basename "$file") ---" >> $DIRECTIVE_LOG
        $EXECUTABLE "$file" "$test_type" >> $DIRECTIVE_LOG 2>&1
    else
        echo "--- "$test_type" :$(basename "$file") ---" >> $SYNTAX_LOG
        $EXECUTABLE "$file" >> $SYNTAX_LOG 2>&1
    fi
    RESULT=$?
    
    if [ $expected_result -eq 0 ]; then
        if [ $RESULT -eq 0 ]; then
            echo -e "${GREEN}success${NC}"
        else
            echo -e "${RED}fail${NC}"
            return 1
        fi
    else
        if [ $RESULT -ne 0 ]; then
            echo -e "${GREEN}success${NC}"
        else
            echo -e "${RED}fail${NC}"
            return 1
        fi
    fi
    
    return 0
}

run_tests_in_directory() {
    local directory=$1          
    local file_prefix=$2       
    local expected_result=$3
    local test_type=$4

    local files=("$directory/$file_prefix"*.conf)
    
    for file in "${files[@]}"; do
        if [ -f "$file" ]; then
            ((total_tests++))
            run_test "$file" $expected_result $test_type
            if [ $? -eq 0 ]; then
                ((passed_tests++))
            fi
        fi
    done
}

total_tests=0
passed_tests=0

echo -e "\n${YELLOW}allowed_methods test:${NC}"
run_tests_in_directory "$ALLOWED_METHODS_DIR" "valid_" 0 "allowed_methods"
run_tests_in_directory "$ALLOWED_METHODS_DIR" "invalid_" 1 "allowed_methods"

echo -e "\n${YELLOW}autoindex test:${NC}"
run_tests_in_directory "$AUTOINDEX_DIR" "valid_" 0 "autoindex"
run_tests_in_directory "$AUTOINDEX_DIR" "invalid_" 1 "autoindex"

echo -e "\n${YELLOW}cgi_extension test:${NC}"
run_tests_in_directory "$CGI_EXTENSION_DIR" "valid_" 0 "cgi_extension"
run_tests_in_directory "$CGI_EXTENSION_DIR" "invalid_" 1 "cgi_extension"

echo -e "\n${YELLOW}cgi_pass test:${NC}"
run_tests_in_directory "$CGI_PASS_DIR" "valid_" 0 "cgi_pass"
run_tests_in_directory "$CGI_PASS_DIR" "invalid_" 1 "cgi_pass"

echo -e "\n${YELLOW}client_max_body_size test:${NC}"
run_tests_in_directory "$CLIENT_MAX_BODY_SIZE_DIR" "valid_" 0 "client_max_body_size"
run_tests_in_directory "$CLIENT_MAX_BODY_SIZE_DIR" "invalid_" 1 "client_max_body_size"

echo -e "\n${YELLOW}error_page test:${NC}"
run_tests_in_directory "$ERROR_PAGE_DIR" "valid_" 0 "error_page"
run_tests_in_directory "$ERROR_PAGE_DIR" "invalid_" 1 "error_page"

echo -e "\n${YELLOW}index test:${NC}"
run_tests_in_directory "$INDEX_DIR" "valid_" 0 "index"
run_tests_in_directory "$INDEX_DIR" "invalid_" 1 "index"

echo -e "\n${YELLOW}listen test:${NC}"
run_tests_in_directory "$LISTEN_DIR" "valid_" 0 "listen"
run_tests_in_directory "$LISTEN_DIR" "invalid_" 1 "listen"

echo -e "\n${YELLOW}return test:${NC}"
run_tests_in_directory "$RETURN_DIR" "valid_" 0 "return"
run_tests_in_directory "$RETURN_DIR" "invalid_" 1 "return"

echo -e "\n${YELLOW}root test:${NC}"
run_tests_in_directory "$ROOT_DIR" "valid_" 0 "root"
run_tests_in_directory "$ROOT_DIR" "invalid_" 1 "root"

echo -e "\n${YELLOW}server_name test:${NC}"
run_tests_in_directory "$SERVER_NAME_DIR" "valid_" 0 "server_name"
run_tests_in_directory "$SERVER_NAME_DIR" "invalid_" 1 "server_name"

echo -e "\n${YELLOW}upload_store test:${NC}"
run_tests_in_directory "$UPLOAD_STORE_DIR" "valid_" 0 "upload_store"
run_tests_in_directory "$UPLOAD_STORE_DIR" "invalid_" 1 "upload_store"

echo -e "\n${YELLOW}block test:${NC}"
run_tests_in_directory "$BLOCK_TEST_DIR" "valid_" 0 "block"
run_tests_in_directory "$BLOCK_TEST_DIR" "invalid_" 1 "block"

echo -e "\n${YELLOW}brace test:${NC}"
run_tests_in_directory "$BRACE_TEST_DIR" "valid_" 0 "brace"
run_tests_in_directory "$BRACE_TEST_DIR" "invalid_" 1 "brace"

echo -e "\n${YELLOW}comment test:${NC}"
run_tests_in_directory "$COMMENT_TEST_DIR" "valid_" 0 "comment"
run_tests_in_directory "$COMMENT_TEST_DIR" "invalid_" 1 "comment"

echo -e "\n${YELLOW}context test:${NC}"
run_tests_in_directory "$CONTEXT_TEST_DIR" "valid_" 0 "context"
run_tests_in_directory "$CONTEXT_TEST_DIR" "invalid_" 1 "context"

echo -e "\n${YELLOW}escape_sequence test:${NC}"
run_tests_in_directory "$ESCAPE_SEQ_TEST_DIR" "valid_" 0 "escape_sequence"
run_tests_in_directory "$ESCAPE_SEQ_TEST_DIR" "invalid_" 1 "escape_sequence"

echo -e "\n${YELLOW}newline test:${NC}"
run_tests_in_directory "$NEWLINE_TEST_DIR" "valid_" 0 "newline"
run_tests_in_directory "$NEWLINE_TEST_DIR" "invalid_" 1 "newline"

echo -e "\n${YELLOW}quote test:${NC}"
run_tests_in_directory "$QUOTE_TEST_DIR" "valid_" 0 "quote"
run_tests_in_directory "$QUOTE_TEST_DIR" "invalid_" 1 "quote"

echo -e "\n${YELLOW}semicolon test:${NC}"
run_tests_in_directory "$SEMICOLON_TEST_DIR" "valid_" 0 "semicolon"
run_tests_in_directory "$SEMICOLON_TEST_DIR" "invalid_" 1 "semicolon"

echo -e "\n${YELLOW}whitespace test:${NC}"
run_tests_in_directory "$WHITESPACE_TEST_DIR" "valid_" 0 "whitespace"
run_tests_in_directory "$WHITESPACE_TEST_DIR" "invalid_" 1 "whitespace"

echo -e "\n${YELLOW}result:${NC}"
echo "total: $total_tests"
echo "success: $passed_tests"
if [ $passed_tests -eq $total_tests ]; then
    echo -e "${GREEN}All tests were successful${NC}"
else
    echo -e "${RED}$((total_tests - passed_tests)) That test failed${NC}"
fi

exit 0
