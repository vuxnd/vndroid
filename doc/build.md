# Build VNdroid

Here are the instructions to build _VNdroid_ (client and server).


## Requirements

You need [adb]. It is available in the [Android SDK platform
tools][platform-tools], or packaged in your distribution (`adb`).

On Windows, download the [platform-tools][platform-tools-windows] and extract
the following files to a directory accessible from your `PATH`:
 - `adb.exe`
 - `AdbWinApi.dll`
 - `AdbWinUsbApi.dll`

The client requires [FFmpeg] and [SDL]. Just follow the instructions.

[adb]: https://developer.android.com/studio/command-line/adb.html
[platform-tools]: https://developer.android.com/studio/releases/platform-tools.html
[platform-tools-windows]: https://dl.google.com/android/repository/platform-tools-latest-windows.zip
[ffmpeg]: https://en.wikipedia.org/wiki/FFmpeg
[SDL]: https://en.wikipedia.org/wiki/Simple_DirectMedia_Layer



## System-specific steps

### Linux

Install the required packages from your package manager.

#### Debian/Ubuntu

```bash
# runtime dependencies
sudo apt install ffmpeg libsdl3-0 adb libusb-1.0-0 va-driver-all

# client build dependencies
sudo apt install gcc git pkg-config meson ninja-build libsdl3-dev \
                 libavcodec-dev libavdevice-dev libavformat-dev libavutil-dev \
                 libswresample-dev libusb-1.0-0-dev libv4l-dev libdrm-dev

# server build dependencies
sudo apt install openjdk-17-jdk
```

On old versions (like Ubuntu 16.04), `meson` is too old. In that case, install
it from `pip3`:

```bash
sudo apt install python3-pip
pip3 install meson
```


#### Fedora

```bash
# enable RPM fusion free
sudo dnf install https://download1.rpmfusion.org/free/fedora/rpmfusion-free-release-$(rpm -E %fedora).noarch.rpm

# client build dependencies
sudo dnf install SDL3-devel ffms2-devel libusb1-devel libavdevice-free-devel \
                 libdrm-devel meson gcc make

# server build dependencies
sudo dnf install java-devel
```



### Windows

#### Cross-compile from Linux

This is the preferred method (and the way the release is built).

From _Debian_, install _mingw_:

```bash
sudo apt install mingw-w64 mingw-w64-tools libz-mingw-w64-dev
```

You also need the JDK to build the server:

```bash
sudo apt install openjdk-17-jdk
```

Then generate the releases:

```bash
./release.sh
```

It will generate win32 and win64 releases into `dist/`.


#### In MSYS2

From Windows, you need [MSYS2] to build the project. From an MSYS2 terminal,
install the required packages:

[MSYS2]: http://www.msys2.org/

```bash
# runtime dependencies
pacman -S mingw-w64-x86_64-sdl3 \
          mingw-w64-x86_64-ffmpeg \
          mingw-w64-x86_64-libusb

# client build dependencies
pacman -S mingw-w64-x86_64-make \
          mingw-w64-x86_64-gcc \
          mingw-w64-x86_64-pkg-config \
          mingw-w64-x86_64-meson
```

For a 32 bits version, replace `x86_64` by `i686`:

```bash
# runtime dependencies
pacman -S mingw-w64-i686-sdl3 \
          mingw-w64-i686-ffmpeg \
          mingw-w64-i686-libusb

# client build dependencies
pacman -S mingw-w64-i686-make \
          mingw-w64-i686-gcc \
          mingw-w64-i686-pkg-config \
          mingw-w64-i686-meson
```

Java (>= 7) is not available in MSYS2, so if you plan to build the server,
install it manually and make it available from the `PATH`:

```bash
export PATH="$JAVA_HOME/bin:$PATH"
```

When following the rest of the build instructions below, make sure you use the
MinGW terminal within MSYS2.


### macOS

Install the packages with [Homebrew]:

[Homebrew]: https://brew.sh/

```bash
# runtime dependencies
brew install sdl3 ffmpeg libusb

# client build dependencies
brew install pkg-config meson
```

Additionally, if you want to build the server, install Java 17 from Caskroom, and
make it available from the `PATH`:

```bash
brew install openjdk@17
export JAVA_HOME="$(/usr/libexec/java_home --version 1.17)"
export PATH="$JAVA_HOME/bin:$PATH"
```


## Common steps

**As a non-root user**, clone this repository and `cd` into it.


### Build

Install the [Android SDK] (_Android Studio_), and set `ANDROID_SDK_ROOT` to its
directory. For example:

[Android SDK]: https://developer.android.com/studio/index.html

```bash
# Linux
export ANDROID_SDK_ROOT=~/Android/Sdk
# Mac
export ANDROID_SDK_ROOT=~/Library/Android/sdk
# Windows
set ANDROID_SDK_ROOT=%LOCALAPPDATA%\Android\sdk
```

Then, build:

```bash
meson setup x --buildtype=release --strip -Db_lto=true
ninja -Cx  # DO NOT RUN AS ROOT
```

_Note: `ninja` must be run as a non-root user (only `ninja
install` must be run as root)._


### Run without installing:

```bash
./run x [options]
```


### Install

After a successful build, you can install _VNdroid_ on the system:

```bash
sudo ninja -Cx install    # without sudo on Windows
```

This installs several files:

 - `/usr/local/bin/vndroid` (main app)
 - `/usr/local/share/vndroid/vndroid-server` (server to push to the device)
 - `/usr/local/share/man/man1/vndroid.1` (manpage)
 - `/usr/local/share/icons/hicolor/256x256/apps/vndroid.png` (app icon)
 - `/usr/local/share/icons/hicolor/256x256/apps/disconnected.png` (device
   disconnected icon)
 - `/usr/local/share/zsh/site-functions/_vndroid` (zsh completion)
 - `/usr/local/share/bash-completion/completions/vndroid` (bash completion)
 - `/usr/local/share/applications/vndroid.desktop` (app-without-console desktop
   entry file)
 - `/usr/local/share/applications/vndroid-console.desktop` (app-with-console
   desktop entry file)

You can then run `vndroid`.


### Uninstall

```bash
sudo ninja -Cx uninstall  # without sudo on Windows
```
