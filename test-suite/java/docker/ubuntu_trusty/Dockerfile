FROM ubuntu:trusty

RUN apt-get update
RUN apt-get -y install build-essential clang-3.5 cmake

RUN apt-get -y install software-properties-common

# djinni requires some stdc++ 4.9 features, e.g. experimental/optional
RUN add-apt-repository ppa:ubuntu-toolchain-r/test && apt-get update
RUN apt-get install -y libstdc++-4.9-dev

# TODO: replace with official openjdk-8-jdk once they post the backport
RUN add-apt-repository ppa:openjdk-r/ppa && apt-get update
RUN apt-get install -y openjdk-8-jdk ant

# CMake 2.8.12 FindJNI is slightly broken for OpenJDK 8; let's help it find Java
ENV JAVA_HOME /usr/lib/jvm/java-8-openjdk-amd64/

ENV CXX clang++
RUN ln -s /usr/bin/clang++{-3.5,}

VOLUME /opt/djinni
CMD cd /opt/djinni/test-suite/java && ant -v test

