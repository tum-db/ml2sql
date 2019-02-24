FROM ubuntu:18.04
MAINTAINER Maximilian E. Schüle <m.schuele@tum.de>
# how many cores to use for compilation
ARG BUILD_CORES
# update
RUN apt-get update
# Install some tools
RUN apt-get install -y cmake g++ wget uuid-dev unzip pkgconf git python xxd default-jre
## Install antlr
RUN cd /tmp && wget http://www.antlr.org/download/antlr4-cpp-runtime-4.7.1-source.zip && wget http://www.antlr.org/download/antlr-4.7.1-complete.jar  && \
   mv antlr-4.7.1-complete.jar /usr/local/lib/antlr-4.7.1-complete.jar && unzip antlr4-cpp-runtime-4.7.1-source.zip -d antlr4-cpp-runtime && \
   cd antlr4-cpp-runtime && mkdir build && mkdir run && cd build && cmake -DCMAKE_BUILD_TYPE=Release .. && make -j${BUILD_CORES} && make install && \
   LD_LIBRARY_PATH=/usr/local/lib && export LD_LIBRARY_PATH && ldconfig -v
## Install pistache
RUN cd /tmp/ && git clone https://github.com/oktal/pistache.git && cd pistache && git submodule update --init && \
   mkdir build && cd build && cmake -G "Unix Makefiles" -DCMAKE_BUILD_TYPE=Release .. && make -j${BUILD_CORES} && make install && \
   LD_LIBRARY_PATH=/usr/local/lib && export LD_LIBRARY_PATH && ldconfig -v
# Run the rest as non root user
RUN useradd -ms /bin/bash dockeruser
USER dockeruser
WORKDIR /home/dockeruser
# copy work dir
RUN mkdir ./ml2sql
COPY --chown=dockeruser . ./ml2sql/
# Install ml2sql
RUN cd ./ml2sql && rm -rf build && mkdir build && cd ./build && cmake .. && make -j${BUILD_CORES}
# Run
EXPOSE 5000
CMD ["/home/dockeruser/ml2sql/build/ml2sqlserver"]
