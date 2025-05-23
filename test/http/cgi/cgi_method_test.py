#!/usr/bin/env python3

import os
import unittest
import subprocess
import datetime
import random
import string
import tempfile

def generateRandomString(length):
    return ''.join(random.choices(string.ascii_letters + string.digits, k=length))

class TestCgiAdditional(unittest.TestCase):
    def setUp(self):
        self.uploadDir = "./uploads/"
        self.cgiPath = "../../../cgi-bin/script.py"
        self.randomString = generateRandomString(30)
        self.randomStringLength = len(self.randomString)
        os.makedirs(self.uploadDir, exist_ok=True)

    def tearDown(self):
        for file in os.listdir(self.uploadDir):
            try:
                os.remove(os.path.join(self.uploadDir, file))
            except:
                pass
        try:
            os.rmdir(self.uploadDir)
        except:
            pass

    def testFileSizeLimit(self):
        """ファイルサイズ制限テスト（10MB超過）"""
        # 10MB + 1バイトのデータを作成
        large_data = b'x' * (10 * 1024 * 1024 + 1)

        env = {
            "REQUEST_METHOD": "POST",
            "QUERY_STRING": "filename=large_file.txt",
            "CONTENT_LENGTH": str(len(large_data)),
            "CONTENT_TYPE": "application/octet-stream",
            "UPLOAD_DIR": self.uploadDir,
        }

        process = subprocess.Popen(
            [self.cgiPath],
            env=env,
            stdin=subprocess.PIPE,
            stdout=subprocess.PIPE,
            stderr=subprocess.PIPE
        )

        stdout, _ = process.communicate(large_data)
        response = stdout.decode('utf-8')
        headers, body = response.split('\n\n', 1)

        self.assertIn('Status: 413', headers)
        self.assertIn('File too large', body)

    def testEmptyContent(self):
        """空のコンテンツアップロードテスト"""
        env = {
            "REQUEST_METHOD": "POST",
            "QUERY_STRING": "filename=empty.txt",
            "CONTENT_LENGTH": "0",
            "CONTENT_TYPE": "application/octet-stream",
            "UPLOAD_DIR": self.uploadDir,
        }

        process = subprocess.Popen(
            [self.cgiPath],
            env=env,
            stdin=subprocess.PIPE,
            stdout=subprocess.PIPE,
            stderr=subprocess.PIPE
        )

        stdout, _ = process.communicate(b'')
        response = stdout.decode('utf-8')
        headers, body = response.split('\n\n', 1)

        self.assertIn('Status: 400', headers)
        self.assertIn('No content to upload', body)

    def testLongFilename(self):
        """長すぎるファイル名テスト（255文字超過）"""
        long_filename = 'a' * 256 + '.txt'

        env = {
            "REQUEST_METHOD": "POST",
            "QUERY_STRING": f"filename={long_filename}",
            "CONTENT_LENGTH": str(self.randomStringLength),
            "CONTENT_TYPE": "application/octet-stream",
            "UPLOAD_DIR": self.uploadDir,
        }

        process = subprocess.Popen(
            [self.cgiPath],
            env=env,
            stdin=subprocess.PIPE,
            stdout=subprocess.PIPE,
            stderr=subprocess.PIPE
        )

        stdout, _ = process.communicate(self.randomString.encode('utf-8'))
        response = stdout.decode('utf-8')
        headers, body = response.split('\n\n', 1)

        self.assertIn('Status: 400', headers)
        self.assertIn('Invalid filename', body)

    def testEmptyFilename(self):
        """空のファイル名テスト"""
        env = {
            "REQUEST_METHOD": "POST",
            "QUERY_STRING": "filename=",
            "CONTENT_LENGTH": str(self.randomStringLength),
            "CONTENT_TYPE": "application/octet-stream",
            "UPLOAD_DIR": self.uploadDir,
        }

        process = subprocess.Popen(
            [self.cgiPath],
            env=env,
            stdin=subprocess.PIPE,
            stdout=subprocess.PIPE,
            stderr=subprocess.PIPE
        )

        stdout, _ = process.communicate(self.randomString.encode('utf-8'))
        response = stdout.decode('utf-8')
        headers, body = response.split('\n\n', 1)

        self.assertIn('Status: 201', headers)
        self.assertIn('Location: /uploads/uploaded_file', headers)

    def testMultipartFormData(self):
        """multipart/form-dataアップロードテスト"""
        boundary = "----WebKitFormBoundary7MA4YWxkTrZu0gW"
        multipart_data = (
            f"--{boundary}\r\n"
            f"Content-Disposition: form-data; name=\"file\"; filename=\"test.txt\"\r\n"
            f"Content-Type: text/plain\r\n\r\n"
            f"{self.randomString}\r\n"
            f"--{boundary}--\r\n"
        ).encode('utf-8')

        env = {
            "REQUEST_METHOD": "POST",
            "QUERY_STRING": "",
            "CONTENT_LENGTH": str(len(multipart_data)),
            "CONTENT_TYPE": f"multipart/form-data; boundary={boundary}",
            "UPLOAD_DIR": self.uploadDir,
        }

        process = subprocess.Popen(
            [self.cgiPath],
            env=env,
            stdin=subprocess.PIPE,
            stdout=subprocess.PIPE,
            stderr=subprocess.PIPE
        )

        stdout, _ = process.communicate(multipart_data)
        response = stdout.decode('utf-8')
        headers, body = response.split('\n\n', 1)

        self.assertIn('Status: 201', headers)
        self.assertIn('Files uploaded', body)
        self.assertIn('test.txt', body)

    def testMultipartMultipleFiles(self):
        """複数ファイルのmultipart/form-dataアップロードテスト"""
        boundary = "----WebKitFormBoundary7MA4YWxkTrZu0gW"
        multipart_data = (
            f"--{boundary}\r\n"
            f"Content-Disposition: form-data; name=\"file1\"; filename=\"test1.txt\"\r\n"
            f"Content-Type: text/plain\r\n\r\n"
            f"File 1 content\r\n"
            f"--{boundary}\r\n"
            f"Content-Disposition: form-data; name=\"file2\"; filename=\"test2.txt\"\r\n"
            f"Content-Type: text/plain\r\n\r\n"
            f"File 2 content\r\n"
            f"--{boundary}--\r\n"
        ).encode('utf-8')

        env = {
            "REQUEST_METHOD": "POST",
            "QUERY_STRING": "",
            "CONTENT_LENGTH": str(len(multipart_data)),
            "CONTENT_TYPE": f"multipart/form-data; boundary={boundary}",
            "UPLOAD_DIR": self.uploadDir,
        }

        process = subprocess.Popen(
            [self.cgiPath],
            env=env,
            stdin=subprocess.PIPE,
            stdout=subprocess.PIPE,
            stderr=subprocess.PIPE
        )

        stdout, _ = process.communicate(multipart_data)
        response = stdout.decode('utf-8')
        headers, body = response.split('\n\n', 1)

        self.assertIn('Status: 201', headers)
        self.assertIn('Files uploaded', body)
        self.assertIn('test1.txt', body)
        self.assertIn('test2.txt', body)

    def testMultipartNoFiles(self):
        """ファイルなしのmultipart/form-dataテスト"""
        boundary = "----WebKitFormBoundary7MA4YWxkTrZu0gW"
        multipart_data = (
            f"--{boundary}\r\n"
            f"Content-Disposition: form-data; name=\"text_field\"\r\n\r\n"
            f"some text value\r\n"
            f"--{boundary}--\r\n"
        ).encode('utf-8')

        env = {
            "REQUEST_METHOD": "POST",
            "QUERY_STRING": "",
            "CONTENT_LENGTH": str(len(multipart_data)),
            "CONTENT_TYPE": f"multipart/form-data; boundary={boundary}",
            "UPLOAD_DIR": self.uploadDir,
        }

        process = subprocess.Popen(
            [self.cgiPath],
            env=env,
            stdin=subprocess.PIPE,
            stdout=subprocess.PIPE,
            stderr=subprocess.PIPE
        )

        stdout, _ = process.communicate(multipart_data)
        response = stdout.decode('utf-8')
        headers, body = response.split('\n\n', 1)

        self.assertIn('Status: 400', headers)
        self.assertIn('No files uploaded', body)

    def testUnsupportedMethod(self):
        """サポートされていないHTTPメソッドテスト"""
        env = {
            "REQUEST_METHOD": "PUT",
            "QUERY_STRING": "",
            "CONTENT_LENGTH": "0",
            "CONTENT_TYPE": "application/octet-stream",
            "UPLOAD_DIR": self.uploadDir,
        }

        process = subprocess.Popen(
            [self.cgiPath],
            env=env,
            stdin=subprocess.PIPE,
            stdout=subprocess.PIPE,
            stderr=subprocess.PIPE
        )

        stdout, _ = process.communicate(b'')
        response = stdout.decode('utf-8')
        headers, body = response.split('\n\n', 1)

        self.assertIn('Status: 501', headers)
        self.assertIn('Not Implemented', body)

    def testJapaneseFilename(self):
        """日本語ファイル名テスト"""
        japanese_filename = "テスト.txt"
        encoded_filename = japanese_filename

        env = {
            "REQUEST_METHOD": "POST",
            "QUERY_STRING": f"filename={encoded_filename}",
            "CONTENT_LENGTH": str(self.randomStringLength),
            "CONTENT_TYPE": "application/octet-stream",
            "UPLOAD_DIR": self.uploadDir,
        }

        process = subprocess.Popen(
            [self.cgiPath],
            env=env,
            stdin=subprocess.PIPE,
            stdout=subprocess.PIPE,
            stderr=subprocess.PIPE
        )

        stdout, _ = process.communicate(self.randomString.encode('utf-8'))
        response = stdout.decode('utf-8')
        headers, body = response.split('\n\n', 1)

        self.assertIn('Status: 201', headers)
        self.assertIn('Location: /uploads/テスト.txt', headers)

    def testBinaryFileUpload(self):
        """バイナリファイルアップロードテスト"""
        binary_data = bytes(range(256))  # 0-255のバイト列

        env = {
            "REQUEST_METHOD": "POST",
            "QUERY_STRING": "filename=binary.bin",
            "CONTENT_LENGTH": str(len(binary_data)),
            "CONTENT_TYPE": "application/octet-stream",
            "UPLOAD_DIR": self.uploadDir,
        }

        process = subprocess.Popen(
            [self.cgiPath],
            env=env,
            stdin=subprocess.PIPE,
            stdout=subprocess.PIPE,
            stderr=subprocess.PIPE
        )

        stdout, _ = process.communicate(binary_data)
        response = stdout.decode('utf-8')
        headers, body = response.split('\n\n', 1)

        self.assertIn('Status: 201', headers)
        self.assertIn('Location: /uploads/binary.bin', headers)

        uploaded_file = os.path.join(self.uploadDir, "binary.bin")
        self.assertTrue(os.path.exists(uploaded_file))
        with open(uploaded_file, 'rb') as f:
            self.assertEqual(f.read(), binary_data)

    def testGetRequestTimestamp(self):
        """GETリクエストのタイムスタンプ確認テスト"""
        env = {
            "REQUEST_METHOD": "GET",
            "QUERY_STRING": "",
            "CONTENT_LENGTH": "0",
            "CONTENT_TYPE": "application/octet-stream",
        }

        process = subprocess.Popen(
            [self.cgiPath],
            env=env,
            stdout=subprocess.PIPE,
            stderr=subprocess.PIPE
        )

        stdout, stderr = process.communicate()
        response = stdout.decode('utf-8')
        headers, body = response.split('\n\n', 1)

        self.assertIn('Status: 200', headers)
        self.assertIn('Content-Type: text/plain', headers)
        # 日本語タイムスタンプ形式の確認
        self.assertRegex(body, r'\d{4}年\d{1,2}月\d{1,2}日 \d{1,2}時\d{1,2}分\d{1,2}秒')

if __name__ == '__main__':
    unittest.main()