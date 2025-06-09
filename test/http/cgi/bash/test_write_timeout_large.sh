#!/bin/bash

# 1.5MBの大容量POSTデータでwriteタイムアウトをテスト
# 一時ファイルを使用して効率的にデータ生成

DATA_SIZE=$((1536 * 1024))  # 1.5MB = 1,572,864 bytes
TEMP_FILE="/tmp/write_timeout_test_data.tmp"

echo "Generating ${DATA_SIZE} bytes of test data for write timeout test..."
echo "Using temporary file for efficient data generation..."

# 一時ファイルを作成
echo -n "WRITE_TIMEOUT_TEST_" > "${TEMP_FILE}"

# 1.5MBまで'W'で埋める
PREFIX_SIZE=$(echo -n "WRITE_TIMEOUT_TEST_" | wc -c)
REMAINING=$((DATA_SIZE - PREFIX_SIZE))

# ddコマンドで効率的にデータを生成
echo "Filling remaining ${REMAINING} bytes with 'W'..."
dd if=/dev/zero bs=1 count=$REMAINING 2>/dev/null | tr '\0' 'W' >> "${TEMP_FILE}"

ACTUAL_SIZE=$(wc -c < "${TEMP_FILE}")
echo "Test data file created. Size: ${ACTUAL_SIZE} bytes"

# POSTリクエストを送信
echo "Sending 1.5MB POST request to write_timeout_test.py..."
echo "CGI script will NOT read stdin, causing pipe buffer to fill up..."
echo "Server should timeout after 30 seconds during write operation..."

curl -v \
  -X POST \
  -H "Content-Type: application/x-www-form-urlencoded" \
  --data-binary "@${TEMP_FILE}" \
  http://localhost:8080/write_timeout_test.py

echo ""
echo "Write timeout test completed."

# 一時ファイルを削除
rm -f "${TEMP_FILE}"
echo "Temporary file cleaned up."