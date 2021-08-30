FROM	alpine:3.13 AS build

WORKDIR	/tmp/workdir

ARG	MAKEFLAGS="-j2"

ARG	PREFIX="/tmp/install"

ARG	BUILD_PACKAGES=" \
	ca-certificates \
	curl \
	bzip2 \
	pkgconfig \
	coreutils \
	libtool \
	g++ \
	make \
	curl \
	cmake \
	zlib-dev \
	openssl-dev \
	boost-dev \
	libconfig-dev \
	gtest-dev"

RUN	apk add --no-cache --update ${BUILD_PACKAGES}

# WT
ARG	WT_VERSION=4.5.0
RUN \
	DIR=/tmp/wt && mkdir -p ${DIR} && cd ${DIR} && \
	curl -sLO https://github.com/emweb/wt/archive/${WT_VERSION}.tar.gz && \
	tar -x --strip-components=1 -f ${WT_VERSION}.tar.gz

RUN \
	DIR=/tmp/wt && mkdir -p ${DIR} && cd ${DIR} && \
	cmake -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX=${PREFIX} -DBUILD_EXAMPLES=OFF -DENABLE_LIBWTTEST=OFF -DCONNECTOR_FCGI=OFF && \
	make && \
	make install

# Fileshelter
COPY . /tmp/fileshelter/

RUN \
	DIR=/tmp/fileshelter && mkdir -p ${DIR} && cd ${DIR} && \
	PKG_CONFIG_PATH=/tmp/install/lib/pkgconfig CXXFLAGS="-I${PREFIX}/include" LDFLAGS="-L${PREFIX}/lib -Wl,--rpath-link=${PREFIX}/lib" cmake . -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX=${PREFIX} -DCMAKE_PREFIX_PATH=${PREFIX} && \
	LD_LIBRARY_PATH=${PREFIX}/lib VERBOSE=1 make && \
	LD_LIBRARY_PATH=${PREFIX}/lib make test && \
	make install && \
	mkdir -p ${PREFIX}/etc/ && \
	cp conf/fileshelter.conf ${PREFIX}/etc

# Now copy all the stuff installed in a new folder (/tmp/fakeroot/)
RUN \
	mkdir -p /tmp/fakeroot/bin && \
	for bin in ${PREFIX}/bin/fileshelter*; \
	do \
		strip --strip-all $bin && \
		cp $bin /tmp/fakeroot/bin/; \
	done && \
	for lib in ${PREFIX}/lib/*.so; \
	do \
		strip --strip-all $lib; \
	done && \
	cp -r ${PREFIX}/lib /tmp/fakeroot/lib && \
	cp -r ${PREFIX}/share /tmp/fakeroot/share && \
	rm -rf /tmp/fakeroot/share/doc && \
	rm -rf /tmp/fakeroot/share/man

## Release Stage
FROM		alpine:3.13 AS release
LABEL		maintainer="Emeric Poupon <emeric.poupon@laposte.net>"
LABEL		version=${FILESHELTER_VERSION}

ARG	RUNTIME_PACKAGES=" \
	libssl1.1 \
	libcrypto1.1 \
	zlib \
	boost-filesystem \
	boost-program_options \
	boost-system \
	boost-thread \
	libconfig++"

ARG	FILESHELTER_USER=fileshelter
ARG	FILESHELTER_GROUP=fileshelter

RUN	apk add --no-cache --update ${RUNTIME_PACKAGES}

RUN	addgroup -S ${FILESHELTER_GROUP} && \
	adduser -S -H ${FILESHELTER_USER} && \
	adduser ${FILESHELTER_USER} ${FILESHELTER_GROUP} && \
	mkdir -p /var/fileshelter && chown -R ${FILESHELTER_USER}:${FILESHELTER_GROUP} /var/fileshelter

VOLUME	/var/fileshelter

USER	${FILESHELTER_USER}:${FILESHELTER_GROUP}

COPY	--from=build /tmp/fakeroot/ /usr
COPY	--from=build /tmp/fakeroot/share/fileshelter/fileshelter.conf /etc/fileshelter.conf

EXPOSE	5091

ENTRYPOINT	["/usr/bin/fileshelter"]

