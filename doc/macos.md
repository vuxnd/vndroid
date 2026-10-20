# On macOS

## Install

See [build.md](build.md) to build and install the app.

You need `adb`, accessible from your `PATH`. If you don't have it yet:

```bash
brew install --cask android-platform-tools
```


## Run

_Make sure that your device meets the [prerequisites](/README.md#prerequisites)._

Once installed, run from a terminal:

```bash
vndroid
```

or with arguments (here to disable audio and record to `file.mkv`):

```bash
vndroid --no-audio --record=file.mkv
```

Documentation for command line arguments is available:
 - `man vndroid`
 - `vndroid --help`
 - in the [documentation](/README.md#documentation)
