FROM ubuntu:22.04

# 安装基础工具
RUN apt-get update && apt-get install -y \
    curl wget git unzip build-essential g++ clang python3 python3-pip

# 安装 Bazelisk（管理 Bazel 版本）
RUN wget -O /usr/local/bin/bazelisk https://github.com/bazelbuild/bazelisk/releases/download/v1.20.0/bazelisk-linux-amd64 \
    && chmod +x /usr/local/bin/bazelisk \
    && ln -s /usr/local/bin/bazelisk /usr/local/bin/bazel

WORKDIR /workspace
