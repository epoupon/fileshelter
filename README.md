# FileShelter
[![Last Release](https://img.shields.io/github/v/release/epoupon/fileshelter?logo=github&label=latest)](https://github.com/epoupon/fileshelter/releases)

_FileShelter_ is a self-hosted software that allows you to easily share files over the Internet.
Just upload one or more files and get an URL back!

A [demo](http://fileshelter.demo.poupon.io) instance is available, with the following limitations:
- Maximum duration is set to 3 days
- Maximum file size is set to 1 MB

## Features
- Period of validity, from one hour to many years
- Optional password protection (download and/or upload)
- Practically unique links, using [UUID](https://fr.wikipedia.org/wiki/Universal_Unique_Identifier)
- Private edit links, used to remove the files or to check the download counters
- Terms Of Service support, fully or partially customizable
- Multiple language support
- Low memory requirements: the demo instance runs on a _Raspberry Pi Zero W_

Once the expiry date is reached, the share is no longer available for download. The files are actually deleted roughly two hours after the share has expired. This is to make sure to not interrupt a download in progress.

## Command line tools
* `fileshelter-list`: list all the shares available for download
* `fileshelter-create`: create a share using local files. The files are _not_ copied in the _Fileshelter_'s working directory. Therefore the files must still exist while the share is avalaible for download. The files are _not_ deleted once the share has expired.

## Installation
See [INSTALL.md](INSTALL.md) file.

## Contributing
Any feedback is welcome:
* feel free to participate in [discussions](https://github.com/epoupon/fileshelter/discussions) if you have questions,
* report any bug or request for new features in the [issue tracker](https://github.com/epoupon/fileshelter/issues),
* submit your pull requests based on the [develop](../../tree/develop) branch.

