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

`adb` is bundled inside the app. The `.dmg` does not install `vndroid` in your
`PATH` or the `vndroid` man page. If you built and installed VNdroid yourself,
`vndroid` needs `adb`, accessible from your `PATH`. If you don't have it yet:

```bash
brew install --cask android-platform-tools
```


## Run

_Make sure that your device meets the [prerequisites](/README.md#prerequisites)._

Once installed, open VNdroid from Applications, or run its executable from a
terminal (if you built and installed VNdroid yourself, run `vndroid` instead):

```bash
/Applications/VNdroid.app/Contents/MacOS/vndroid
```

or with arguments (here to disable audio and record to `file.mkv`):

```bash
/Applications/VNdroid.app/Contents/MacOS/vndroid --no-audio --record=file.mkv
```

Documentation for command line arguments is available:
 - `/Applications/VNdroid.app/Contents/MacOS/vndroid --help`
 - the man page source, [vndroid.1](/app/vndroid.1) (`man vndroid` if you built
   and installed VNdroid yourself)
 - in the [documentation](/README.md#documentation)
