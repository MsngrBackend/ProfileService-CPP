FROM ubuntu:24.04 AS builder

ENV DEBIAN_FRONTEND=noninteractive

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

# ------------------------------------------------------------------
# FIX unofficial curlpp package config for minio-cpp
# ------------------------------------------------------------------

RUN mkdir -p /usr/local/lib/cmake/unofficial-curlpp && \
    cat > /usr/local/lib/cmake/unofficial-curlpp/unofficial-curlppConfig.cmake <<'EOF'
add_library(unofficial::curlpp::curlpp INTERFACE IMPORTED)

find_library(CURLPP_LIBRARY curlpp REQUIRED)
find_library(CURL_LIBRARY curl REQUIRED)

target_link_libraries(unofficial::curlpp::curlpp
    INTERFACE
    ${CURLPP_LIBRARY}
    ${CURL_LIBRARY}
)
EOF

# ------------------------------------------------------------------
# FIX unofficial inih package config for minio-cpp
# ------------------------------------------------------------------

RUN mkdir -p /usr/local/lib/cmake/unofficial-inih && \
    cat > /usr/local/lib/cmake/unofficial-inih/unofficial-inihConfig.cmake <<'EOF'
find_path(INIH_INCLUDE_DIR
    NAMES inih/INIReader.h
    PATHS /usr/include
)

add_library(unofficial::inih::inireader INTERFACE IMPORTED)

target_include_directories(unofficial::inih::inireader
    INTERFACE
    ${INIH_INCLUDE_DIR}
)
EOF

# ------------------------------------------------------------------
# Build minio-cpp
# ------------------------------------------------------------------

RUN git clone --depth 1 https://github.com/minio/minio-cpp.git /tmp/minio-cpp && \
    cd /tmp/minio-cpp && \
    cmake -B build \
        -DCMAKE_BUILD_TYPE=Release \
        -DBUILD_SHARED_LIBS=ON \
        -DCMAKE_PREFIX_PATH="/usr/local/lib/cmake;/usr/lib/aarch64-linux-gnu/cmake" && \
    cmake --build build -j$(nproc) && \
    cmake --install build

WORKDIR /app

COPY . .

RUN cmake -B build \
    -DCMAKE_BUILD_TYPE=Release

RUN cmake --build build -j$(nproc)

# ==================================================================
# RUNTIME
# ==================================================================

FROM ubuntu:24.04

ENV DEBIAN_FRONTEND=noninteractive

RUN apt-get update && apt-get install -y \
    ca-certificates \
    curl \
    libpqxx-dev \
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
    libcurlpp0t64 \
    libinih1 \
    libpugixml1v5 \
    zlib1g \
    && rm -rf /var/lib/apt/lists/*

WORKDIR /app

COPY --from=builder /usr/local/lib/libminiocpp.so* /usr/local/lib/
COPY --from=builder /app/build/profile_service /app/profile_service

RUN ldconfig

EXPOSE 8080

CMD ["./profile_service"]