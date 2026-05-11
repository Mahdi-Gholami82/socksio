# Socksio

Asynchronous SOCKS5 proxy server implementation compatible with RFC-1928.

> Note: Currently supports only SOCKS version 5, CONNECT
command, and no authentication.

### Build
```shell
cmake -B build -S . -DCMAKE_BUILD_TYPE=Release
cmake --build build
```

### Usage
```
Usage: socksio [options]
Options:
        -h, --help
                Show this help message and exit
        -p, --port
                Listen port number, Default: 1080
        -a, --addr
                Listen address, Default: 0.0.0.0
        -l, --log-level
                Logging level, Default: info (debug|info|warn|error|critical)
```