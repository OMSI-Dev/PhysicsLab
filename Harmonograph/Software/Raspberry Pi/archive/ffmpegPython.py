import cv2
import subprocess
import numpy as np

# Replace "YOUR_CAMERA_NAME" with the name of your camera from the ffmpeg list
camera_name = "YOUR_CAMERA_NAME"

# Command to capture video using ffmpeg with mjpeg codec
ffmpeg_command = [
    'ffmpeg',
    '-f', 'dshow',  # Input format for Windows
    '-i', f'video={camera_name}',  # Input device
    '-vcodec', 'mjpeg',  # Video codec
    '-r', '30',  # Frame rate
    '-s', '1920x1080',  # Resolution
    '-f', 'image2pipe',  # Output format
    '-pix_fmt', 'bgr24',  # Pixel format for OpenCV
    '-vcodec', 'rawvideo',  # Output codec
    '-'
]

# Open the ffmpeg process
process = subprocess.Popen(ffmpeg_command, stdout=subprocess.PIPE, stderr=subprocess.PIPE)

# Read the ffmpeg output using OpenCV
while True:
    # Capture frame-by-frame
    frame_size = 1920 * 1080 * 3  # Width * Height * 3 (for bgr24)
    raw_frame = process.stdout.read(frame_size)
    if len(raw_frame) != frame_size:
        break
    
    # Convert raw frame to numpy array
    frame = np.frombuffer(raw_frame, np.uint8).reshape((1080, 1920, 3))
    
    # Display the resulting frame
    cv2.imshow('Webcam Video', frame)

    # Press 'q' to quit the video
    if cv2.waitKey(1) & 0xFF == ord('q'):
        break

# When everything is done, release the capture
process.terminate()
cv2.destroyAllWindows()
