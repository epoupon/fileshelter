# FileShelter

[![Build Status](https://travis-ci.org/epoupon/fileshelter.svg?branch=master)](https://travis-ci.org/epoupon/fileshelter)

FileShelter is a self-hosted software that allows you to easily share files over the Internet.
Just upload a file and get an URL back!

The aim is to provide a very simple web application, with few package dependencies, so that anyone can install it on his own server.

A [demo](http://fileshelter.demo.poupon.io) instance is available, with the following limitations:
- Maximum duration is set to 3 days
- Maximum download limit is set to 10
- Maximum file size is set to 1 MB

A [docker image](https://github.com/paulgalow/fileshelter-docker) is available, thanks to Paul Galow.

## Features
- Multiple file upload (a zip is created server side)
- Period of validity (from one hour to years)
- Optional download limit
- Customizable interface to allow or to prevent users to set the download limit or the duration validity
- Optional password protection (download and upload)
- Practically unique links, using [UUID](https://fr.wikipedia.org/wiki/Universal_Unique_Identifier)
- Private link, used to remove the file or to check the download counter
- Terms of service support
- Multiple language support (english, french, german, russian)

Once the expiry date or the download limit is reached, the download is no longer available and the file is deleted.

## Installation
### From Source
#### Debian/Ubuntu dependencies
```sh
apt-get install build-essential autoconf automake libboost-dev libconfig++-dev libzip-dev
```

You also need wt4, that is not packaged yet on Debian. See [installation instructions](https://www.webtoolkit.eu/wt/doc/reference/html/InstallationUnix.html).

#### Build
```sh
git clone https://github.com/epoupon/fileshelter.git fileshelter
cd fileshelter
autoreconf -vfi
mkdir build
cd build
../configure --prefix=/usr --sysconfdir=/etc
```
configure will complain if a mandatory library is missing.

```sh
make
```

```sh
make install
```
This last command requires root privileges.


## Configuration
FileShelter uses a configuration file, installed in '/etc/fileshelter.conf'.
It is recommended to edit this file and change the relevant settings (working directory, maximum file size, maximum validity duration, listen port, etc.)

A basic "Terms of Services" is provided. The configuration file contains the definition of the fields needed by the default tos.
You may also specify an alternate tos file to fit your needs.

It is highly recommended to run fileshelter as a non root user. Therefore make sure the user has write permissions on the working directory.

### Reverse proxy settings
You have to set the 'behind-reverse-proxy' option to 'true' in the configuration file.

Here is an example to make FileShelter properly work on myserver.org using nginx.
```
server {
    listen 80;

    server_name myserver.org;

    access_log            /var/log/nginx/myserver.access.log;

    proxy_request_buffering off;
    proxy_buffering off;
    proxy_buffer_size 4k;

    location / {

      proxy_set_header        Host $host;
      proxy_set_header        X-Real-IP $remote_addr;
      proxy_set_header        X-Forwarded-For $proxy_add_x_forwarded_for;
      proxy_set_header        X-Forwarded-Proto $scheme;

      proxy_pass          http://localhost:5091;
      proxy_read_timeout  120;
    }
}
```

## Running
```sh
fileshelter
```
Logs are output in the working directory, in the files 'fileshelter.log' and 'fileshelter.access.log'

Alternatively, you may want to specify another configuration file:
```sh
fileshelter /another/config/file
```

To connect to FileShelter, just open your favorite browser and go to http://localhost:5091

## Credits
- Wt, awesome framework: http://www.webtoolkit.eu/
- Bootstrap: http://getbootstrap.com/


