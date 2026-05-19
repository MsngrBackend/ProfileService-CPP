# =========================================================
# Builder stage
# =========================================================
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
    && rm -rf /var/lib/apt/lists/*

# =========================================================
# Build and install inih with CMake support
# =========================================================
RUN git clone https://github.com/benhoyt/inih.git /tmp/inih && \
    cd /tmp/inih && \
    mkdir -p build && \
    cd build && \
    cmake -DCMAKE_BUILD_TYPE=Release \
          -DBUILD_SHARED_LIBS=ON \
          -DCMAKE_POSITION_INDEPENDENT_CODE=ON \
          .. && \
    make -j$(nproc) && \
    make install && \
    ldconfig

# =========================================================
# Build and install minio-cpp from source
# =========================================================
RUN git clone https://github.com/minio/minio-cpp.git /tmp/minio-cpp && \
    cd /tmp/minio-cpp && \
    # Use a stable commit that's known to work
    git checkout 5e8ad3c && \
    mkdir -p build && \
    cd build && \
    cmake -DCMAKE_BUILD_TYPE=Release \
          -DBUILD_SHARED_LIBS=ON \
          -DCMAKE_PREFIX_PATH="/usr/local/lib/cmake;/usr/lib/$(uname -m)-linux-gnu/cmake" \
          -Dinih_DIR="/usr/local/lib/cmake/inih" \
          .. && \
    make -j$(nproc) && \
    make install && \
    ldconfig

# =========================================================
# Build the application
# =========================================================
WORKDIR /build

# Copy CMake files and source
COPY CMakeLists.txt ./
COPY src/ ./src/
COPY include/ ./include/

# Create build directory
RUN mkdir -p build && cd build && \
    cmake .. \
        -DCMAKE_BUILD_TYPE=Release \
        -DCMAKE_PREFIX_PATH="/usr/local/lib/cmake;/usr/lib/$(uname -m)-linux-gnu/cmake" && \
    make -j$(nproc)

# =========================================================
# Runtime stage
# =========================================================
FROM ubuntu:24.04 AS runtime

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
    libpugixml1v5 \
    zlib1g \
    && rm -rf /var/lib/apt/lists/*

# Copy built libraries from builder
COPY --from=builder /usr/local/lib/libinih* /usr/local/lib/
COPY --from=builder /usr/local/lib/libminiocpp* /usr/local/lib/
COPY --from=builder /usr/local/include/inih/ /usr/local/include/inih/
COPY --from=builder /usr/local/include/miniocpp/ /usr/local/include/miniocpp/
COPY --from=builder /usr/local/lib/cmake/inih/ /usr/local/lib/cmake/inih/
COPY --from=builder /usr/local/lib/cmake/miniocpp/ /usr/local/lib/cmake/miniocpp/

# Copy application binary
COPY --from=builder /build/build/profile_service /app/profile_service

# Run ldconfig to update library cache
RUN ldconfig

# Set working directory
WORKDIR /app

# Expose port
EXPOSE 8082

# Run the application
CMD ["/app/profile_service"]