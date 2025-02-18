#!/usr/bin/env -S python3 -B

# Copyright (c) 2025 Project CHIP Authors
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
# http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

import base64
import hashlib
import http.server
import json
import os
import re
import ssl

DEFAULT_HOSTNAME = "localhost"
DEFAULT_PORT = 4443


TC = {
    0XFFF1: {
        0x8001: {
            "schemaVersion": 1,
            "esfRevision": 1,
            "defaultCountry": "US",
            "countryEntries": {
                "US": {
                    "defaultLanguage": "en",
                    "languageEntries": {
                        "en": [
                            {
                                "ordinal": 0,
                                "required": True,
                                "title": "Terms and Conditions",
                                "text": "<p><b>Feature 1 Text</b><br><br>Please accept these.</p>"
                            },
                            {
                                "ordinal": 1,
                                "required": False,
                                "title": "Privacy Policy",
                                "text": "<p>Feature 2 Text</p>"
                            }
                        ],
                        "es": [
                            {
                                "ordinal": 0,
                                "required": True,
                                "title": "Términos y condiciones",
                                "text": "<p><b>Característica 1 Texto</b><br><br>Por favor acéptelos.</p>"
                            },
                            {
                                "ordinal": 1,
                                "required": False,
                                "title": "Política de privacidad",
                                "text": "<p>Característica 2 Texto</p>"
                            }
                        ]
                    }
                },
                "MX": {
                    "defaultLanguage": "es",
                    "languageEntries": {
                        "es": [
                            {
                                "ordinal": 0,
                                "required": True,
                                "title": "Términos y condiciones",
                                "text": "<p><b>Característica 1 Texto</b><br><br>Por favor acéptelos.</p>"
                            }
                        ]
                    }
                },
                "CN": {
                    "defaultLanguage": "zh",
                    "languageEntries": {
                        "zh": [
                            {
                                "ordinal": 0,
                                "required": True,
                                "title": "条款和条件",
                                "text": "<p><b>产品1文字</b></p>"
                            },
                            {
                                "ordinal": 1,
                                "required": False,
                                "title": "隐私条款",
                                "text": "<p><b>产品2文字</b></p>"
                            }
                        ]
                    }
                },
                "RU": {
                    "defaultLanguage": "ru",
                    "languageEntries": {
                        "ru": [
                            {
                                "ordinal": 0,
                                "required": True,
                                "title": "Условия и положения",
                                "text": "<p><b>Текст функции 1</b><br><br>Пожалуйста, примите эти условия пользования.</p>"
                            },
                            {
                                "ordinal": 1,
                                "required": False,
                                "title": "Положение о конфиденциальности",
                                "text": "<p>Текст функции 2</p>"
                            }
                        ]
                    }
                }
            }
        }
    }
}

MODELS = {
    0XFFF1: {
        0x8001: {
            "model":
            {
                "vid": 65521,
                "pid": 32769,
                "deviceTypeId": 65535,
                "productName": "TEST_PRODUCT",
                "productLabel": "All Clusters App",
                "partNumber": "",
                "commissioningCustomFlow": 2,
                "commissioningCustomFlowUrl": "",
                "commissioningModeInitialStepsHint": 0,
                "commissioningModeInitialStepsInstruction": "",
                "commissioningModeSecondaryStepsHint": 0,
                "commissioningModeSecondaryStepsInstruction": "",
                "creator": "chip project",
                "lsfRevision": 0,
                "lsfUrl": "",
                "productUrl": "https://github.com/project-chip/connectedhomeip/tree/master/examples/all-clusters-app",
                "supportUrl": "https://github.com/project-chip/connectedhomeip/",
                "userManualUrl": "",
                "enhancedSetupFlowOptions": 1,
                "enhancedSetupFlowTCUrl": f"https://{DEFAULT_HOSTNAME}:{DEFAULT_PORT}/tc/65521/32769",
                "enhancedSetupFlowTCRevision": 1,
                "enhancedSetupFlowTCDigest": "",
                "enhancedSetupFlowTCFileSize": 0,
                "enhancedSetupFlowMaintenanceUrl": ""
            }
        }
    }
}


class RESTRequestHandler(http.server.BaseHTTPRequestHandler):
    def __init__(self, *args, **kwargs):
        self.routes = {
            r"/dcl/model/models/(\d+)/(\d+)": self.handle_model_request,
            r"/tc/(\d+)/(\d+)": self.handle_tc_request,
        }
        super().__init__(*args, **kwargs)

    def do_GET(self):
        for pattern, handler in self.routes.items():
            match = re.match(pattern, self.path)
            if match:
                response = handler(*match.groups())
                if response:
                    self.send_response(200)
                    self.send_header("Content-Type", "application/json")
                    self.end_headers()
                    self.wfile.write(json.dumps(response).encode("utf-8"))
                return

        # Handle 404 for unmatched paths
        self.send_response(404)
        self.send_header("Content-Type", "application/json")
        self.end_headers()
        self.wfile.write(json.dumps({"error": "Not found"}).encode("utf-8"))

    def handle_model_request(self, vendor_id, product_id):
        vendor_id = int(vendor_id)
        product_id = int(product_id)
        if vendor_id in MODELS and product_id in MODELS[vendor_id]:
            model = MODELS[int(vendor_id)][int(product_id)]
            # We will return a model that contains the file size and the digest of the TC.
            # Instead of manually setting them, it is calculated on the fly.
            tc = TC[int(vendor_id)][int(product_id)]
            tc_encoded = json.dumps(tc).encode("utf-8")
            sha256_hash = hashlib.sha256(tc_encoded).digest()
            model['model']['enhancedSetupFlowTCFileSize'] = len(tc_encoded)
            model['model']['enhancedSetupFlowTCDigest'] = base64.b64encode(
                sha256_hash).decode("utf-8")

            return model

        return None

    def handle_tc_request(self, vendor_id, product_id):
        vendor_id = int(vendor_id)
        product_id = int(product_id)
        if vendor_id in TC and product_id in TC[vendor_id]:
            return TC[int(vendor_id)][int(product_id)]

        return None


def run_https_server(cert_file="cert.pem", key_file="key.pem"):
    # Creates a basic HTTP server instance that listens on DEFAULT_HOSTNAME and DEFAULT_PORT
    # RESTRequestHandler handles incoming HTTP requests
    httpd = http.server.HTTPServer((DEFAULT_HOSTNAME, DEFAULT_PORT), RESTRequestHandler)

    # Creates an SSL context using TLS protocol for secure communications
    context = ssl.SSLContext(ssl.PROTOCOL_TLS_SERVER)

    # Loads the SSL certificate and private key for the server
    # cert_file: contains the server's public certificate
    # key_file: contains the server's private key
    context.load_cert_chain(certfile=cert_file, keyfile=key_file)

    # Uses a context manager (with statement) to wrap the HTTP server's socket with SSL
    # server_side=True indicates this is a server socket
    # The wrapped socket is automatically closed when exiting the with block
    with context.wrap_socket(httpd.socket, server_side=True) as httpd.socket:
        print(f"Serving on https://{DEFAULT_HOSTNAME}:{DEFAULT_PORT}")
        # Starts the server and runs indefinitely, handling incoming HTTPS requests
        httpd.serve_forever()


# Generate self-signed certificates if needed
def generate_self_signed_cert(cert_file="cert.pem", key_file="key.pem"):
    from subprocess import run
    run([
        "openssl", "req", "-x509", "-nodes", "-days", "365", "-newkey", "rsa:2048",
        "-keyout", key_file, "-out", cert_file,
        "-subj", f"/C=US/ST=Test/L=Test/O=Test/OU=Test/CN={DEFAULT_HOSTNAME}"
    ])


# Check if certificates exist; if not, generate them
if not os.path.exists("cert.pem") or not os.path.exists("key.pem"):
    print("Generating self-signed certificates...")
    generate_self_signed_cert()

# Run the server
run_https_server()
