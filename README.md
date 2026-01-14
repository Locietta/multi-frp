# multi-frp

Just a simple wrapper executable that launches multiple frp clients to connect to multiple frp servers. When it dies, it also kills all frp clients it launched.

I need a standalone executable so I can easily detect if my frp forwardings are running and restart them automatically with autohotkey/systemd...

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