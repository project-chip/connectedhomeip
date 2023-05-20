#!/usr/bin/env bash

#
#    Copyright (c) 2022-2023 Project CHIP Authors
#
#    Licensed under the Apache License, Version 2.0 (the "License");
#    you may not use this file except in compliance with the License.
#    You may obtain a copy of the License at
#
#        http://www.apache.org/licenses/LICENSE-2.0
#
#    Unless required by applicable law or agreed to in writing, software
#    distributed under the License is distributed on an "AS IS" BASIS,
#    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
#    See the License for the specific language governing permissions and
#    limitations under the License.
#

mkdir -p third_party/java_deps/artifacts
curl --fail --location --silent --show-error https://repo1.maven.org/maven2/com/google/code/findbugs/jsr305/3.0.2/jsr305-3.0.2.jar -o third_party/java_deps/artifacts/jsr305-3.0.2.jar
curl --fail --location --silent --show-error https://repo1.maven.org/maven2/org/json/json/20220924/json-20220924.jar -o third_party/java_deps/artifacts/json-20220924.jar
curl --fail --location --silent --show-error https://repo1.maven.org/maven2/org/jetbrains/kotlin/kotlin-stdlib/1.8.10/kotlin-stdlib-1.8.10.jar -o third_party/java_deps/artifacts/kotlin-stdlib-1.8.10.jar
curl --fail --location --silent --show-error https://repo1.maven.org/maven2/org/jetbrains/kotlin/kotlin-test/1.8.10/kotlin-test-1.8.10.jar -o third_party/java_deps/artifacts/kotlin-test-1.8.10.jar
curl --fail --location --silent --show-error https://repo1.maven.org/maven2/com/google/protobuf/protobuf-java/3.22.0/protobuf-java-3.22.0.jar -o third_party/java_deps/artifacts/protobuf-java-3.22.0.jar
curl --fail --location --silent --show-error https://repo1.maven.org/maven2/com/google/truth/truth/1.1.3/truth-1.1.3.jar -o third_party/java_deps/artifacts/truth-1.1.3.jar
curl --fail --location --silent --show-error https://repo1.maven.org/maven2/junit/junit/4.13.2/junit-4.13.2.jar -o third_party/java_deps/artifacts/junit-4.13.2.jar
curl --fail --location --silent --show-error https://repo1.maven.org/maven2/com/google/code/gson/gson/2.9.1/gson-2.9.1.jar -o third_party/java_deps/artifacts/gson-2.9.1.jar
