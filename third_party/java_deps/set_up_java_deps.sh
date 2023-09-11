#!/usr/bin/env bash

#
# SPDX-FileCopyrightText: 2022-2023 Project CHIP Authors
#
# SPDX-License-Identifier: Apache-2.0
#

set -e

mkdir -p third_party/java_deps/artifacts

function download_jar() {
    _HOST=$1
    _MAVEN_PATH=$2
    _JAR_NAME=$3

    if [ ! -f "third_party/java_deps/artifacts/$_JAR_NAME" ]; then
        curl --fail --location --silent --show-error \
            "https://$_HOST/maven2/$_MAVEN_PATH/$_JAR_NAME" \
            -o "third_party/java_deps/artifacts/$_JAR_NAME"
    fi
}

download_jar "repo1.maven.org" "com/google/code/findbugs/jsr305/3.0.2" "jsr305-3.0.2.jar"
download_jar "repo1.maven.org" "com/google/code/gson/gson/2.9.1" "gson-2.9.1.jar"
download_jar "repo1.maven.org" "com/google/protobuf/protobuf-java/3.22.0" "protobuf-java-3.22.0.jar"
download_jar "repo1.maven.org" "com/google/truth/truth/1.1.3" "truth-1.1.3.jar"
download_jar "repo1.maven.org" "junit/junit/4.13.2" "junit-4.13.2.jar"
download_jar "repo1.maven.org" "org/jetbrains/kotlin/kotlin-stdlib/1.8.10" "kotlin-stdlib-1.8.10.jar"
download_jar "repo1.maven.org" "org/jetbrains/kotlin/kotlin-test/1.8.10" "kotlin-test-1.8.10.jar"
download_jar "repo1.maven.org" "org/json/json/20220924" "json-20220924.jar"

# Unit test requirements
download_jar "repo1.maven.org" "org/hamcrest/hamcrest-all/1.3" "hamcrest-all-1.3.jar"
download_jar "dist.wso2.org" "com/google/common/google-collect/1.0-rc1/" "google-collect-1.0-rc1.jar"
