# Dynamic Sensor Detection Implementation

## Overview

This document describes the implementation of dynamic sensor detection in prudynt-t, which allows the streaming system to automatically configure sensors by reading live system data from `/proc/jz/sensor/` filesystem instead of relying on static JSON configuration files.

## Changes Made

### 1. SystemSensor.cpp and SystemSensor.hpp Updates

**Direct Implementation:**
- Updated `SystemSensor::getSensorInfo()` to read directly from `/proc/jz/sensor/` files only
- Implemented `readProcString()` and `readProcInt()` methods for file system access
- Updated `isAvailable()` to check for `/proc/jz/sensor/` directory existence
- Removed redundant fallback to `sensor all` command (which reads the same proc files)
- Clear error handling: throws runtime_error if `/proc/jz/sensor/` directory is not accessible
- Individual file fallback: uses default values when specific proc files don't exist

**Files Modified:**
- `src/SystemSensor.hpp` - Added method declarations and updated documentation
- `src/SystemSensor.cpp` - Complete implementation rewrite
- `res/prudynt.json` - Removed dynamically detected sensor parameters

### 2. Config.cpp and Config.hpp Updates

**Added Missing Sensor Parameters:**
- `sensor.chip_id` → `/proc/jz/sensor/chip_id`
- `sensor.version` → `/proc/jz/sensor/version`
- `sensor.min_fps` → `/proc/jz/sensor/min_fps`

**Modified Configuration Priority Logic:**
- Added `isSensorProcParameter()` helper function to identify sensor parameters with proc paths
- Modified `handleConfigItem()` to prioritize proc files for sensor parameters
- Sensor parameters now read from proc files first, JSON only as fallback
- Non-sensor parameters maintain original JSON-first behavior

**Cleaned Up Default Configuration:**
- Removed dynamically detected parameters from `res/prudynt.json`
- Kept only hardware configuration parameters that don't have proc file equivalents
- Added explanatory comment about dynamic parameter detection

**Complete Sensor Proc Mappings (with enforced priority):**
- `sensor.model` → `/proc/jz/sensor/name`
- `sensor.chip_id` → `/proc/jz/sensor/chip_id`
- `sensor.version` → `/proc/jz/sensor/version`
- `sensor.i2c_address` → `/proc/jz/sensor/i2c_addr`
- `sensor.width` → `/proc/jz/sensor/width`
- `sensor.height` → `/proc/jz/sensor/height`
- `sensor.fps` → `/proc/jz/sensor/max_fps`
- `sensor.min_fps` → `/proc/jz/sensor/min_fps`
- `sensor.i2c_bus` → `/proc/jz/sensor/i2c_bus`
- `sensor.boot` → `/proc/jz/sensor/boot`
- `sensor.mclk` → `/proc/jz/sensor/mclk`
- `sensor.video_interface` → `/proc/jz/sensor/video_interface`
- `sensor.gpio_reset` → `/proc/jz/sensor/reset_gpio`

**Parameters Removed from Default JSON Configuration:**
The following sensor parameters have been removed from `res/prudynt.json` because they are now dynamically detected:
- `model` (detected from `/proc/jz/sensor/name`)
- `width` (detected from `/proc/jz/sensor/width`)
- `height` (detected from `/proc/jz/sensor/height`)
- `fps` (detected from `/proc/jz/sensor/max_fps`)
- `i2c_address` (detected from `/proc/jz/sensor/i2c_addr`)

**Parameters Kept in JSON Configuration:**
These parameters remain in the JSON config because they don't have corresponding proc files:
- `boot`, `i2c_bus`, `mclk`, `video_interface` - Hardware configuration parameters
- `gpio_reset` - **SoC-specific GPIO pin** (T40/T41 only), must be configured per board (default: -1 = disabled)

**Important Notes:**
- All parameters with proc file mappings now prioritize proc files over JSON configuration
- `/proc/jz/sensor/` is the single source of truth for sensor hardware detection
- `gpio_reset` is **SoC-specific** and varies per board design (used only on T40/T41 platforms)
- Default `gpio_reset: -1` means disabled; must be configured per board if sensor reset is needed

## How It Works

### Configuration Priority Order

**For Sensor Parameters with `/proc/jz/sensor/` mappings:**

1. **Proc Filesystem** (highest priority - single source of truth)
   - Always read from `/proc/jz/sensor/` files first when available
   - Provides authoritative live sensor data directly from the driver

2. **JSON Configuration** (fallback only)
   - Only used if the corresponding proc file doesn't exist or can't be read
   - Allows manual override when proc filesystem is unavailable

3. **Default Values** (last resort)
   - Used if both proc file and JSON value are unavailable

**For Non-Sensor Parameters:**

1. **JSON Configuration** (highest priority)
   - Manual configuration takes precedence

2. **Proc Filesystem** (fallback)
   - Used if JSON value is not set and proc path exists

3. **Default Values** (last resort)
   - Used if both sources are unavailable

### SystemSensor Implementation

```cpp
// Direct approach: Read from /proc/jz/sensor/ or fail clearly
if (!isAvailable()) {
    throw std::runtime_error("Sensor proc filesystem /proc/jz/sensor/ is not accessible");
}

// Read sensor parameters directly from proc files
info.name = readProcString("name");
info.width = readProcInt("width", info.width);  // Uses default if file missing
// ... other parameters
```

### Config System Integration

The modified `handleConfigItem()` function in Config.cpp now enforces proc priority for sensor parameters:

```cpp
// For sensor parameters with proc paths, prioritize proc files over JSON
if (isSensorProcParameter(item)) {
    // Try proc file first for sensor parameters
    std::ifstream procFile(item.procPath);
    // Read and parse proc file content
}

// Only read from JSON if proc file failed or this is not a sensor proc parameter
if (!readFromProc) {
    // Parse and read JSON configuration
}
```

## Benefits

### 1. Single Source of Truth
- `/proc/jz/sensor/` files are now the authoritative source for sensor hardware data
- No possibility of JSON configuration overriding actual hardware capabilities
- Automatic detection of sensor capabilities and settings directly from driver

### 2. Clear Error Handling
- Existing JSON configurations continue to work
- Clear failure when `/proc/jz/sensor/` directory is not accessible
- Individual file fallback to defaults when specific proc files are missing

### 3. Eliminated Configuration Conflicts
- Impossible for JSON config to contradict actual sensor hardware
- Automatic adaptation to different sensor models without manual configuration
- Real-time sensor parameter updates directly from hardware

## Usage

### For Thingino Systems

On thingino systems with `/proc/jz/sensor/` available:

1. **Automatic Hardware Detection**: Sensor parameters are automatically read from `/proc/jz/sensor/`
2. **No JSON Override**: JSON sensor values are ignored when proc files exist (enforcing single source of truth)
3. **Minimal Configuration**: Only non-sensor parameters need to be specified in JSON

### For Development Systems

On development systems without `/proc/jz/sensor/`:

1. **Clear Failure**: SystemSensor::getSensorInfo() throws runtime_error
2. **Config System Fallback**: Config system still reads from JSON or uses defaults
3. **JSON Configuration**: Specify sensor parameters in `prudynt.json` for testing

## Testing

The implementation has been tested for:

- ✅ Compilation without errors
- ✅ Proper fallback mechanism when `/proc/jz/sensor/` unavailable
- ✅ Integration with existing Config system
- ✅ Backward compatibility with JSON configuration

## Future Enhancements

1. **Hot-Reload**: Detect sensor changes at runtime
2. **Multi-Sensor**: Support for multiple sensors
3. **Validation**: Enhanced parameter validation from proc data
4. **Caching**: Cache proc file reads for performance

## Files Changed

- `src/SystemSensor.hpp` - Interface and documentation updates
- `src/SystemSensor.cpp` - Complete implementation rewrite  
- `src/Config.hpp` - Added missing sensor parameters to `_sensor` struct
- `src/Config.cpp` - Added proc path mappings for new parameters
- `DYNAMIC_SENSOR_DETECTION.md` - This documentation file

## Conclusion

The dynamic sensor detection implementation provides a robust, backward-compatible solution for automatic sensor configuration. It eliminates the need for manual JSON configuration while maintaining full compatibility with existing setups and providing graceful fallbacks for all scenarios.
