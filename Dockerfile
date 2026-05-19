# Stage 1: Builder
FROM ubuntu:24.04 AS builder

# Install dependencies
RUN apt-get update && apt-get install -y \
    build-essential cmake pkg-config git curl ca-certificates \
    libssl-dev zlib1g-dev libcurl4-openssl-dev libcurlpp-dev \
    libpqxx-dev libboost-all-dev nlohmann-json3-dev \
    libpugixml-dev libinih-dev \
    && rm -rf /var/lib/apt/lists/*

# Create missing CMake configs
RUN mkdir -p /usr/local/lib/cmake/unofficial-curlpp && \
    echo 'add_library(unofficial::curlpp::curlpp INTERFACE IMPORTED)' > /usr/local/lib/cmake/unofficial-curlpp/unofficial-curlppConfig.cmake && \
    echo 'set_target_properties(unofficial::curlpp::curlpp PROPERTIES INTERFACE_INCLUDE_DIRECTORIES "/usr/include/curlpp" INTERFACE_LINK_LIBRARIES "curl")' >> /usr/local/lib/cmake/unofficial-curlpp/unofficial-curlppConfig.cmake

RUN mkdir -p /usr/local/lib/cmake/unofficial-inih && \
    echo 'add_library(unofficial::inih::inih INTERFACE IMPORTED)' > /usr/local/lib/cmake/unofficial-inih/unofficial-inihConfig.cmake && \
    echo 'set_target_properties(unofficial::inih::inih PROPERTIES INTERFACE_INCLUDE_DIRECTORIES "/usr/include" INTERFACE_LINK_LIBRARIES "inih")' >> /usr/local/lib/cmake/unofficial-inih/unofficial-inihConfig.cmake

# Build minio-cpp
RUN git clone --depth 1 https://github.com/minio/minio-cpp.git /tmp/minio-cpp && \
    cd /tmp/minio-cpp && \
    sed -i 's/set_value(value)/set_value(value.c_str())/' src/utils.cc && \
    cmake -B build \
        -DCMAKE_BUILD_TYPE=Release \
        -DBUILD_SHARED_LIBS=OFF \
        -DBUILD_TESTS=OFF \
        -DBUILD_EXAMPLES=OFF \
        -DCMAKE_PREFIX_PATH="/usr/local/lib/cmake" && \
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
    ca-certificates curl libpqxx-dev \
    libboost-system1.83.0 libboost-program-options1.83.0 \
    libboost-log1.83.0 libboost-thread1.83.0 \
    libboost-filesystem1.83.0 libboost-regex1.83.0 \
    libboost-date-time1.83.0 libssl3t64 libcurl4t64 \
    libcurlpp0t64 libinih1 libpugixml1v5 zlib1g \
    && rm -rf /var/lib/apt/lists/*

COPY --from=builder /usr/local/lib/libminiocpp.a /usr/local/lib/
COPY --from=builder /usr/local/include/minio-cpp/ /usr/local/include/minio-cpp/
COPY --from=builder /build/build/profile_service /usr/local/bin/

RUN ldconfig
EXPOSE 8082
CMD ["/usr/local/bin/profile_service"]