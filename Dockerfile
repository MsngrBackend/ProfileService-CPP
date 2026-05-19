# Stage 1: Builder
FROM ubuntu:24.04 AS builder

# Install system build dependencies
RUN apt-get update && apt-get install -y \
    build-essential \
    cmake \
    pkg-config \
    git \
    curl \
    ca-certificates \
    libssl-dev \
    zlib1g-dev \
    libcurl4-openssl-dev \
    libcurlpp-dev \
    libpqxx-dev \
    libboost-all-dev \
    nlohmann-json3-dev \
    libpugixml-dev \
    libinih-dev \
    && rm -rf /var/lib/apt/lists/*

# Build minio-cpp from source directly (no vcpkg)
RUN git clone --depth 1 https://github.com/minio/minio-cpp.git /tmp/minio-cpp && \
    cd /tmp/minio-cpp && \
    # Fix pugixml compatibility
    sed -i 's/set_value(value)/set_value(value.c_str())/' src/utils.cc && \
    # Build as static library
    cmake -B build \
        -DCMAKE_BUILD_TYPE=Release \
        -DBUILD_SHARED_LIBS=OFF \
        -DBUILD_TESTS=OFF \
        -DBUILD_EXAMPLES=OFF && \
    cmake --build build -j$(nproc) && \
    cmake --install build

# Build your project
WORKDIR /build
COPY . .
RUN mkdir -p build && cd build && \
    cmake .. -DCMAKE_BUILD_TYPE=Release && \
    cmake --build . -j$(nproc)

# Stage 2: Runtime
FROM ubuntu:24.04
RUN apt-get update && apt-get install -y \
    ca-certificates \
    curl \
    libpqxx-dev \
    libboost-system1.83.0 \
    libboost-program-options1.83.0 \
    libboost-log1.83.0 \
    libboost-thread1.83.0 \
    libboost-filesystem1.83.0 \
    libboost-regex1.83.0 \
    libboost-date-time1.83.0 \
    libssl3t64 \
    libcurl4t64 \
    libcurlpp0t64 \
    libinih1 \
    libpugixml1v5 \
    zlib1g \
    && rm -rf /var/lib/apt/lists/*

COPY --from=builder /usr/local/lib/libminiocpp.a /usr/local/lib/
COPY --from=builder /usr/local/include/minio-cpp/ /usr/local/include/minio-cpp/
COPY --from=builder /build/build/profile_service /usr/local/bin/

RUN ldconfig
EXPOSE 8082
WORKDIR /app
CMD ["/usr/local/bin/profile_service"]