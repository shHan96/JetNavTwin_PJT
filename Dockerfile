FROM nvcr.io/nvidia/l4t-base:r32.3.1

LABEL maintainer="yeobeom bangga0403@gmail.com" \
      description="Cross-compilation environment for ARM64" \
      version="1.0"

# Set environment variables
ENV DEBIAN_FRONTEND=noninteractive \
    CROSS_COMPILE=1

# Install essential tools and cross-compilation toolchain
RUN apt-get update && apt-get install -y \
    build-essential \
    cmake \
    make \
    tar \
    wget \
    xz-utils && \
    apt-get clean && \
    rm -rf /var/lib/apt/lists/*

# Download and install NVIDIA L4T cross-compilation toolchain
RUN wget https://developer.nvidia.com/embedded/dlc/l4t-gcc-7-3-1-toolchain-64-bit && \
    tar xf l4t-gcc-7-3-1-toolchain-64-bit && \
    mv gcc-linaro-7.3.1-2018.05-x86_64_aarch64-linux-gnu /opt/ && \
    rm l4t-gcc-7-3-1-toolchain-64-bit

# Set environment variables
ENV PATH="/opt/gcc-linaro-7.3.1-2018.05-x86_64_aarch64-linux-gnu/bin:${PATH}" \
    CC=aarch64-linux-gnu-gcc \
    CXX=aarch64-linux-gnu-g++

# Set working directory
WORKDIR /app

CMD ["/bin/bash"]