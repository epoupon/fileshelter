# FileShelter
FileShelter is a self-hosted software that allows you to easily share files over the Internet.
Just upload a file and get an URL back!

The aim is to provide a very simple web application, with few package dependencies, so that anyone can install it on his own server.

A limited [Demo](http://demo.fileshelter.suroot.com) instance is available.

## Features
- Expiry date
- Download limit
- Password protection
- Practically unique links, using [UUID](https://fr.wikipedia.org/wiki/Universal_Unique_Identifier)
- Private link, used to remove the file or to check the download counter
- Basic terms of service

Once the expiry date or the download limit is reached, the download is no longer available and the file is deleted.

## Installation
### Debian dependencies
```sh
$ apt-get install build-essential autoconf automake libboost-dev libwtdbosqlite-dev libwthttp-dev libwtdbo-dev libwt-dev libconfig++-dev
```

### Build
```sh
$ git clone https://github.com/epoupon/fileshelter.git fileshelter
$ cd fileshelter
$ autoreconf -vfi
$ mkdir build
$ cd build
$ ../configure --prefix=/usr --sysconfdir=/etc
```
configure will complain if a mandatory library is missing.

```sh
$ make
```

```sh
$ make install
```
This last command requires root privileges.

## Configuration
FileShelter uses a configuration file, installed in '/etc/fileshelter.conf'
It is recommended to edit this file and change the relevant settings (maximum file size, maximum validity duration, etc.)

A basic "Terms of Services" is provided, located in '/usr/share/fileshelter/approot/tos.xml'. You may modify it to fir your needs.
Do not forget to change the organization name ("msg-tos-org") and the site's URL ("msg-tos-url").

It is highly recommended to run fileshelter as a non root user. Therefore make sure the user has write permissions on the working directory.

## Running
```sh
$ fileshelter
```
Logs are output in the working directory, in the file 'fileshelter.log'

Alternatively, you may want to specify another configuration file:
```sh
$ fileshelter /another/config/file
```

## Credits

- Wt, awesome framework: http://www.webtoolkit.eu/
- Bootstrap: http://getbootstrap.com/


