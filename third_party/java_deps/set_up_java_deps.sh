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

set -e

mkdir -p third_party/java_deps/artifacts

function download_maven_jar() {
    _MAVEN_PATH=$1
    _JAR_NAME=$2

    if [ ! -f "third_party/java_deps/artifacts/$_JAR_NAME" ]; then
        curl --fail --location --silent --show-error \
            "https://repo1.maven.org/maven2/$_MAVEN_PATH/$_JAR_NAME" \
            -o "third_party/java_deps/artifacts/$_JAR_NAME"
    fi
}

download_maven_jar "com/google/code/findbugs/jsr305/3.0.2" "jsr305-3.0.2.jar"
download_maven_jar "org/json/json/20220924" "json-20220924.jar"
download_maven_jar "org/jetbrains/kotlin/kotlin-stdlib/1.8.10" "kotlin-stdlib-1.8.10.jar"
download_maven_jar "org/jetbrains/kotlin/kotlin-test/1.8.10" "kotlin-test-1.8.10.jar"
download_maven_jar "com/google/protobuf/protobuf-java/3.22.0" "protobuf-java-3.22.0.jar"
download_maven_jar "com/google/truth/truth/1.1.3" "truth-1.1.3.jar"
download_maven_jar "junit/junit/4.13.2" "junit-4.13.2.jar"
download_maven_jar "com/google/code/gson/gson/2.9.1" "gson-2.9.1.jar"
