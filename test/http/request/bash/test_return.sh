#!/bin/bash

# Test all return directive endpoints

echo "Testing all return endpoints..."
echo ""

# 200
curl -i http://localhost:8080/200-no-text
curl -i http://localhost:8080/200-with-text
curl -i http://localhost:8080/200-with-url

# 201
curl -i http://localhost:8080/201-no-text
curl -i http://localhost:8080/201-with-text
curl -i http://localhost:8080/201-with-url

# 202
curl -i http://localhost:8080/202-no-text
curl -i http://localhost:8080/202-with-text
curl -i http://localhost:8080/202-with-url

# 203
curl -i http://localhost:8080/203-no-text
curl -i http://localhost:8080/203-with-text
curl -i http://localhost:8080/203-with-url

# 204
curl -i http://localhost:8080/204-no-text
curl -i http://localhost:8080/204-with-text
curl -i http://localhost:8080/204-with-url

# 205
curl -i http://localhost:8080/205-no-text
curl -i http://localhost:8080/205-with-text
curl -i http://localhost:8080/205-with-url

# 206
curl -i http://localhost:8080/206-no-text
curl -i http://localhost:8080/206-with-text
curl -i http://localhost:8080/206-with-url

# 300
curl -i http://localhost:8080/300-no-text
curl -i http://localhost:8080/300-with-text
curl -i http://localhost:8080/300-with-url

# 301
curl -i http://localhost:8080/301-no-text
curl -i http://localhost:8080/301-with-text
curl -i http://localhost:8080/301-with-url

# 302
curl -i http://localhost:8080/302-no-text
curl -i http://localhost:8080/302-with-text
curl -i http://localhost:8080/302-with-url

# 303
curl -i http://localhost:8080/303-no-text
curl -i http://localhost:8080/303-with-text
curl -i http://localhost:8080/303-with-url

# 304
curl -i http://localhost:8080/304-no-text
curl -i http://localhost:8080/304-with-text
curl -i http://localhost:8080/304-with-url

# 307
curl -i http://localhost:8080/307-no-text
curl -i http://localhost:8080/307-with-text
curl -i http://localhost:8080/307-with-url

# 308
curl -i http://localhost:8080/308-no-text
curl -i http://localhost:8080/308-with-text
curl -i http://localhost:8080/308-with-url

# 400
curl -i http://localhost:8080/400-no-text
curl -i http://localhost:8080/400-with-text
curl -i http://localhost:8080/400-with-url

# 401
curl -i http://localhost:8080/401-no-text
curl -i http://localhost:8080/401-with-text
curl -i http://localhost:8080/401-with-url

# 402
curl -i http://localhost:8080/402-no-text
curl -i http://localhost:8080/402-with-text
curl -i http://localhost:8080/402-with-url

# 403
curl -i http://localhost:8080/403-no-text
curl -i http://localhost:8080/403-with-text
curl -i http://localhost:8080/403-with-url

# 404
curl -i http://localhost:8080/404-no-text
curl -i http://localhost:8080/404-with-text
curl -i http://localhost:8080/404-with-url

# 405
curl -i http://localhost:8080/405-no-text
curl -i http://localhost:8080/405-with-text
curl -i http://localhost:8080/405-with-url

# 406
curl -i http://localhost:8080/406-no-text
curl -i http://localhost:8080/406-with-text
curl -i http://localhost:8080/406-with-url

# 407
curl -i http://localhost:8080/407-no-text
curl -i http://localhost:8080/407-with-text
curl -i http://localhost:8080/407-with-url

# 408
curl -i http://localhost:8080/408-no-text
curl -i http://localhost:8080/408-with-text
curl -i http://localhost:8080/408-with-url

# 409
curl -i http://localhost:8080/409-no-text
curl -i http://localhost:8080/409-with-text
curl -i http://localhost:8080/409-with-url

# 410
curl -i http://localhost:8080/410-no-text
curl -i http://localhost:8080/410-with-text
curl -i http://localhost:8080/410-with-url

# 411
curl -i http://localhost:8080/411-no-text
curl -i http://localhost:8080/411-with-text
curl -i http://localhost:8080/411-with-url

# 412
curl -i http://localhost:8080/412-no-text
curl -i http://localhost:8080/412-with-text
curl -i http://localhost:8080/412-with-url

# 413
curl -i http://localhost:8080/413-no-text
curl -i http://localhost:8080/413-with-text
curl -i http://localhost:8080/413-with-url

# 414
curl -i http://localhost:8080/414-no-text
curl -i http://localhost:8080/414-with-text
curl -i http://localhost:8080/414-with-url

# 415
curl -i http://localhost:8080/415-no-text
curl -i http://localhost:8080/415-with-text
curl -i http://localhost:8080/415-with-url

# 416
curl -i http://localhost:8080/416-no-text
curl -i http://localhost:8080/416-with-text
curl -i http://localhost:8080/416-with-url

# 417
curl -i http://localhost:8080/417-no-text
curl -i http://localhost:8080/417-with-text
curl -i http://localhost:8080/417-with-url

# 418
curl -i http://localhost:8080/418-no-text
curl -i http://localhost:8080/418-with-text
curl -i http://localhost:8080/418-with-url

# 421
curl -i http://localhost:8080/421-no-text
curl -i http://localhost:8080/421-with-text
curl -i http://localhost:8080/421-with-url

# 422
curl -i http://localhost:8080/422-no-text
curl -i http://localhost:8080/422-with-text
curl -i http://localhost:8080/422-with-url

# 426
curl -i http://localhost:8080/426-no-text
curl -i http://localhost:8080/426-with-text
curl -i http://localhost:8080/426-with-url

# 500
curl -i http://localhost:8080/500-no-text
curl -i http://localhost:8080/500-with-text
curl -i http://localhost:8080/500-with-url

# 501
curl -i http://localhost:8080/501-no-text
curl -i http://localhost:8080/501-with-text
curl -i http://localhost:8080/501-with-url

# 502
curl -i http://localhost:8080/502-no-text
curl -i http://localhost:8080/502-with-text
curl -i http://localhost:8080/502-with-url

# 503
curl -i http://localhost:8080/503-no-text
curl -i http://localhost:8080/503-with-text
curl -i http://localhost:8080/503-with-url

# 504
curl -i http://localhost:8080/504-no-text
curl -i http://localhost:8080/504-with-text
curl -i http://localhost:8080/504-with-url

# 505
curl -i http://localhost:8080/505-no-text
curl -i http://localhost:8080/505-with-text
curl -i http://localhost:8080/505-with-url

# server return
curl -i http://localhost:8081/

# roop
curl -i http://localhost:8080/loop1

echo ""
echo "All tests completed!"