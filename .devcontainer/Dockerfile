FROM ubuntu:bionic


ARG USERNAME=vscode
ARG USER_UID=1000
ARG USER_GID=$USER_UID
ENV LANG en_US.utf8

RUN apt-get update
RUN apt-get install -y locales && localedef -i en_US -c -f UTF-8 -A /usr/share/locale/locale.alias en_US.UTF-8
RUN apt-get -fy install git vim emacs sudo \
    apt-utils dialog \
    iproute2 procps lsb-release \
    build-essential cmake cppcheck valgrind \ 
    make automake g++ libtool llvm-9-dev llvm-9-tools llvm-9 llvm-9-runtime \
    curl \
    wget \
    jq \
    clang-9 \
    clang-format-9 \
    clang-tidy-9 \
    lcov \
    shellcheck \
    gcc-multilib \
    libfftw3-dev

RUN groupadd --gid $USER_GID $USERNAME
RUN useradd -s /bin/bash --uid $USER_UID --gid $USER_GID -m $USERNAME
RUN echo $USERNAME ALL=\(root\) NOPASSWD:ALL > /etc/sudoers.d/$USERNAME
RUN chmod 0440 /etc/sudoers.d/$USERNAME

RUN ln -s /usr/bin/clang-format-9 /usr/local/bin/clang-format


RUN mkdir -p /var/bsim
WORKDIR /var/bsim

RUN cd /var/bsim
RUN curl https://storage.googleapis.com/git-repo-downloads/repo > ./repo  && chmod a+x ./repo
RUN ./repo init -u https://github.com/BabbleSim/manifest.git -m everything.xml -b master
RUN ./repo sync
RUN make everything -j 8
