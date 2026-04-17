# Sensor Configuration Cleanup

## Before: Static Configuration

Previously, the sensor section in `prudynt.json` contained many parameters that duplicated hardware detection:

```json
{
  "sensor": {
    "boot": 0,
    "fps": 25,                    // ❌ Now detected from /proc/jz/sensor/max_fps
    "gpio_reset": 91,
    "height": 1080,               // ❌ Now detected from /proc/jz/sensor/height
    "i2c_address": 55,            // ❌ Now detected from /proc/jz/sensor/i2c_addr
    "i2c_bus": 0,
    "mclk": 1,
    "model": "gc2053",            // ❌ Now detected from /proc/jz/sensor/name
    "video_interface": 0,
    "width": 1920                 // ❌ Now detected from /proc/jz/sensor/width
  }
}
```

**Problems with this approach:**
- ❌ JSON values could contradict actual hardware
- ❌ Manual configuration required for each sensor type
- ❌ Risk of mismatched parameters causing issues
- ❌ Duplication between hardware detection and configuration

## After: Dynamic Detection

Now the sensor section only contains hardware configuration parameters that cannot be auto-detected:

```json
{
  "sensor": {
    "_comment": "Dynamic parameters (model, width, height, fps, i2c_address, chip_id, version, min_fps) are automatically detected from /proc/jz/sensor/ and should not be configured here",
    "boot": 0,
    "gpio_reset": -1,
    "i2c_bus": 0,
    "mclk": 1,
    "video_interface": 0
  }
}
```

**Benefits of this approach:**
- ✅ Hardware detection parameters automatically read from `/proc/jz/sensor/`
- ✅ Impossible for JSON to contradict actual hardware capabilities
- ✅ Minimal configuration required - only hardware setup parameters
- ✅ Single source of truth for sensor hardware detection

## Dynamically Detected Parameters

These parameters are now automatically detected and **removed from JSON configuration**:

| Parameter | Source | Description |
|-----------|--------|-------------|
| `model` | `/proc/jz/sensor/name` | Sensor model name (e.g., "gc2083") |
| `width` | `/proc/jz/sensor/width` | Native sensor width (e.g., 1920) |
| `height` | `/proc/jz/sensor/height` | Native sensor height (e.g., 1080) |
| `fps` | `/proc/jz/sensor/max_fps` | Maximum FPS capability |
| `i2c_address` | `/proc/jz/sensor/i2c_addr` | I2C address (e.g., 0x37) |
| `chip_id` | `/proc/jz/sensor/chip_id` | Chip ID (e.g., "0x2083") |
| `version` | `/proc/jz/sensor/version` | Sensor version string |
| `min_fps` | `/proc/jz/sensor/min_fps` | Minimum FPS capability |

## Hardware Configuration Parameters

These parameters remain in JSON configuration because they control hardware setup rather than detection:

| Parameter | Description | Why in JSON |
|-----------|-------------|-------------|
| `boot` | Boot parameter | Hardware configuration, not detection |
| `gpio_reset` | Reset GPIO pin (T40/T41 only) | **SoC-specific pin assignment** (varies per board) |
| `i2c_bus` | I2C bus number | Hardware bus configuration |
| `mclk` | MCLK setting | Clock configuration |
| `video_interface` | Video interface type | Interface configuration |

## Migration Guide

If you have an existing `prudynt.json` with sensor parameters:

1. **Remove dynamic parameters** from your sensor configuration:
   - Remove: `model`, `width`, `height`, `fps`, `i2c_address`
   - Keep: `boot`, `gpio_reset`, `i2c_bus`, `mclk`, `video_interface`

2. **Verify hardware detection** works:
   - Check that `/proc/jz/sensor/` directory exists
   - Verify sensor files are readable: `ls -la /proc/jz/sensor/`

3. **Test configuration**:
   - Restart prudynt service
   - Verify sensor detection in logs
   - Check that stream parameters match actual hardware

## Result

The configuration is now cleaner, more reliable, and automatically adapts to different sensor hardware without manual intervention.
