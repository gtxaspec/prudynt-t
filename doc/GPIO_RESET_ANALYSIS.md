# GPIO Reset Analysis and Fix

## Problem Identified

The user correctly identified that `gpio_reset: 91` is not a universal default and varies per SoC. Investigation revealed:

## Code Analysis

### Where gpio_reset is Used

**File: `src/IMPSystem.cpp` (lines 16-26)**
```cpp
#if defined(PLATFORM_T40) || defined(PLATFORM_T41)
    // Additional fields required for T40/T41 platforms
    out.i2c.i2c_adapter_id = cfg->sensor.i2c_bus;
    out.rst_gpio = cfg->sensor.gpio_reset;  // ← ONLY USED HERE
    out.pwdn_gpio = -1;
    out.power_gpio = -1;
    out.sensor_id = 0;
    out.video_interface = static_cast<IMPSensorVinType>(cfg->sensor.video_interface);
    out.mclk = static_cast<IMPSensorMclk>(cfg->sensor.mclk);
    out.default_boot = 0;
#endif
```

### Key Findings

1. **Platform-Specific**: `gpio_reset` is **only used on T40/T41 platforms**
2. **Hardware-Specific**: GPIO pin assignments vary per board design and SoC
3. **Not Auto-Detectable**: No `/proc/jz/sensor/reset_gpio` file exists in the available proc files
4. **Incorrect Default**: The value `91` was arbitrary and incorrect for most systems

### Available /proc/jz/sensor/ Files

From user's system listing:
```
chip_id   height    i2c_addr  max_fps   min_fps   name      version   width
```

**Notable absence**: `reset_gpio` is **NOT** available in proc filesystem.

## Solution Implemented

### 1. Changed Default Value

**Before:**
```json
"gpio_reset": 91  // ❌ Incorrect universal default
```

**After:**
```json
"gpio_reset": -1  // ✅ Disabled by default, must be configured per board
```

### 2. Updated Validation

**Before:**
```cpp
{"sensor.gpio_reset", sensor.gpio_reset, 91, validateIntGe0, false, "/proc/jz/sensor/reset_gpio"}
```

**After:**
```cpp
{"sensor.gpio_reset", sensor.gpio_reset, -1, [](const int &v) { return v >= -1; }, false, "/proc/jz/sensor/reset_gpio"}
```

### 3. Updated SystemSensor Default

**Before:**
```cpp
SensorInfo() : /* ... */ reset_gpio(91), /* ... */
```

**After:**
```cpp
SensorInfo() : /* ... */ reset_gpio(-1), /* ... */
```

## Rationale for -1 Default

1. **Disabled State**: `-1` typically means "disabled" or "not configured" in GPIO contexts
2. **Explicit Configuration**: Forces users to explicitly set the correct GPIO pin for their board
3. **No Assumptions**: Avoids incorrect assumptions about hardware design
4. **Safe Default**: Won't accidentally drive wrong GPIO pins

## Board-Specific Configuration

Users must now configure `gpio_reset` based on their specific board:

### Example Configurations

```json
// T40/T41 Board A
{
  "sensor": {
    "gpio_reset": 57
  }
}

// T40/T41 Board B  
{
  "sensor": {
    "gpio_reset": 63
  }
}

// T31 or other platforms (not used)
{
  "sensor": {
    "gpio_reset": -1
  }
}
```

## Impact Assessment

### Positive Changes
- ✅ **Correct Defaults**: No more incorrect GPIO pin assumptions
- ✅ **Explicit Configuration**: Users must consciously set the right pin
- ✅ **Platform Awareness**: Clear that this is T40/T41 specific
- ✅ **Safe Operation**: Won't accidentally control wrong GPIO pins

### Migration Required
- ⚠️ **Existing Configs**: Users with T40/T41 systems need to set correct `gpio_reset` value
- ⚠️ **Board Documentation**: Need to document correct GPIO pins per board type

## Documentation Updates

1. **Configuration Files**: Updated default JSON configuration
2. **Code Comments**: Added platform-specific notes
3. **Migration Guide**: Explained gpio_reset requirements
4. **Validation**: Updated to allow -1 (disabled) value

## Conclusion

The `gpio_reset` parameter is now properly handled as a **board-specific hardware configuration** rather than a universal default. This prevents incorrect GPIO pin usage and forces explicit configuration for systems that actually need sensor reset functionality.

**Key Takeaway**: Hardware-specific parameters like GPIO pins should never have universal defaults - they must be configured per board design.
