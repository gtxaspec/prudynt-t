# Prudynt JSON Configuration Documentation

This document describes the JSON configuration format for Prudynt-T video streamer. The configuration file is located at `/etc/prudynt.json` and uses standard JSON format.

## Configuration Management

### Using jct (JSON Config Tool)

The thingino firmware includes `jct` for easy JSON configuration management:

```bash
# Get a configuration value
jct /etc/prudynt.json get motion.enabled

# Set a configuration value  
jct /etc/prudynt.json set motion.enabled true

# Print entire configuration
jct /etc/prudynt.json print

# Create new config file
jct /etc/prudynt.json create
```

### Manual Editing

You can also edit the JSON file directly with any text editor:

```bash
vi /etc/prudynt.json
nano /etc/prudynt.json
```

Validate JSON syntax with:
```bash
jq . /etc/prudynt.json
```

## Configuration Structure

The configuration is organized into the following main sections:

- **version**: Configuration file version
- **general**: General application settings
- **rtsp**: RTSP streaming settings
- **sensor**: Camera sensor configuration
- **image**: Image processing settings
- **stream0**: Primary video stream settings
- **stream1**: Secondary video stream settings  
- **stream2**: JPEG snapshot settings
- **websocket**: WebSocket server settings
- **audio**: Audio input/output settings
- **motion**: Motion detection settings

## Configuration Reference

### Version

```json
{
  "version": "1.0"
}
```

**version** (string): Configuration file version identifier.

### General Settings

```json
{
  "general": {
    "loglevel": "INFO",
    "osd_pool_size": 1025,
    "imp_polling_timeout": 500
  }
}
```

**loglevel** (string): Logging level. Options: `EMERGENCY`, `ALERT`, `CRITICAL`, `ERROR`, `WARN`, `NOTICE`, `INFO`, `DEBUG`.

**osd_pool_size** (integer): OSD pool size (0-1024). Controls memory allocation for on-screen display elements.

**imp_polling_timeout** (integer): IMP polling timeout in milliseconds (1-5000). Controls hardware polling frequency.

### RTSP Settings

```json
{
  "rtsp": {
    "port": 554,
    "name": "thingino prudynt",
    "est_bitrate": 5000,
    "out_buffer_size": 500000,
    "send_buffer_size": 307200,
    "session_reclaim": 65,
    "auth_required": true,
    "username": "thingino",
    "password": "thingino"
  }
}
```

**port** (integer): Port number for RTSP service (default: 554).

**name** (string): Descriptive name for the RTSP service.

**est_bitrate** (integer): Estimated bitrate for RTSP streaming in kbps.

**out_buffer_size** (integer): Output buffer size for RTSP streaming in bytes.

**send_buffer_size** (integer): Send buffer size for RTSP streaming in bytes.

**session_reclaim** (integer): Client session timeout in seconds. Sessions are reclaimed after this period of inactivity.

**auth_required** (boolean): Enable RTSP authentication.

**username** (string): Username for RTSP authentication.

**password** (string): Password for RTSP authentication.

### Sensor Settings

```json
{
  "sensor": {
    "model": "gc2053",
    "i2c_address": 55,
    "i2c_bus": 0,
    "fps": 25,
    "width": 1920,
    "height": 1080,
    "boot": 0,
    "mclk": 1,
    "video_interface": 0,
    "gpio_reset": 91
  }
}
```

**model** (string): Sensor model identifier.

**i2c_address** (integer): I2C address of the sensor (0x0 - 0x7F). Use decimal format in JSON.

**i2c_bus** (integer): I2C bus ID of the sensor (T40/T41 only).

**fps** (integer): Frames per second captured by the sensor (0-60).

**width** (integer): Width of the sensor's image in pixels.

**height** (integer): Height of the sensor's image in pixels.

**boot** (integer): Boot mode of the sensor (T40/T41 only).

**mclk** (integer): Clock interface ID of the sensor (T40/T41 only).

**video_interface** (integer): Video interface ID (CSI) of the sensor (T40/T41 only).

**gpio_reset** (integer): Reset GPIO pin number of the sensor (T40/T41 only).

### Image Settings

```json
{
  "image": {
    "vflip": false,
    "hflip": false,
    "ae_compensation": 128,
    "anti_flicker": 2,
    "backlight_compensation": 0,
    "brightness": 128,
    "contrast": 128,
    "core_wb_mode": 0,
    "defog_strength": 128,
    "dpc_strength": 128,
    "drc_strength": 128,
    "highlight_depress": 0,
    "hue": 128,
    "max_again": 160,
    "max_dgain": 80,
    "running_mode": 0,
    "saturation": 128,
    "sharpness": 128,
    "sinter_strength": 128,
    "temper_strength": 128,
    "wb_bgain": 0,
    "wb_rgain": 0
  }
}
```

**vflip** (boolean): Enable vertical flip of the image.

**hflip** (boolean): Enable horizontal flip of the image.

**ae_compensation** (integer): Auto-exposure compensation level (0-255).

**anti_flicker** (integer): Anti-flicker mode setting.

**backlight_compensation** (integer): Backlight compensation level.

**brightness** (integer): Image brightness level (0-255).

**contrast** (integer): Image contrast level (0-255).

**core_wb_mode** (integer): Core white balance mode.

**defog_strength** (integer): Defog processing strength (0-255).

**dpc_strength** (integer): Dead pixel correction strength (0-255).

**drc_strength** (integer): Dynamic range compression strength (0-255).

**highlight_depress** (integer): Highlight depression level.

**hue** (integer): Image hue adjustment (0-255).

**max_again** (integer): Maximum analog gain value.

**max_dgain** (integer): Maximum digital gain value.

**running_mode** (integer): Image processing running mode.

**saturation** (integer): Image saturation level (0-255).

**sharpness** (integer): Image sharpness level (0-255).

**sinter_strength** (integer): Spatial noise reduction strength (0-255).

**temper_strength** (integer): Temporal noise reduction strength (0-255).

**wb_bgain** (integer): White balance blue gain adjustment.

**wb_rgain** (integer): White balance red gain adjustment.

## Stream Configuration

Both stream0 and stream1 follow the same configuration structure. Stream0 is typically the main high-resolution stream, while stream1 is the secondary lower-resolution stream.

### Stream Settings

```json
{
  "stream0": {
    "enabled": true,
    "audio_enabled": true,
    "rtsp_endpoint": "ch0",
    "rtsp_info": "stream0",
    "format": "H264",
    "bitrate": 3000,
    "mode": "CBR",
    "width": 1920,
    "height": 1080,
    "buffers": 2,
    "fps": 25,
    "gop": 20,
    "max_gop": 60,
    "profile": 2,
    "rotation": 0
  }
}
```

**enabled** (boolean): Enable or disable the stream.

**audio_enabled** (boolean): Enable audio on the stream.

**rtsp_endpoint** (string): Endpoint name for the RTSP URL stream.

**rtsp_info** (string): Endpoint info for the RTSP URL stream.

**format** (string): Video format for the stream. Options: `H264`, `H265`.

**bitrate** (integer): Bitrate for the stream in kbps.

**mode** (string): Rate control mode. Options:
- `CBR`: Constant Bit Rate
- `VBR`: Variable Bit Rate  
- `FIXQP`: Fixed Quantization Parameter
- `SMART`: Smart mode (T1x, T2x, T30 only)
- `CAPPED_VBR`: Capped Variable Bit Rate (T31 only)
- `CAPPED_QUALITY`: Capped Quality (T31 only)

**width** (integer): Width of the video stream in pixels.

**height** (integer): Height of the video stream in pixels.

**buffers** (integer): Number of buffers for the stream.

**fps** (integer): Frames per second for the stream (0-60).

**gop** (integer): Group of Pictures size for the stream.

**max_gop** (integer): Maximum GOP size for the stream.

**profile** (integer): H.264/H.265 profile. Options:
- `0`: Baseline profile
- `1`: Main profile  
- `2`: High profile

**rotation** (integer): Video stream rotation. Options:
- `0`: No rotation
- `1`: 90 degrees
- `2`: 270 degrees

### Advanced Quality Control Parameters

For fine-tuning stream quality, the following advanced parameters are available:

```json
{
  "stream0": {
    "min_qp": 20,
    "max_qp": 45,
    "initial_qp": -1,
    "max_bitrate": 6000,
    "i_frame_interval": 0,
    "quality_level": 2,
    "adaptive_gop": true,
    "scene_change_detection": false
  }
}
```

**min_qp** (integer): Minimum quantization parameter (0-51). Lower values = higher quality. Use -1 for auto.

**max_qp** (integer): Maximum quantization parameter (0-51). Higher values = lower quality. Use -1 for auto.

**initial_qp** (integer): Initial quantization parameter (0-51). Use -1 for auto.

**max_bitrate** (integer): Maximum bitrate for VBR modes in kbps. Use -1 for auto (1.5x target bitrate).

**i_frame_interval** (integer): I-frame interval in seconds (0-300). 0 = use GOP setting.

**quality_level** (integer): Quality level for SMART mode (0-6). Higher = better quality but larger files.

**adaptive_gop** (boolean): Enable adaptive GOP sizing based on frame rate (GOP = FPS × 2). Default: true.

**scene_change_detection** (boolean): Enable scene change detection for better quality transitions.

### OSD (On-Screen Display) Settings

Each stream can have its own OSD configuration:

```json
{
  "stream0": {
    "osd": {
      "enabled": true,
      "start_delay": 0,
      "font_path": "/usr/share/fonts/NotoSansDisplay-Condensed2.ttf",
      "font_size": 64,
      "font_color": 4294967295,
      "font_stroke_enabled": true,
      "font_stroke_size": 64,
      "font_stroke_color": 4278190080,
      "time_enabled": true,
      "time_format": "%F %T",
      "user_text_enabled": true,
      "user_text_format": "%hostname",
      "uptime_enabled": true,
      "uptime_format": "Uptime: %02lu:%02lu:%02lu",
      "logo_enabled": true,
      "logo_path": "/usr/share/images/thingino_logo_1.bgra",
      "logo_width": 100,
      "logo_height": 30,
      "logo_transparency": 255,
      "pos_time_x": 10,
      "pos_time_y": 10,
      "pos_user_text_x": 900,
      "pos_user_text_y": 5,
      "pos_uptime_x": 1600,
      "pos_uptime_y": 5,
      "pos_logo_x": 1800,
      "pos_logo_y": 1030,
      "font_stroke": 1,
      "font_xscale": 100,
      "font_yscale": 100,
      "font_yoffset": 3,
      "logo_rotation": 0,
      "time_rotation": 0,
      "time_transparency": 255,
      "uptime_rotation": 0,
      "uptime_transparency": 255,
      "user_text_rotation": 0,
      "user_text_transparency": 255
    }
  }
}
```

**enabled** (boolean): Enable or disable the OSD.

**start_delay** (integer): Delayed start of the OSD display in seconds (0-5000).

**font_path** (string): Path to the font file for OSD text.

**font_size** (integer): Font size for OSD text.

**font_color** (integer): Font color in ARGB format (use decimal values).

**font_stroke_enabled** (boolean): Enable stroke (outline) for OSD text.

**font_stroke_size** (integer): Size of the font stroke.

**font_stroke_color** (integer): Color of the font stroke in ARGB format.

**time_enabled** (boolean): Enable time display in the OSD.

**time_format** (string): Format string for displaying time (strftime format).

**user_text_enabled** (boolean): Enable display of custom user text.

**user_text_format** (string): Custom text to display. Supports variables like `%hostname`.

**uptime_enabled** (boolean): Enable display of system uptime.

**uptime_format** (string): Format string for displaying uptime.

**logo_enabled** (boolean): Enable display of a logo image.

**logo_path** (string): Path to the logo image file.

**logo_width** (integer): Width of the logo image.

**logo_height** (integer): Height of the logo image.

**logo_transparency** (integer): Transparency for the logo (0-255).

**pos_time_x/y** (integer): X/Y position for the time display.

**pos_user_text_x/y** (integer): X/Y position for the user text.

**pos_uptime_x/y** (integer): X/Y position for the uptime display.

**pos_logo_x/y** (integer): X/Y position for the logo.

**font_stroke** (integer): Stroke width for the font.

**font_xscale/yscale** (integer): X/Y scale for the font (percentage).

**font_yoffset** (integer): Y offset for the font.

**logo_rotation** (integer): Rotation for the logo (0-360 degrees).

**time_rotation** (integer): Rotation for the time display (0-360 degrees).

**time_transparency** (integer): Transparency for the time display (0-255).

**uptime_rotation** (integer): Rotation for the uptime display (0-360 degrees).

**uptime_transparency** (integer): Transparency for the uptime display (0-255).

**user_text_rotation** (integer): Rotation for the user text (0-360 degrees).

**user_text_transparency** (integer): Transparency for the user text (0-255).

### Stream2 Settings (JPEG Snapshots)

```json
{
  "stream2": {
    "enabled": true,
    "jpeg_path": "/tmp/snapshot.jpg",
    "jpeg_quality": 75,
    "jpeg_refresh": 1000,
    "jpeg_channel": 0,
    "jpeg_idle_fps": 1
  }
}
```

**enabled** (boolean): Enable or disable JPEG snapshot stream.

**jpeg_path** (string): File path for JPEG snapshots.

**jpeg_quality** (integer): Quality of JPEG snapshots (1-100).

**jpeg_refresh** (integer): Refresh rate for JPEG snapshots in milliseconds.

**jpeg_channel** (integer): JPEG channel source (0 or 1).

**jpeg_idle_fps** (integer): FPS when no requests are made via WebSocket/HTTP. 0 = sleep on idle.

### WebSocket Settings

```json
{
  "websocket": {
    "enabled": true,
    "secured": false,
    "loglevel": 4096,
    "port": 8089
  }
}
```

**enabled** (boolean): Enable or disable WebSocket server.

**secured** (boolean): Enable or disable secured WebSocket (WSS).

**loglevel** (integer): Log level for WebSocket operations.

**port** (integer): Port number for WebSocket service.

### Audio Settings

```json
{
  "audio": {
    "input_enabled": true,
    "input_format": "OPUS",
    "input_bitrate": 40,
    "input_sample_rate": 16000,
    "input_high_pass_filter": false,
    "input_agc_enabled": false,
    "input_vol": 80,
    "input_gain": 25,
    "input_alc_gain": 0,
    "input_agc_target_level_dbfs": 10,
    "input_agc_compression_gain_db": 0,
    "input_noise_suppression": 0,
    "force_stereo": false,
    "output_enabled": true,
    "output_sample_rate": 16000
  }
}
```

**input_enabled** (boolean): Enable or disable audio input.

**input_format** (string): Audio format. Options: `OPUS`, `AAC`, `PCM`, `G711A`, `G711U`, `G726`.

**input_bitrate** (integer): Audio encoder bitrate in kbps (6-256).

**input_sample_rate** (integer): Input audio sampling rate in Hz. Options: 8000, 16000, 24000, 44100, 48000.

**input_high_pass_filter** (boolean): Enable high pass filter for audio input.

**input_agc_enabled** (boolean): Enable Automatic Gain Control for audio input.

**input_vol** (integer): Input volume for audio (-30 to 120).

**input_gain** (integer): Input gain for audio (-1 to 31). -1 disables this setting.

**input_alc_gain** (integer): ALC gain for audio input (-1 to 7). -1 disables this setting.

**input_agc_target_level_dbfs** (integer): AGC target level in dBFS (0-31).

**input_agc_compression_gain_db** (integer): AGC compression gain in dB (0-90).

**input_noise_suppression** (integer): Noise suppression level (0-3).

**force_stereo** (boolean): Enable stereo audio. Best supported with PCM and OPUS.

**output_enabled** (boolean): Enable two-way audio output (backchannel audio).

**output_sample_rate** (integer): Output audio sampling rate in Hz. Must match input device.

### Motion Detection Settings

```json
{
  "motion": {
    "enabled": false,
    "ivs_polling_timeout": 1000,
    "monitor_stream": 1,
    "script_path": "/usr/sbin/motion",
    "debounce_time": 0,
    "post_time": 0,
    "cooldown_time": 5,
    "init_time": 5,
    "min_time": 1,
    "sensitivity": 1,
    "skip_frame_count": 5,
    "frame_width": 1920,
    "frame_height": 1080,
    "roi_0_x": 0,
    "roi_0_y": 0,
    "roi_1_x": 1920,
    "roi_1_y": 1080,
    "roi_count": 1
  }
}
```

**enabled** (boolean): Enable or disable motion detection.

**ivs_polling_timeout** (integer): Query timeout for motion detection frames in milliseconds.

**monitor_stream** (integer): Stream to monitor for motion (0 or 1).

**script_path** (string): Path to script executed when motion is detected.

**debounce_time** (integer): Time to wait before triggering motion detection again.

**post_time** (integer): Time after motion stops to continue recording.

**cooldown_time** (integer): Time to wait after motion event before detecting new motion.

**init_time** (integer): Time for motion detection to initialize at startup.

**min_time** (integer): Minimum time to track motion detection.

**sensitivity** (integer): Sensitivity level of motion detection.

**skip_frame_count** (integer): Number of frames to skip (reduces CPU load).

**frame_width** (integer): Width of frame used for motion detection.

**frame_height** (integer): Height of frame used for motion detection.

**roi_0_x/y** (integer): Top-left corner coordinates of first Region of Interest.

**roi_1_x/y** (integer): Bottom-right corner coordinates of first Region of Interest.

**roi_count** (integer): Number of active Regions of Interest.

## SOC Compatibility

Some options are only supported on specific SOC versions:

- **T1x, T2x, T30**: Support `SMART` mode
- **T31**: Support `CAPPED_VBR` and `CAPPED_QUALITY` modes, but not `SMART`
- **T40/T41**: Additional sensor configuration options

For detailed compatibility information, see: https://github.com/gtxaspec/prudynt-t/wiki/Configuration

## Examples

### Basic Configuration

```json
{
  "version": "1.0",
  "general": {
    "loglevel": "INFO"
  },
  "sensor": {
    "fps": 25
  },
  "stream0": {
    "enabled": true,
    "format": "H264",
    "bitrate": 3000,
    "mode": "CBR",
    "fps": 25
  },
  "motion": {
    "enabled": false
  }
}
```

### High-Quality Streaming

```json
{
  "version": "1.0",
  "stream0": {
    "enabled": true,
    "format": "H264",
    "bitrate": 8000,
    "mode": "VBR",
    "width": 1920,
    "height": 1080,
    "fps": 30,
    "profile": 2
  }
}
```

### Motion Detection Enabled

```json
{
  "version": "1.0",
  "motion": {
    "enabled": true,
    "sensitivity": 3,
    "cooldown_time": 10,
    "script_path": "/usr/sbin/motion"
  }
}
```

## Migration from libconfig

If you have an existing `prudynt.cfg` file, you can use the migration script:

```bash
/usr/bin/cfg-to-json.sh /etc/prudynt.cfg /etc/prudynt.json
```

Then validate and adjust the JSON file as needed.

## Troubleshooting

### Validation

Always validate your JSON after editing:

```bash
jq . /etc/prudynt.json
```

### Common Issues

1. **Missing commas**: JSON requires commas between object properties
2. **Trailing commas**: JSON doesn't allow trailing commas
3. **Unquoted keys**: All keys must be quoted strings
4. **Comments**: JSON doesn't support comments

### Restart Service

After configuration changes, restart the prudynt service:

```bash
/etc/init.d/S95prudynt restart
```
