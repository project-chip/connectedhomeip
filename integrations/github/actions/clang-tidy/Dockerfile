FROM ubuntu:xenial

COPY run_clang.sh /run_clang.sh
RUN chmod +x /run_clang.sh
RUN apt update
RUN apt-get install -fy clang clang-format clang-tidy wget curl jq

ENTRYPOINT ["/run_clang.sh"]
