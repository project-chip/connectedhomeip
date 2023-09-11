#
# SPDX-FileCopyrightText: 2020 Project CHIP Authors
#
# SPDX-License-Identifier: Apache-2.0
#

FROM generic_node_image
RUN apt-get install -y libglib2.0
COPY out/debug/chip-all-clusters-app /usr/bin/
COPY entrypoint.sh /

ENTRYPOINT ["/entrypoint.sh", "server"]
