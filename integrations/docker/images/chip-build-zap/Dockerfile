FROM alpine:3.14

RUN apk add --no-cache \
  nodejs=14.18.1-r0 \
  openjdk11=11.0.14_p9-r0 \
  npm=7.17.0-r0 \
  python3=3.9.5-r2 \
  pixman-dev=0.40.0-r2 \
  cairo-dev=1.16.0-r3 \
  pango-dev=1.48.5-r0 \
  make=4.3-r0 \
  g++=10.3.1_git20210424-r2 \
  libjpeg-turbo-dev=2.1.0-r0 \
  giflib-dev=5.2.1-r0 \
  git=2.32.0-r0 \
  clang-dev=11.1.0-r1 \
  py3-pip=20.3.4-r1

# Generator dependencies
RUN pip3 install --no-cache-dir \
    click==8.0.3                \
    coloredlogs==15.0.1         \
    jinja2==3.0.3               \
    lark==1.0.0                 \
    stringcase==1.2.0
