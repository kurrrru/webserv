#!/bin/bash

chmod -R 755 ./docs/html/no_perm.html
rm -f ./docs/html/no_perm.html

rm -rf ./docs/empty_dir

chmod -R 755 ./docs/no_perm_dir
rm -rf ./docs/no_perm_dir

chmod -R 755 ./docs/no_perm_uploads/no_perm_uploadstore
rm -rf ./docs/no_perm_uploads
