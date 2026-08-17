#!/bin/bash

# Build different variants
build_http() {
    echo "Building HTTP version..."
    docker build -f Dockerfile.http -t profile-service:http .
}

build_https() {
    echo "Building HTTPS version (with auto-generated certificates)..."
    docker build -f Dockerfile.https -t profile-service:https .
}

build_both() {
    echo "Building version with both HTTP and HTTPS..."
    docker build --build-arg ENABLE_HTTPS=true -f Dockerfile -t profile-service:both .
}

case "${1:-http}" in
    http)
        build_http
        ;;
    https)
        build_https
        ;;
    both)
        build_both
        ;;
    all)
        build_http
        build_https
        build_both
        ;;
    *)
        echo "Usage: $0 {http|https|both|all}"
        exit 1
        ;;
esac