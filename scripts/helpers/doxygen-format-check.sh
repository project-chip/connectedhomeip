#!/bin/sh

#
#    Copyright (c) 2020 Project CHIP Authors
#    Copyright (c) 2018 Nest Labs, Inc.
#    All rights reserved.
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

# Managed namespace constructs within CHIP ended up seriously
# confusing Doxygen, and I could not find a nicer way to evaluate
# those macros via Doxygen's PREDEFINED directives.  Instead, we
# process the incoming files via this script file.

FILENAME=${1}
# clang format has difficulty recognizing the
# ChipMakeManagedNamespaceIdentifier() macro as a valid namespace;
# temporarily change the namespace name to something more palatable to
# clang-format

sed -e 's:^[ 	]*namespace[ 	]\+ChipMakeManagedNamespaceIdentifier(\([^ 	,()]\+\),[ 	]*kChipManagedNamespaceDesignation_\([^ 	,()]\+\))[ 	]*{[ 	]*$:namespace \1_\2 {:g' \
    -e 's/NS_ENUM(\([_a-zA-Z0-9][_a-zA-Z0-9]*\)[ 	]*,[ 	]*\([_a-zA-Z0-9][_a-zA-Z0-9]*\))/ enum \2 : \1 \2; enum \2 : \1/g' \
    -e 's/__attribute__(([^)]*))//g' \
    -e 's/__attribute__([^)]*)//g' "$FILENAME"
