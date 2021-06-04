# FileShelter

[![Build Status](https://travis-ci.org/epoupon/fileshelter.svg?branch=master)](https://travis-ci.org/epoupon/fileshelter)

_FileShelter_ is a self-hosted software that allows you to easily share files over the Internet.
Just upload one or more files and get an URL back!

A [demo](http://fileshelter.demo.poupon.io) instance is available, with the following limitations:
- Maximum duration is set to 3 days
- Maximum download limit is set to 10
- Maximum file size is set to 1 MB

A [docker image](https://github.com/paulgalow/fileshelter-docker) is available, thanks to Paul Galow.

## Features
- Period of validity (from one hour to years)
- Customizable interface to set the duration validity
- Optional password protection (download and upload)
- Practically unique links, using [UUID](https://fr.wikipedia.org/wiki/Universal_Unique_Identifier)
- Private links, used to remove the files or to check the download counters
- Terms of service support
- Multiple language support (english, french, german, russian)
- Low memory requirements: the demo instance runs on a _Raspberry Pi Zero W_

Once the expiry date or the download limit is reached, the download is no longer available and the file is deleted.

## Installation

See [INSTALL.md](INSTALL.md) file.

## Contributing

Any feedback is welcome:
* feel free to participate in [discussions](https://github.com/epoupon/fileshelter/discussions) if you have questions,
* report any bug or request for new features in the [issue tracker](https://github.com/epoupon/fileshelter/issues),
* submit your pull requests based on the [develop](../../tree/develop) branch.

