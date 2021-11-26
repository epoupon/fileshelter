- [Installation](#installation)
  - [From Docker images](#from-docker-images)
  - [From packages](#from-packages)
  - [From source](#from-source)
- [Deployment](#deployment)
  - [Configuration](#configuration)
  - [Reverse proxy settings](#reverse-proxy-settings)
  - [Run](#run)

# Installation
## From Docker images
Official _Docker_ images are available, please see detailed instructions at https://hub.docker.com/r/epoupon/fileshelter.

## From packages
### Debian Buster packages

_Buster_ packages are provided for _amd64_ and _armhf_ architectures.

As root, trust the following debian package provider and add it in your list of repositories:
```sh
wget -O- https://debian.poupon.io/apt/debian/epoupon.gpg.key | apt-key add -
echo "deb https://debian.poupon.io/apt/debian buster main" > /etc/apt/sources.list.d/epoupon.list
```

To install or upgrade _fileshelter_:
```sh
apt update
apt install fileshelter
```

The _fileshelter_ service is started just after the package installation, run by a dedicated _fileshelter_ system user.</br>
Please refer to [Deployment](#deployment) for further configuration options.

## From source
__Note__: this installation process and the default values of the configuration files have been written for _Debian Buster_. Therefore, you may have to adapt commands and/or paths in order to fit to your distribution.
### Debian/Ubuntu dependencies
__Note__: a C++17 compiler is needed to compile _fileshelter_
```sh
apt-get install build-essential cmake libboost-dev libconfig++-dev
```

You also need _Wt4_, that is not packaged yet on _Debian_. See [installation instructions](https://www.webtoolkit.eu/wt/doc/reference/html/InstallationUnix.html).

### Build
```sh
git clone https://github.com/epoupon/fileshelter.git fileshelter
cd fileshelter
mkdir build
cd build
cmake .. -DCMAKE_BUILD_TYPE=Release
```
cmake will complain if a mandatory library is missing.

__Note__: you can customize the installation directory using `-DCMAKE_INSTALL_PREFIX=path` (defaults to `/usr/local`).

```sh
make
```
__Note__: you can use `make -jN` to speed up compilation time (N is the number of compilation workers to spawn).

### Installation

__Note__: the commands of this section require root privileges.

```sh
make install
```

Create a dedicated system user:
```sh
useradd --system --group fileshelter
```

Copy the configuration files:
```sh
cp /usr/share/fileshelter/fileshelter.conf /etc/fileshelter.conf
cp /usr/share/fileshelter/fileshelter.service /lib/systemd/system/fileshelter.service
```

Create the working directory and give it access to the _fileshelter_ user:
```sh
mkdir /var/fileshelter
chown -R fileshelter:fileshelter /var/fileshelter
chmod -R 770 /var/fileshelter
```

To make _Fileshelter_ run automatically during startup:
```sh
systemctl enable fileshelter
```

### Upgrade

To upgrade _FileShelter_ from source, you need to update the master branch and rebuild/install it:
```sh
cd build
git pull
make
```

Then using root privileges:
```sh
make install
systemctl restart fileshelter
```

# Deployment

## Configuration
_Fileshelter_ uses a configuration file, installed by default in `/etc/fileshelter.conf`. It is recommended to edit this file and change relevant settings (listen address, listen port, working directory, ...).

A basic _Terms of Services_ is provided. The configuration file contains the definition of the fields needed by the default tos.
You may also specify an alternate tos file to fit your needs.

If a setting is not present in the configuration file, a hardcoded default value is used (the same as in the [default.conf](conf/fileshelter.conf) file)

## Reverse proxy settings
_Fileshelter_ is shipped with an embedded web server, but it is recommended to deploy behind a reverse proxy. You have to set the _behind-reverse-proxy_ option to _true_ in the `fileshelter.conf` configuration file.

Here is an example to make _Fileshelter_ properly work on _myserver.org_ using _nginx_:
```
server {
    listen 80;

    server_name myserver.org;

    access_log            /var/log/nginx/myserver.access.log;

    proxy_request_buffering off;
    proxy_buffering off;
    proxy_buffer_size 4k;
    client_max_body_size 100M; # Make the number the same as 'max-share-size' in fileshelter.conf

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

## Run
```sh
systemctl start fileshelter
```

Log traces can be accessed using journactl:
```sh
journalctl -u fileshelter.service
```

To connect to _Fileshelter_, just open your favorite browser and go to http://localhost:5091

