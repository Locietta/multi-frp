# multi-frp

Just a simple wrapper executable that launches multiple frp clients to connect to multiple frp servers.

I need a standalone executable so I can easily detect if my frp forwardings are running with autohotkey...

## Usage

```powershell
./multi-frp <config.json>
```

### Configuration file example:

```json
{
    "frpc": "path/to/frpc",
    "configs": [
        "path/to/config1.toml",
        "path/to/config2.toml"
        // ... add more configs as needed
    ]
}
```