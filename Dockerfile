# Multi-stage build for Profile Service
# Stage 1: Builder
FROM ubuntu:24.04 AS builder

# Install base build tools
RUN apt-get update && apt-get install -y \
    build-essential \
    cmake \
    pkg-config \
    git \
    curl \
    zip \
    unzip \
    tar \
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
    ninja-build \
    && rm -rf /var/lib/apt/lists/*

# Install vcpkg with specific version to avoid CMake 4.x bug
RUN git clone https://github.com/Microsoft/vcpkg.git /opt/vcpkg && \
    cd /opt/vcpkg && \
    # Pin to a known working version (December 2024)
    git checkout 2024.12.09 && \
    ./bootstrap-vcpkg.sh -disableMetrics

# Install minio-cpp using vcpkg with ARM64 triplet
RUN /opt/vcpkg/vcpkg install minio-cpp --triplet arm64-linux

# Copy source code
WORKDIR /build
COPY . .

# Configure and build the application
RUN mkdir -p build && \
    cd build && \
    cmake .. \
        -DCMAKE_TOOLCHAIN_FILE=/opt/vcpkg/scripts/buildsystems/vcpkg.cmake \
        -DCMAKE_BUILD_TYPE=Release && \
    cmake --build . -j$(nproc)

# Stage 2: Runtime
FROM ubuntu:24.04

# Install runtime dependencies
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
    libpugixml1v5 \
    zlib1g \
    && rm -rf /var/lib/apt/lists/*

# Copy vcpkg-installed libraries
COPY --from=builder /opt/vcpkg/installed/arm64-linux/lib/*.so* /usr/local/lib/
COPY --from=builder /opt/vcpkg/installed/arm64-linux/include/minio-cpp/ /usr/local/include/minio-cpp/

# Update library cache
RUN ldconfig

# Copy the built executable
COPY --from=builder /build/build/profile_service /usr/local/bin/

EXPOSE 8082
WORKDIR /app
CMD ["/usr/local/bin/profile_service"]