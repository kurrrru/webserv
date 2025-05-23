#!/usr/bin/env python3

import os
import unittest
import subprocess
import datetime

class TestCgiGET(unittest.TestCase):
    def setUp(self):
        self.env = {
            "REQUEST_METHOD": "GET",
            "QUERY_STRING": "",
            "CONTENT_LENGTH": "0",
            "CONTENT_TYPE": "application/octet-stream",
        }

    def testGetRequest(self):
        process = subprocess.Popen(
            ["../../../cgi-bin/script.py"],
            env=self.env,
            stdout=subprocess.PIPE,
            stderr=subprocess.PIPE
        )

        stdout, _ = process.communicate()
        response = stdout.decode('utf-8')
        headers, body = response.split('\n\n', 1)

        self.assertIn('Status: 200', headers)
        self.assertIn('Content-Type: text/plain', headers)
        self.assertRegex(body, r'\d{4}年\d{1,2}月\d{1,2}日 \d{1,2}時\d{1,2}分\d{1,2}秒')

if __name__ == '__main__':
    unittest.main()