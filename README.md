# VNdroid

VNdroid is a fork of [scrcpy v5.0.1](https://github.com/Genymobile/scrcpy), created by Romain Vimont and Genymobile. All credit for the original work goes to them.

This application mirrors Android devices (video and audio) connected via USB or [TCP/IP](doc/connection.md#tcpip-wireless) and allows control using the computer's keyboard and mouse. It does not require _root_ access or an app installed on the device. It works on _Linux_, _Windows_, and _macOS_.

## Prerequisites

The Android device requires at least API 21 (Android 5.0). [Audio forwarding](doc/audio.md) is supported for API >= 30 (Android 11+). Make sure you [enabled USB debugging](https://developer.android.com/studio/debug/dev-options#enable) on your device(s).

On some devices (especially Xiaomi), you might get the following error:

```
Injecting input events requires the caller (or the source of the instrumentation, if any) to have the INJECT_EVENTS permission.
```

In that case, you need to enable an additional option `USB debugging (Security Settings)` (this is an item different from `USB debugging`) to control it using a keyboard and mouse. Rebooting the device is necessary once this option is set.

Note that USB debugging is not required to run VNdroid in [OTG mode](doc/otg.md).

## Build and run

See [doc/build.md](doc/build.md), then the platform notes for [Linux](doc/linux.md), [Windows](doc/windows.md) and [macOS](doc/macos.md).

## Documentation

 - [Connection](doc/connection.md)
 - [Video](doc/video.md)
 - [Audio](doc/audio.md)
 - [Control](doc/control.md)
 - [Keyboard](doc/keyboard.md)
 - [Mouse](doc/mouse.md)
 - [Gamepad](doc/gamepad.md)
 - [Device](doc/device.md)
 - [Window](doc/window.md)
 - [Recording](doc/recording.md)
 - [Virtual display](doc/virtual-display.md)
 - [Tunnels](doc/tunnels.md)
 - [OTG](doc/otg.md)
 - [Camera](doc/camera.md)
 - [Video4Linux](doc/v4l2.md)
 - [Shortcuts](doc/shortcuts.md)
 - [Developers](doc/develop.md)

## License

Copyright (C) 2018 Genymobile. Copyright (C) 2018-2026 Romain Vimont. Licensed under the Apache License, Version 2.0, see [LICENSE](LICENSE).
