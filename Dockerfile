# Multi-stage build for Profile Service
# Stage 1: Builder
FROM ubuntu:24.04 AS builder

# Install build dependencies
RUN apt-get update && apt-get install -y \
    build-essential \
    gcc \
    g++ \
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
    && rm -rf /var/lib/apt/lists/*

# Create CMake config for curlpp (if needed)
RUN mkdir -p /usr/local/lib/cmake/unofficial-curlpp && \
    cat > /usr/local/lib/cmake/unofficial-curlpp/unofficial-curlppConfig.cmake <<'EOF'
add_library(unofficial::curlpp::curlpp INTERFACE IMPORTED)
set_target_properties(unofficial::curlpp::curlpp PROPERTIES
    INTERFACE_INCLUDE_DIRECTORIES "/usr/include/curlpp"
    INTERFACE_LINK_LIBRARIES "curl"
)
EOF

# Create CMake config for inih (if needed)
RUN mkdir -p /usr/local/lib/cmake/unofficial-inih && \
    cat > /usr/local/lib/cmake/unofficial-inih/unofficial-inihConfig.cmake <<'EOF'
add_library(unofficial::inih::inih INTERFACE IMPORTED)
set_target_properties(unofficial::inih::inih PROPERTIES
    INTERFACE_INCLUDE_DIRECTORIES "/usr/include"
    INTERFACE_LINK_LIBRARIES "inih"
)
EOF

# Build minio-cpp from source using the LATEST main branch (not a shallow clone)
RUN git clone https://github.com/minio/minio-cpp.git /tmp/minio-cpp && \
    cd /tmp/minio-cpp && \
    # Use the latest main branch which has better compatibility
    git checkout main && \
    git pull origin main && \
    # Build and install
    cmake -B build \
        -DCMAKE_BUILD_TYPE=Release \
        -DBUILD_SHARED_LIBS=ON \
        -DCMAKE_PREFIX_PATH="/usr/local/lib/cmake;/usr/lib/$(uname -m)-linux-gnu/cmake" && \
    cmake --build build -j$(nproc) && \
    cmake --install build

# Copy source code
WORKDIR /build
COPY . .

# Configure and build the application
RUN mkdir -p build && \
    cd build && \
    cmake .. \
        -DCMAKE_BUILD_TYPE=Release \
        -DCMAKE_PREFIX_PATH="/usr/local/lib/cmake;/usr/lib/$(uname -m)-linux-gnu/cmake" && \
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
    libcurlpp0t64 \
    libinih1 \
    libpugixml1v5 \
    zlib1g \
    && rm -rf /var/lib/apt/lists/*

# Copy minio-cpp libraries from builder
COPY --from=builder /usr/local/lib/libminiocpp.so* /usr/local/lib/
COPY --from=builder /usr/local/include/minio-cpp/ /usr/local/include/minio-cpp/

# Update library cache
RUN ldconfig

# Copy the built executable
COPY --from=builder /build/build/profile_service /usr/local/bin/

# Expose port
EXPOSE 8082

# Set working directory
WORKDIR /app

# Run the service
CMD ["/usr/local/bin/profile_service"]