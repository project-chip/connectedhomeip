# Mock HTTP/HTTPS Server

## Overview

This project provides a configurable mock HTTP/HTTPS server designed for API
testing, dynamic response generation, and automated request handling. It
supports static responses, dynamic custom response handlers, query parameter
matching, request body validation (including regex), and both HTTP and HTTPS
protocols.

## Setup

```bash
openssl req -x509 -newkey rsa:2048 -keyout server.key -out server.crt -days 365 -nodes -subj "/C=US/ST= /L= /O= /OU= /CN=localhost"
```

chip-tool validates the server certificate. To use chip-tool with this
self-signed certificate, either run it with `SSL_CERT_FILE=server.crt` and pass
`localhost` as the DCL hostname, or disable certificate validation with
`--dcl-disable-https-validation 1` on the `pairing` commands
(`--disable-https-validation 1` on the `dcl` commands).

## Features

### Server Functionality

-   Secure HTTPS with TLS support
-   CLI-driven execution with customizable options (port, configuration files,
    SSL options)
-   Handles GET, POST, PUT, and DELETE requests
-   Concurrent request handling via threading

### Route Matching

-   Exact path and wildcard (\*) path matching
-   Query parameter validation
-   Priority-based route matching

### Configuration

-   Main server configuration file
-   Separate routing configuration directory
-   JSON-based configuration format
-   Dynamic route loading

### Security & Logging

-   TLS encryption (HTTPS only)
-   Structured logging with DEBUG level
-   Graceful error handling for invalid routes

## Running Tests

### Test Execution

You can run the tests using one of these methods:

1. Using Python unittest with PYTHONPATH:

```bash
PYTHONPATH=$PYTHONPATH:/workspace/connectedhomeip/integrations/mock_server/src python3 -m unittest integrations/mock_server/tests/test_mock_server.py
```
