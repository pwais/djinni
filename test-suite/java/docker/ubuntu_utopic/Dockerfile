FROM ubuntu:utopic-20150625

RUN apt-get update
RUN apt-get -y install git build-essential clang cmake

RUN apt-get install -y openjdk-8-jdk ant

ENV CXX clang++

# CMake 2.8.12 FindJNI is slightly broken for OpenJDK 8; let's help it find Java
ENV JAVA_HOME /usr/lib/jvm/java-8-openjdk-amd64/

VOLUME /opt/djinni
CMD cd /opt/djinni/test-suite/java && ant -v test && rm -rf /opt/djinni/test-suite/java/build 

