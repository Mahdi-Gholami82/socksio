# SOCKS Server

Simple SOCKS5 proxy server implementation using ASIO and C++23.

> Note: Currently supports only SOCKS version 5, CONNECT
command, and no authentication.

### Build
```shell
cmake -B build -S . -DCMAKE_BUILD_TYPE=Release
cmake --build build
```

### Usage
```shell
socksio --addr 127.0.0.1 --port 1080
```