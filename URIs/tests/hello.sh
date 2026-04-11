#!/bin/bash
echo "Content-Type: text/plain"
echo ""
echo "Hello from Bash CGI!"
echo "Method: $REQUEST_METHOD"
echo "Query: $QUERY_STRING"
