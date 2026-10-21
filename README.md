# Raspberry Pi 5 — YOLO / NCNN Object Detection on Yocto

This project runs a custom Object Detection model on Raspberry Pi 5 using a custom Yocto Linux image.

The model was trained on my own dataset with photos of my cat, Yuna.

## What was done

### Level 1

- Created my own dataset with photos of Yuna.
- Labeled 165 images.
- Model class: `yuna`.
- Dataset split:
  - 120 images for training;
  - 45 images for validation.
- Trained the model using Ultralytics YOLO.
- Tested the trained model on separate images and on live video.

Validation results:

- Precision: `0.951`
- Recall: `0.973`
- mAP50: `0.960`
- mAP50-95: `0.839`

After training, the model was exported to NCNN format for inference on Raspberry Pi.

---

## Level 2 — Yocto + Raspberry Pi 5

The model was integrated into a custom Yocto image for Raspberry Pi 5.

The image includes:

- NCNN;
- Yuna detection model;
- `libcamera`;
- `libpisp`;
- IMX219 camera support;
- `yuna-live` detection application;
- automatic camera startup;
- automatic detector startup;
- Wi-Fi configuration from the SD card boot partition.

System:

```text
Yocto / Poky 5.0.21
Scarthgap
Raspberry Pi 5
```

Camera:

```text
Raspberry Pi IMX219 8MP
```

---

## How it works

After Raspberry Pi starts:

```text
IMX219 camera
      ↓
libcamera / cam
      ↓
RAW frames
      ↓
yuna-live
      ↓
NCNN model
      ↓
TCP :5000
      ↓
Windows viewer
```

`yuna-live` performs inference directly on Raspberry Pi.

The model is stored in:

```text
/usr/share/yuna-detector/model.ncnn.param
/usr/share/yuna-detector/model.ncnn.bin
```

---

## Automatic startup

After Raspberry Pi boots, these services start automatically:

```text
yuna-live.service
yuna-camera.service
wifi-setup.service
```

They can be checked with:

```bash
systemctl status yuna-live.service
systemctl status yuna-camera.service
systemctl status wifi-setup.service
```

---

## Wi-Fi

The Wi-Fi SSID and password are not stored inside the Yocto image.

After writing the image to the SD card, create this file in the root of the boot partition:

```text
wpa_supplicant.conf
```

Example:

```ini
country=UA

network={
    ssid="MyWiFi"
    psk="MyPassword"
}
```

On boot, Raspberry Pi reads this file and uses it to configure Wi-Fi.

This makes it possible to change the Wi-Fi network without rebuilding the Yocto image.

---

## Building the Yocto image

In WSL:

```bash
cd ~/Projects/ros2-yocto-rpi5
source setup-build.sh
```

Build the image:

```bash
bitbake ros2-counter-image
```

The generated files are located in:

```text
build/tmp/deploy/images/raspberrypi5/
```

The SD card image is:

```text
ros2-counter-image-jazzy-raspberrypi5.rootfs.wic.bz2
```

It can be unpacked with:

```bash
bunzip2 -kc \
  ros2-counter-image-jazzy-raspberrypi5.rootfs.wic.bz2 \
  > ros2-counter-image-jazzy-raspberrypi5.img
```

The `.img` file can then be written to the SD card using Raspberry Pi Imager.

---

## Results

The camera stream works at approximately:

```text
15 FPS
```

During testing, CPU usage of the `yuna-live` process changed depending on the current workload.

The highest CPU usage I observed during testing was approximately:

```text
68%
```

Live Yuna detection was tested and recorded on video.

The video shows:

- live video from Raspberry Pi;
- bounding box around Yuna;
- detection confidence;
- NCNN inference time;
- FPS.

---

## Issue found during testing

During testing, I found a problem with RAW frames.

When the Windows viewer was not connected, the camera continued writing frames to:

```text
/tmp/livecam
```

This caused `/tmp` to grow up to approximately `3.9 GB`.

I updated `yuna-live` so that old frames are removed even when no viewer is connected.

After the fix, `/tmp/livecam` keeps only a few recent frames and uses only a few MB.

---

## Level 3

Level 3 with separate ROS 2 nodes for Computer Vision was not implemented.

This project includes Level 1 and Level 2.