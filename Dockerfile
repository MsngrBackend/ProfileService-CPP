# Multi-stage build for Profile Service
# Using Ubuntu 24.04 for better package support

# Stage 1: Builder
FROM ubuntu:24.04 AS builder

# Install build dependencies including curlpp
RUN apt-get update && apt-get install -y \
    build-essential \
    gcc \
    g++ \
    cmake \
    pkg-config \
    git \
    curl \
    libcurl4-openssl-dev \
    libcurlpp-dev \
    zip \
    unzip \
    tar \
    libpqxx-dev \
    libboost-all-dev \
    libssl-dev \
    zlib1g-dev \
    nlohmann-json3-dev \
    libpugixml-dev \
    libinih-dev \
    && rm -rf /var/lib/apt/lists/*

# Create a cmake config for curlpp (minio-cpp expects unofficial-curlpp)
RUN mkdir -p /usr/local/lib/cmake/unofficial-curlpp && \
    cat > /usr/local/lib/cmake/unofficial-curlpp/unofficial-curlppConfig.cmake <<'EOF'
# CMake config for curlpp
include(FindPkgConfig)
pkg_check_modules(CURLPP IMPORTED_TARGET libcurlpp)
if(NOT CURLPP_FOUND)
    find_library(CURLPP_LIBRARY NAMES curlpp)
    find_path(CURLPP_INCLUDE_DIR NAMES curlpp/cURLpp.hpp)
    if(CURLPP_LIBRARY AND CURLPP_INCLUDE_DIR)
        add_library(unofficial::curlpp UNKNOWN IMPORTED)
        set_target_properties(unofficial::curlpp PROPERTIES
            IMPORTED_LOCATION "${CURLPP_LIBRARY}"
            INTERFACE_INCLUDE_DIRECTORIES "${CURLPP_INCLUDE_DIR}"
        )
    endif()
endif()
EOF

# Build minio-cpp
RUN git clone --depth 1 https://github.com/minio/minio-cpp.git /minio-cpp && \
    cd /minio-cpp && \
    # Patch CMakeLists.txt to find curlpp
    sed -i 's/find_package(unofficial-curlpp REQUIRED)/find_package(PkgConfig REQUIRED)\npkg_check_modules(CURLPP REQUIRED libcurlpp)/' CMakeLists.txt && \
    cmake -B build \
        -DCMAKE_BUILD_TYPE=Release \
        -DBUILD_SHARED_LIBS=ON \
        -DCMAKE_PREFIX_PATH=/usr/local/lib/cmake/unofficial-curlpp \
        -DCMAKE_CXX_FLAGS="-I/usr/include/curlpp" && \
    cmake --build build -j$(nproc) && \
    cmake --install build

WORKDIR /app
COPY . .

# Build the application
RUN cmake -B build -DCMAKE_BUILD_TYPE=Release && \
    cmake --build build -j$(nproc)

# Stage 2: Runtime
FROM ubuntu:24.04

RUN apt-get update && apt-get install -y \
    libpqxx-dev \
    libboost-system1.83.0 \
    libboost-program-options1.83.0 \
    libboost-log1.83.0 \
    libboost-thread1.83.0 \
    libboost-filesystem1.83.0 \
    libboost-regex1.83.0 \
    libboost-date-time1.83.0 \
    libcurl4t64 \
    libcurlpp-dev \
    libssl3t64 \
    ca-certificates \
    && rm -rf /var/lib/apt/lists/*

WORKDIR /app

COPY --from=builder /app/build/profile_service /app/
COPY --from=builder /usr/local/lib/libminiocpp* /usr/local/lib/
COPY --from=builder /usr/local/lib/cmake/miniocpp /usr/local/lib/cmake/miniocpp 2>/dev/null || true

ENV LD_LIBRARY_PATH=/usr/local/lib:${LD_LIBRARY_PATH}

EXPOSE 8082

CMD ["./profile_service"]