# ISP bypass

Think of “ISP bypass” as “skip most of the on‑chip image signal processing.” On Ingenic T31, that means many ISP modules (noise reduction, color pipeline, AE/AWB hooks, etc.) are disabled or not applied when bypass is ON, while the capture/framesource/encoder path still runs.

## When ISP bypass is ON

Pros:
- Lower latency and slightly lower power/bandwidth (less processing in the ISP)
- Can sidestep ISP-stage quirks on some kernels
- Sometimes allows higher sustainable FPS on constrained systems

Cons:
- Image quality typically drops:
  - Less or no denoise/temper/sinter, DRC/WDR, lens shading, color correction, gamma
  - 50/60 Hz anti‑flicker and exposure control may be ineffective
  - Colors/noise/low‑light performance are worse, especially at night
- Some ISP tuning calls become no‑ops (so the “Set…Strength/Mode” knobs won’t do much)

Best for:
- Latency‑sensitive or very constrained builds, or when working around ISP issues

## When ISP bypass is OFF

Pros:
- Full image processing pipeline active:
  - Demosaic, black level, lens shading, color correction, gamma
  - Temporal/spatial noise reduction, DRC/WDR, anti‑flicker, AE/AWB hooks
- Much better low‑light quality, fewer artifacts and banding, more consistent colors

Cons:
- Slightly higher latency and power/memory bandwidth
- More sensitivity to mismatched ISP configuration (wrong sizes/formats can break the pipeline if not set correctly)

Best for:
- Production image quality (day/night), stable lighting control, and consistent color

## Practical guidance

- If you care most about quality (especially low light, indoor flicker): keep bypass OFF.
- If you’re optimizing for latency or have a kernel that misbehaves with the ISP modules: keep it ON.
- Try ISP bypass OFF and compare:
  - Noise/grain in low light
  - Color accuracy/white balance
  - Flicker performance under 50/60 Hz lighting
  - Latency (glass‑to‑glass) differences
