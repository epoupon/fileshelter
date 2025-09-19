- [Installation](#installation)
  - [From Docker images](#from-docker-images)
  - [From Debian packages](#from-debian-packages)
  - [From source](#from-source)
- [Deployment](#deployment)
  - [Configuration](#configuration)
  - [Reverse proxy settings](#reverse-proxy-settings)
  - [Run](#run)

# Installation
## From Docker images
Official _Docker_ images are available, please see detailed instructions at https://hub.docker.com/r/epoupon/fileshelter.

## From Debian packages
_Trixie_ packages are provided for _amd64_ architectures.
As root, trust the following debian package provider and add it in your list of repositories:
```sh
wget https://debian.poupon.dev/apt/debian/epoupon.gpg -P /usr/share/keyrings
echo "deb [signed-by=/usr/share/keyrings/epoupon.gpg] https://debian.poupon.dev/apt/debian trixie main" > /etc/apt/sources.list.d/epoupon.list
```
To install or upgrade _Fileshelter_:
```sh
apt update
apt install fileshelter
```
The `fileshelter` service is started just after the package installation, run by a dedicated `fileshelter` system user.</br>
Please refer to [Deployment](#deployment) for further configuration options.
## From source
__Note__: this installation process and the default values of the configuration files have been written for _Debian Bookworm_. Therefore, you may have to adapt commands and/or paths in order to fit to your distribution.
### Debian/Ubuntu dependencies
__Note__: a C++17 compiler is needed to compile _Fileshelter_
```sh
apt-get install build-essential cmake libboost-dev libconfig++-dev libarchive-dev
```

You also need _Wt4_, that is not packaged on _Debian_. See [installation instructions](https://www.webtoolkit.eu/wt/doc/reference/html/InstallationUnix.html).

### Build
```sh
git clone https://github.com/epoupon/fileshelter.git fileshelter
cd fileshelter
mkdir build
cd build
cmake .. -DCMAKE_INSTALL_PREFIX=/usr -DCMAKE_BUILD_TYPE=Release
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

Create the working directory and give it access to the `fileshelter` user:
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

A basic _Terms of Service_ is provided. The configuration file contains the definition of the fields needed by the default ToS.
You may also specify an alternate ToS file to fit your needs.

If a setting is not present in the configuration file, a hardcoded default value is used (the same as in the [default.conf](conf/fileshelter.conf) file)

## Reverse proxy settings
_Fileshelter_ is shipped with an embedded web server, but it is recommended to deploy behind a reverse proxy.
You have to set the `behind-reverse-proxy` option to `true` in the `fileshelter.conf` configuration file and to adjust the trusted proxy list in `trusted-proxies`.

__Note__: when running in a docker environment, you have to trust the docker gateway IP (which is `172.17.0.1` by default)

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

Logs can be accessed using `journalctl`:
```sh
journalctl -u fileshelter.service
```

To connect to _Fileshelter_, just open your favorite browser and go to http://localhost:5091

