FROM centos:6

# Get and select Java 8 (64-bit)
RUN yum install -y java-1.8.0-openjdk-devel
ENV JAVA_HOME /usr/lib/jvm/java-1.8.0-openjdk

# Get other build utils
RUN yum install -y cmake wget tar ant

# djinni requires llvm with libstdc++ 4.9 features, 
# e.g. experimental/optional, so we need a modern
# compiler.  Let's get gcc 4.9 from Scientific Linux Cern
RUN yum install -y wget
WORKDIR /etc/yum.repos.d
RUN wget http://linuxsoft.cern.ch/cern/scl/slc6-scl.repo
RUN yum -y --nogpgcheck install devtoolset-3-gcc devtoolset-3-gcc-c++

# Make devtoolset's gcc accessible
ENV PATH /opt/rh/devtoolset-3/root/usr/bin:/usr/local/sbin:/usr/local/bin:/usr/sbin:/usr/bin:/sbin:/bin

VOLUME /opt/djinni
CMD cd /opt/djinni/test-suite/java && ant -v test && ant clean 

