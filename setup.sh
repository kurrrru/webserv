#!/bin/bash

# no permission html file testcase
touch ./docs/html/no_perm.html
echo "This HTML file has no permissions" > ./docs/html/no_perm.html
chmod 000 ./docs/html/no_perm.html

# empty directory testcase
mkdir ./docs/empty_dir

# html file in no permissions directory testcase
mkdir ./docs/no_perm_dir
touch ./docs/no_perm_dir/sample.html
echo "This directory has no permissions" > ./docs/no_perm_dir/sample.html
chmod 000 ./docs/no_perm_dir

# no permission uploadstore directory testcase
mkdir ./docs/no_perm_uploads
mkdir ./docs/no_perm_uploads/no_perm_uploadstore
chmod 000 ./docs/no_perm_uploads/no_perm_uploadstore
