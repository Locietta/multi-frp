# multi-frp

Just a simple wrapper executable that launches multiple frp clients to connect to multiple frp servers. When it dies, it also kills all frp clients it launched.

I need a standalone executable so I can easily detect if my frp forwardings are running and restart them automatically with autohotkey/systemd...

## Installation

Download the latest release from the [releases page](https://github.com/Locietta/multi-frp/releases).

Or you can install it via [Scoop](https://scoop.sh/):

```powershell
scoop bucket add sniffer https://github.com/Locietta/sniffer
scoop install multi-frp
```

### Building from source

You will need:

- A C++23 compatible compiler
  - Windows: VS 2022 or VS 2026 **with clang-cl**
  - linux: gcc 14+ or clang 19+
- [xmake](https://xmake.io/) v3.0+

Then run:

```bash
xmake f -m release -y # configure
xmake                 # build all
# the executable will be located at build/ 
```

## Usage

```powershell
$ multi-frp -h
Usage: multi-frp [--help] [--version] --config CONFIG_FILE

Optional arguments:
  -h, --help                shows help message and exits
  -v, --version             prints version information and exits
  -c, --config CONFIG_FILE  Path to the JSON configuration file [required]
```

### Configuration file example:

```jsonc
{
  "frpc": "path/to/frpc",
  "configs": [
    "path/to/config1.toml",
    "path/to/config2.toml"
    // ... add more configs as needed
  ]
}
```
