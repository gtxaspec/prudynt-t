# RTSP Runtime Status Interface

## Overview

The RTSP Runtime Status Interface provides real-time access to active RTSP stream parameters through a file-based interface similar to the `/proc` filesystem pattern. This enables shell scripts and external tools to query current streaming configuration without parsing JSON files or interacting with the application directly.

## Interface Location

The interface creates files under `/run/prudynt/rtsp/` with the following structure:

```
/run/prudynt/rtsp/
├── stream0/
│   ├── format      # Video codec (e.g., "H264", "H265")
│   ├── fps         # Frame rate (e.g., "25")
│   ├── width       # Video width (e.g., "1920")
│   ├── height      # Video height (e.g., "1080")
│   ├── endpoint    # RTSP endpoint (e.g., "ch0")
│   ├── url         # Full RTSP URL (e.g., "rtsp://192.168.1.100:554/ch0")
│   ├── bitrate     # Bitrate in kbps (e.g., "3000")
│   ├── mode        # Bitrate mode (e.g., "CBR", "VBR")
│   └── enabled     # Stream enabled status (e.g., "true", "false")
└── stream1/
    ├── format
    ├── fps
    ├── width
    ├── height
    ├── endpoint
    ├── url
    ├── bitrate
    ├── mode
    └── enabled
```

## Available Parameters

For each active RTSP stream, the following parameters are exposed:

| Parameter | Description | Example Values |
|-----------|-------------|----------------|
| `format` | Video codec | `H264`, `H265` |
| `fps` | Frame rate | `25`, `30` |
| `width` | Video width in pixels | `1920`, `640` |
| `height` | Video height in pixels | `1080`, `360` |
| `endpoint` | RTSP endpoint path | `ch0`, `ch1` |
| `url` | Complete RTSP URL | `rtsp://192.168.1.100:554/ch0` |
| `bitrate` | Bitrate in kbps | `3000`, `1000` |
| `mode` | Bitrate control mode | `CBR`, `VBR`, `SMART` |
| `enabled` | Stream enabled status | `true`, `false` |

## Usage Examples

### Shell Script Examples

#### Basic Stream Information
```bash
#!/bin/bash

RTSP_DIR="/run/prudynt/rtsp"

echo "=== Active RTSP Streams ==="
for stream_dir in "$RTSP_DIR"/stream*; do
    if [ -d "$stream_dir" ]; then
        stream_name=$(basename "$stream_dir")
        echo
        echo "Stream: $stream_name"
        echo "  Format:     $(cat "$stream_dir/format" 2>/dev/null || echo "N/A")"
        echo "  Resolution: $(cat "$stream_dir/width" 2>/dev/null || echo "N/A")x$(cat "$stream_dir/height" 2>/dev/null || echo "N/A")"
        echo "  FPS:        $(cat "$stream_dir/fps" 2>/dev/null || echo "N/A")"
        echo "  Bitrate:    $(cat "$stream_dir/bitrate" 2>/dev/null || echo "N/A") kbps"
        echo "  Mode:       $(cat "$stream_dir/mode" 2>/dev/null || echo "N/A")"
        echo "  URL:        $(cat "$stream_dir/url" 2>/dev/null || echo "N/A")"
        echo "  Enabled:    $(cat "$stream_dir/enabled" 2>/dev/null || echo "N/A")"
    fi
done
```

#### Stream Monitoring
```bash
#!/bin/bash

# Monitor specific stream parameters
STREAM="stream0"
RTSP_DIR="/run/prudynt/rtsp/$STREAM"

if [ -d "$RTSP_DIR" ]; then
    format=$(cat "$RTSP_DIR/format" 2>/dev/null)
    fps=$(cat "$RTSP_DIR/fps" 2>/dev/null)
    bitrate=$(cat "$RTSP_DIR/bitrate" 2>/dev/null)

    echo "Stream $STREAM: $format @ ${fps}fps, ${bitrate}kbps"
else
    echo "Stream $STREAM is not active"
fi
```

#### URL Extraction for External Tools
```bash
#!/bin/bash

# Extract RTSP URLs for use with external tools like ffmpeg
RTSP_DIR="/run/prudynt/rtsp"

for stream_dir in "$RTSP_DIR"/stream*; do
    if [ -d "$stream_dir" ]; then
        stream_name=$(basename "$stream_dir")
        url=$(cat "$stream_dir/url" 2>/dev/null)
        enabled=$(cat "$stream_dir/enabled" 2>/dev/null)

        if [ "$enabled" = "true" ] && [ -n "$url" ]; then
            echo "Active stream $stream_name: $url"

            # Example: Use with ffmpeg
            # ffmpeg -i "$url" -t 10 -c copy "recording_${stream_name}.mp4"
        fi
    fi
done
```

#### System Monitoring Integration
```bash
#!/bin/bash

# Integration with system monitoring tools
RTSP_DIR="/run/prudynt/rtsp"

# Check if any streams are active
if [ -d "$RTSP_DIR" ] && [ "$(ls -A "$RTSP_DIR" 2>/dev/null)" ]; then
    echo "RTSP_STREAMS_ACTIVE=1"

    # Count active streams
    active_count=0
    for stream_dir in "$RTSP_DIR"/stream*; do
        if [ -d "$stream_dir" ]; then
            enabled=$(cat "$stream_dir/enabled" 2>/dev/null)
            if [ "$enabled" = "true" ]; then
                ((active_count++))
            fi
        fi
    done
    echo "RTSP_ACTIVE_STREAM_COUNT=$active_count"
else
    echo "RTSP_STREAMS_ACTIVE=0"
    echo "RTSP_ACTIVE_STREAM_COUNT=0"
fi
```

### One-liner Commands

```bash
# Get stream0 resolution
echo "$(cat /run/prudynt/rtsp/stream0/width 2>/dev/null)x$(cat /run/prudynt/rtsp/stream0/height 2>/dev/null)"

# Get all active RTSP URLs
find /run/prudynt/rtsp -name "url" -exec cat {} \;

# Check if any streams are H265
find /run/prudynt/rtsp -name "format" -exec grep -l "H265" {} \;

# Get total bitrate of all active streams
find /run/prudynt/rtsp -name "bitrate" -exec cat {} \; | awk '{sum+=$1} END {print sum " kbps"}'
```

## Behavior

### Stream Lifecycle
- **Stream Start**: When an RTSP stream is started, its directory and parameter files are created automatically
- **Stream Stop**: When an RTSP stream is stopped, its directory and all parameter files are removed
- **RTSP Restart**: When the RTSP server restarts, all existing status files are cleaned up and recreated for active streams

### File Format
- Each parameter file contains a single line with the parameter value
- No trailing whitespace or special characters (except newline)
- Numeric values are stored as plain text numbers
- Boolean values are stored as "true" or "false"
- String values are stored as-is

### Error Handling
- If prudynt is not running, the `/run/prudynt/rtsp/` directory will not exist
- If a stream is disabled, its directory will not be present
- Always check for file existence before reading to handle graceful degradation

## Integration Notes

### Single Source of Truth
The RTSP Runtime Status Interface provides the authoritative source for active stream parameters, following the same pattern as the sensor detection system that uses `/proc/jz/sensor/` as the single source of truth.

### Performance
- File operations are lightweight and suitable for frequent polling
- No locks or complex synchronization required for reading
- Files are updated atomically when stream parameters change

### Compatibility
- Works with any shell or scripting language that can read files
- No special libraries or dependencies required
- Compatible with standard Unix tools (cat, grep, awk, etc.)

## Testing

A test script is provided to verify the interface functionality:

```bash
# Run the test script
./test_rtsp_status.sh
```

This will verify:
- Directory structure creation
- Parameter file format validation
- Shell script compatibility
- Error handling

## Troubleshooting

### Interface Not Available
If `/run/prudynt/rtsp/` doesn't exist:
1. Check if prudynt is running
2. Verify RTSP streams are enabled in configuration
3. Check prudynt logs for RTSPStatus initialization errors

### Missing Stream Directories
If expected stream directories are missing:
1. Check if streams are enabled in `/etc/prudynt.json`
2. Verify stream configuration is valid
3. Check if RTSP server started successfully

### Empty or Invalid Parameter Files
If parameter files contain unexpected values:
1. Check prudynt logs for RTSPStatus update errors
2. Verify stream configuration matches expected values
3. Restart prudynt to refresh the interface
