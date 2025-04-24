# prudynt-t

**prudynt-t** is a video server based on the **[prudynt-v3](https://git.i386.io/wyze/prudynt-v3)** project originally for the Wyzecam v3. It extends the functionality of the original project while expanding compatibility with modern Ingenic hardware.

## Features

- **Video Compression**: Supports both H264 and H265 codecs for efficient video compression and streaming.
- **Two-Way Audio**: Enables bidirectional audio communication using AAC and PCMU codecs for supported devices.
- **Expanded Configuration**: Integrated support for **[libimp_control](https://github.com/gtxaspec/libimp_control)**.
- **Thingino Integration**: Seamlessly integrates with **[thingino](https://github.com/themactep/thingino-firmware)**, enhancing connectivity and control options.

## Building

The best and most binary-compatible way to build prudynt-t is by using `buildroot_dev.sh` within the [Thingino buildroot](https://github.com/themactep/thingino-firmware/wiki/Development) environment. Alternatively, you can use the following Docker image for a more isolated setup:
```
# Clone the repo
git clone https://github.com/your-user/prudynt-t.git
cd prudynt-t

# Update submodules
git submodule update --init

# Build for a specific target and build type
docker build \
  --build-arg TARGET=T31 \
  --build-arg BUILD_TYPE=dynamic \
  -t prudynt-builder .

docker run --rm -v "$(pwd):/src" prudynt-builder

# You will find the resulting binary at: bin/
```

## Contributing

Contributions to prudynt-t are welcome! If you have improvements, bug fixes, or new features, please feel free to submit a pull request or open an issue.
