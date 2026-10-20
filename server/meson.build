# It may be useful to use a prebuilt server, so that no Android SDK is required
# to build. If the 'prebuilt_server' option is set, just copy the file as is.
prebuilt_server = get_option('prebuilt_server')
if prebuilt_server == ''
    custom_target('vndroid-server',
                  # gradle is responsible for tracking source changes
                  build_by_default: true,
                  build_always_stale: true,
                  output: 'vndroid-server',
                  command: [find_program('./scripts/build-wrapper.sh'), meson.current_source_dir(), '@OUTPUT@', get_option('buildtype')],
                  console: true,
                  install: true,
                  install_dir: 'share/vndroid')
else
    if not prebuilt_server.startswith('/')
        # prebuilt server path is relative to the root VNdroid directory
        prebuilt_server = '../' + prebuilt_server
    endif
    custom_target('vndroid-server-prebuilt',
                  input: prebuilt_server,
                  output: 'vndroid-server',
                  command: ['cp', '@INPUT@', '@OUTPUT@'],
                  install: true,
                  install_dir: 'share/vndroid')
endif

if meson.version().version_compare('>= 0.58.0')
       devenv = environment()
       devenv.set('VNDROID_SERVER_PATH', meson.current_build_dir() / 'vndroid-server')
       meson.add_devenv(devenv)
endif
