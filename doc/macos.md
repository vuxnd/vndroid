# On macOS

## Install

Releases are shipped as a `.dmg`: open it, then drag VNdroid to Applications.

The app is not notarized, so macOS refuses to open it the first time. Open
System Settings > Privacy & Security, scroll down, click "Open Anyway", then
launch the app again. Or, in a terminal:

```bash
xattr -d com.apple.quarantine /Applications/VNdroid.app
```

Alternatively, see [build.md](build.md) to build and install the app.

`adb` is bundled inside the app. To run `vndroid` from a terminal, you need
`adb`, accessible from your `PATH`. If you don't have it yet:

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
