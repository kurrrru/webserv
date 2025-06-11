#!/bin/bash

# 64KB (65536 bytes) のテストデータを生成
DATA_SIZE=65536
TEST_DATA=""

echo "Generating ${DATA_SIZE} bytes of test data..."

# "TEST_DATA_" プレフィックスを追加
TEST_DATA="TEST_DATA_"
REMAINING=$((DATA_SIZE - ${#TEST_DATA}))

# 残りのサイズを'A'で埋める
for ((i=0; i<REMAINING; i++)); do
    TEST_DATA="${TEST_DATA}A"
done

echo "Test data generated. Length: ${#TEST_DATA} bytes"

# POSTリクエストを送信
echo "Sending POST request to webserv..."
curl -v \
  -X POST \
  -H "Content-Type: application/x-www-form-urlencoded" \
  -d "${TEST_DATA}" \
  http://localhost:8080/large_post_test.py

echo ""
echo "POST test completed."