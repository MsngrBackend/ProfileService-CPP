# ============================================================
# Stage 1: Builder
# ============================================================

FROM ubuntu:24.04 AS builder

ENV DEBIAN_FRONTEND=noninteractive

# ============================================================
# Install build dependencies
# ============================================================

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

# ============================================================
# unofficial-curlpp compatibility package
# minio-cpp expects unofficial::curlpp
# ============================================================

RUN mkdir -p /usr/local/lib/cmake/unofficial-curlpp && \
    cat > /usr/local/lib/cmake/unofficial-curlpp/unofficial-curlppConfig.cmake <<'EOF'
find_library(CURLPP_LIBRARY NAMES curlpp REQUIRED)
find_path(CURLPP_INCLUDE_DIR NAMES curlpp/cURLpp.hpp REQUIRED)

add_library(unofficial::curlpp UNKNOWN IMPORTED)

set_target_properties(unofficial::curlpp PROPERTIES
    IMPORTED_LOCATION "${CURLPP_LIBRARY}"
    INTERFACE_INCLUDE_DIRECTORIES "${CURLPP_INCLUDE_DIR}"
)
EOF

# ============================================================
# unofficial-inih compatibility package
# minio-cpp expects unofficial::inih
# ============================================================

RUN mkdir -p /usr/local/lib/cmake/unofficial-inih && \
    cat > /usr/local/lib/cmake/unofficial-inih/unofficial-inihConfig.cmake <<'EOF'
find_library(INIH_LIBRARY NAMES inih REQUIRED)
find_path(INIH_INCLUDE_DIR NAMES inih/INIReader.h REQUIRED)

add_library(unofficial::inih::libinih UNKNOWN IMPORTED)
add_library(unofficial::inih::inireader UNKNOWN IMPORTED)

set_target_properties(unofficial::inih::libinih PROPERTIES
    IMPORTED_LOCATION "${INIH_LIBRARY}"
    INTERFACE_INCLUDE_DIRECTORIES "${INIH_INCLUDE_DIR}"
)

set_target_properties(unofficial::inih::inireader PROPERTIES
    IMPORTED_LOCATION "${INIH_LIBRARY}"
    INTERFACE_INCLUDE_DIRECTORIES "${INIH_INCLUDE_DIR}"
)
EOF

# ============================================================
# Build and install minio-cpp
# ============================================================

RUN git clone --depth 1 https://github.com/minio/minio-cpp.git /tmp/minio-cpp && \
    cd /tmp/minio-cpp && \
    cmake -B build \
        -DCMAKE_BUILD_TYPE=Release \
        -DBUILD_SHARED_LIBS=ON \
        -DCMAKE_PREFIX_PATH=/usr/local/lib/cmake && \
    cmake --build build -j$(nproc) && \
    cmake --install build

# ============================================================
# Build profile service
# ============================================================

WORKDIR /app

COPY . .

RUN cmake -B build \
        -DCMAKE_BUILD_TYPE=Release && \
    cmake --build build -j$(nproc)

# ============================================================
# Stage 2: Runtime
# ============================================================

FROM ubuntu:24.04

ENV DEBIAN_FRONTEND=noninteractive

# ============================================================
# Runtime dependencies only
# ============================================================

RUN apt-get update && apt-get install -y \
    ca-certificates \
    curl \
    libpqxx-7.7 \
    libboost-system1.83.0 \
    libboost-program-options1.83.0 \
    libboost-log1.83.0 \
    libboost-log1.83-dev \
    libboost-thread1.83.0 \
    libboost-filesystem1.83.0 \
    libboost-regex1.83.0 \
    libboost-date-time1.83.0 \
    libssl3t64 \
    libcurl4t64 \
    libcurlpp0 \
    libinih1 \
    zlib1g \
    && rm -rf /var/lib/apt/lists/*

# ============================================================
# App directory
# ============================================================

WORKDIR /app

# ============================================================
# Copy binary
# ============================================================

COPY --from=builder /app/build/profile_service .

# ============================================================
# Copy minio-cpp shared libraries
# ============================================================

COPY --from=builder /usr/local/lib/libminiocpp* /usr/local/lib/

# ============================================================
# Refresh linker cache
# ============================================================

RUN ldconfig

# ============================================================
# Runtime library path
# ============================================================

ENV LD_LIBRARY_PATH=/usr/local/lib

# ============================================================
# Expose app port
# ============================================================

EXPOSE 8082

# ============================================================
# Start service
# ============================================================

CMD ["./profile_service"]