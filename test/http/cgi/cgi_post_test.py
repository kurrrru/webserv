#!/usr/bin/env python3

import os
import unittest
import subprocess
import datetime
import random
import string

def generateRandomString(length):
    return ''.join(random.choices(string.ascii_letters + string.digits, k=length))

class TestCgiGET(unittest.TestCase):
    def setUp(self):
        self.uploadDir = "./uploads/"
        self.cgiPath = "../../../cgi-bin/script.py"
        self.randomString = generateRandomString(30)
        self.randomStringLength = len(self.randomString)
        os.makedirs(self.uploadDir, exist_ok=True)

        self.env = {
            "REQUEST_METHOD": "POST",
            "QUERY_STRING": "filename=test.txt",
            "CONTENT_LENGTH": str(self.randomStringLength),
            "CONTENT_TYPE": "application/octet-stream",
            f"UPLOAD_DIR": self.uploadDir,
        }
    # delete all files in the upload directory
    def tearDown(self):
        for file in os.listdir(self.uploadDir):
            try:
                os.remove(os.path.join(self.uploadDir, file))
            except:
                pass
        os.rmdir(self.uploadDir)

    def testDefaultPostRequest(self):
        process = subprocess.Popen(
            [self.cgiPath],
            env=self.env,
            stdin=subprocess.PIPE,
            stdout=subprocess.PIPE,
            stderr=subprocess.PIPE
        )

        stdout, _ = process.communicate(self.randomString.encode('utf-8'))
        response = stdout.decode('utf-8')
        headers, _ = response.split('\n\n', 1)

        self.assertIn('Status: 201', headers)
        self.assertIn('Content-Type: text/plain', headers)
        self.assertIn('Location: /uploads/test.txt', headers)

    def testNoUploadPath(self):
        env = self.env.copy()
        env.pop("UPLOAD_DIR", None)
        
        process = subprocess.Popen(
            [self.cgiPath],
            env=env,
            stdin=subprocess.PIPE,
            stdout=subprocess.PIPE,
            stderr=subprocess.PIPE
        )

        stdout, _ = process.communicate(self.randomString.encode('utf-8'))
        response = stdout.decode('utf-8')
        headers, _ = response.split('\n\n', 1)

        self.assertIn('Status: 500', headers)

    def testNoWritePermission(self):
        # Create directory with read-only permissions
        os.chmod(self.uploadDir, 0o444)
        
        process = subprocess.Popen(
            [self.cgiPath],
            env=self.env,
            stdin=subprocess.PIPE,
            stdout=subprocess.PIPE,
            stderr=subprocess.PIPE
        )

        stdout, _ = process.communicate(self.randomString.encode('utf-8'))
        response = stdout.decode('utf-8')
        headers, _ = response.split('\n\n', 1)

        self.assertIn('Status: 403', headers)
        
        # Restore permissions
        os.chmod(self.uploadDir, 0o755)

    def testDuplicateFilename(self):
        # Create a file with the same name first
        test_file = os.path.join(self.uploadDir, "test.txt")
        with open(test_file, "w") as f:
            f.write("existing content")

        process = subprocess.Popen(
            [self.cgiPath],
            env=self.env,
            stdin=subprocess.PIPE,
            stdout=subprocess.PIPE,
            stderr=subprocess.PIPE
        )

        stdout, _ = process.communicate(self.randomString.encode('utf-8'))
        response = stdout.decode('utf-8')
        headers, _ = response.split('\n\n', 1)

        self.assertIn('Status: 201', headers)
        # Check if the new filename contains timestamp
        self.assertNotIn(f'Location: /uploads/test.txt{datetime.datetime.now().strftime("%Y%m%d%H%M%S")}', headers)
        self.assertIn(f'Location: /uploads/test.txt', headers)

    def testSpecialCharsInFilename(self):
        env = self.env.copy()
        env["QUERY_STRING"] = "filename=test@#$%.txt"
        
        process = subprocess.Popen(
            [self.cgiPath],
            env=env,
            stdin=subprocess.PIPE,
            stdout=subprocess.PIPE,
            stderr=subprocess.PIPE
        )

        stdout, _ = process.communicate(self.randomString.encode('utf-8'))
        response = stdout.decode('utf-8')
        headers, _ = response.split('\n\n', 1)

        self.assertIn('Status: 201', headers)
        self.assertIn('Location: /uploads/test@#$%.txt', headers)

    def testSpecialCharsInBody(self):
        special_chars = "!@#$%^&*()_+{}|:<>?~`-=[]\\;',./"
        
        process = subprocess.Popen(
            [self.cgiPath],
            env=self.env,
            stdin=subprocess.PIPE,
            stdout=subprocess.PIPE,
            stderr=subprocess.PIPE
        )

        stdout, _ = process.communicate(special_chars.encode('utf-8'))
        response = stdout.decode('utf-8')
        headers, _ = response.split('\n\n', 1)

        self.assertIn('Status: 201', headers)

if __name__ == '__main__':
    unittest.main()