# Prudynt JSON Configuration Quick Reference

## Common Configuration Tasks

### Enable Motion Detection

```bash
jct /etc/prudynt.json set motion.enabled true
jct /etc/prudynt.json set motion.sensitivity 3
jct /etc/prudynt.json set motion.cooldown_time 10
```

### Change Stream Quality

```bash
# Ultra High Quality (for local recording)
jct /etc/prudynt.json set stream0.bitrate 8000
jct /etc/prudynt.json set stream0.mode "VBR"
jct /etc/prudynt.json set stream0.profile 2
jct /etc/prudynt.json set stream0.max_bitrate 12000
jct /etc/prudynt.json set stream0.min_qp 18
jct /etc/prudynt.json set stream0.max_qp 35

# High Quality (for fast internet)
jct /etc/prudynt.json set stream0.bitrate 5000
jct /etc/prudynt.json set stream0.mode "VBR"
jct /etc/prudynt.json set stream0.max_bitrate 7500
jct /etc/prudynt.json set stream0.min_qp 20
jct /etc/prudynt.json set stream0.max_qp 40

# Balanced Quality (default)
jct /etc/prudynt.json set stream0.bitrate 3000
jct /etc/prudynt.json set stream0.mode "CBR"
jct /etc/prudynt.json set stream0.min_qp 25
jct /etc/prudynt.json set stream0.max_qp 45

# Low Bandwidth (for slow internet)
jct /etc/prudynt.json set stream1.bitrate 500
jct /etc/prudynt.json set stream1.width 640
jct /etc/prudynt.json set stream1.height 360
jct /etc/prudynt.json set stream1.mode "CBR"
jct /etc/prudynt.json set stream1.min_qp 30
jct /etc/prudynt.json set stream1.max_qp 50
```

### Configure RTSP Authentication

```bash
jct /etc/prudynt.json set rtsp.auth_required true
jct /etc/prudynt.json set rtsp.username "admin"
jct /etc/prudynt.json set rtsp.password "secure123"
```

### Adjust Image Settings

```bash
# Flip image
jct /etc/prudynt.json set image.vflip true
jct /etc/prudynt.json set image.hflip false

# Adjust brightness and contrast
jct /etc/prudynt.json set image.brightness 140
jct /etc/prudynt.json set image.contrast 120
```

### Configure Audio

```bash
# Enable audio input
jct /etc/prudynt.json set audio.input_enabled true
jct /etc/prudynt.json set audio.input_format "OPUS"
jct /etc/prudynt.json set audio.input_bitrate 64

# Enable on streams
jct /etc/prudynt.json set stream0.audio_enabled true
jct /etc/prudynt.json set stream1.audio_enabled true
```

### OSD Configuration

```bash
# Enable OSD
jct /etc/prudynt.json set stream0.osd.enabled true
jct /etc/prudynt.json set stream0.osd.time_enabled true
jct /etc/prudynt.json set stream0.osd.user_text_enabled true

# Position elements
jct /etc/prudynt.json set stream0.osd.pos_time_x 20
jct /etc/prudynt.json set stream0.osd.pos_time_y 20
```

### WebSocket Settings

```bash
# Enable WebSocket
jct /etc/prudynt.json set websocket.enabled true
jct /etc/prudynt.json set websocket.port 8089
```

### JPEG Snapshots

```bash
# Configure snapshots
jct /etc/prudynt.json set stream2.enabled true
jct /etc/prudynt.json set stream2.jpeg_quality 85
jct /etc/prudynt.json set stream2.jpeg_refresh 500
```

### Advanced Quality Tuning

```bash
# Enable adaptive features for better quality
jct /etc/prudynt.json set stream0.adaptive_gop true
jct /etc/prudynt.json set stream0.scene_change_detection true

# Fine-tune QP for specific scenarios
# For security cameras (prioritize detail)
jct /etc/prudynt.json set stream0.min_qp 15
jct /etc/prudynt.json set stream0.max_qp 35

# For wildlife cameras (balance quality/battery)
jct /etc/prudynt.json set stream0.min_qp 25
jct /etc/prudynt.json set stream0.max_qp 45

# For indoor monitoring (consistent quality)
jct /etc/prudynt.json set stream0.mode "CBR"
jct /etc/prudynt.json set stream0.min_qp 28
jct /etc/prudynt.json set stream0.max_qp 42

# For outdoor monitoring (variable conditions)
jct /etc/prudynt.json set stream0.mode "VBR"
jct /etc/prudynt.json set stream0.max_bitrate 6000
jct /etc/prudynt.json set stream0.min_qp 20
jct /etc/prudynt.json set stream0.max_qp 50
```

## Quick Checks

### View Current Settings

```bash
# Check motion detection
jct /etc/prudynt.json get motion.enabled

# Check stream settings
jct /etc/prudynt.json get stream0.bitrate
jct /etc/prudynt.json get stream0.mode

# Check RTSP settings
jct /etc/prudynt.json get rtsp.port
jct /etc/prudynt.json get rtsp.auth_required
```

### Print Sections

```bash
# Print entire config
jct /etc/prudynt.json print

# Print specific sections (requires jq)
jct /etc/prudynt.json print | jq .motion
jct /etc/prudynt.json print | jq .stream0
jct /etc/prudynt.json print | jq .rtsp
```

## Restart Service

After making changes, restart the service:

```bash
/etc/init.d/S95prudynt restart
```

## Backup and Restore

### Backup Configuration

```bash
cp /etc/prudynt.json /etc/prudynt.json.backup
```

### Restore Configuration

```bash
cp /etc/prudynt.json.backup /etc/prudynt.json
/etc/init.d/S95prudynt restart
```

## Validation

Always validate JSON after manual editing:

```bash
jq . /etc/prudynt.json
```

If validation fails, restore from backup and try again.

## Default Values

If a setting is not specified in the JSON file, prudynt will use its built-in default values. You only need to specify settings you want to change from the defaults.

## Color Values

OSD colors are specified as decimal ARGB values:

- White: `4294967295` (0xFFFFFFFF)
- Black: `4278190080` (0xFF000000)  
- Red: `4294901760` (0xFFFF0000)
- Green: `4278255360` (0xFF00FF00)
- Blue: `4278190335` (0xFF0000FF)

## Format Strings

### Time Format

Uses strftime format:
- `%F %T`: 2023-12-25 14:30:45
- `%Y-%m-%d %H:%M:%S`: 2023-12-25 14:30:45
- `%a %b %d %H:%M`: Mon Dec 25 14:30

### User Text Variables

- `%hostname`: System hostname
- Custom text: Any static string

### Uptime Format

Uses printf format for hours:minutes:seconds:
- `"Uptime: %02lu:%02lu:%02lu"`: Uptime: 12:34:56
