FROM debian:jessie

# Use Java 8 backport
RUN echo "deb http://http.debian.net/debian jessie-backports main" >> /etc/apt/sources.list

RUN apt-get update

RUN apt-get -y install build-essential clang llvm cmake
RUN apt-get install -y openjdk-8-jdk ant

# Select Java 8
RUN update-java-alternatives -s java-1.8.0-openjdk-amd64
RUN rm /usr/lib/jvm/default-java
RUN ln -s /usr/lib/jvm/java-8-openjdk-amd64 /usr/lib/jvm/default-java

# CMake 3.0.2 FindJNI is slightly broken for Debian Jessie; let's help it find Java
ENV JAVA_HOME /usr/lib/jvm/java-8-openjdk-amd64/

ENV CXX clang++

VOLUME /opt/djinni
CMD cd /opt/djinni/test-suite/java && ant -v test && ant clean 

