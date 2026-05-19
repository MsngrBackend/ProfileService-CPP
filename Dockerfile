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
    libpqxx-dev \
    libboost-all-dev \
    nlohmann-json3-dev \
    libpugixml-dev \
    libinih-dev \
    && rm -rf /var/lib/apt/lists/*

# Install vcpkg
RUN git clone https://github.com/Microsoft/vcpkg.git /opt/vcpkg && \
    /opt/vcpkg/bootstrap-vcpkg.sh

# Install minio-cpp using vcpkg
RUN /opt/vcpkg/vcpkg install minio-cpp --triplet x64-linux

# Copy source code
WORKDIR /build
COPY . .

# Configure and build the application, informing CMake about vcpkg
RUN mkdir -p build && \
    cd build && \
    cmake .. \
        -DCMAKE_TOOLCHAIN_FILE=/opt/vcpkg/scripts/buildsystems/vcpkg.cmake \
        -DCMAKE_BUILD_TYPE=Release && \
    cmake --build . -j$(nproc)

# Stage 2: Runtime
FROM ubuntu:24.04

# Install runtime dependencies (same as before)
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

# Copy the built executable
COPY --from=builder /build/build/profile_service /usr/local/bin/

EXPOSE 8082
WORKDIR /app
CMD ["/usr/local/bin/profile_service"]