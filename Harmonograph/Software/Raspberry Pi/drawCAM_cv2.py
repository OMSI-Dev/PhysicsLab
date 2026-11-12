import sys, os
import time
import threading
import cv2
import numpy as np
import datetime

def get_current_dir():
    if getattr(sys, 'frozen', False):
        return sys._MEIPASS
    else:
        return os.path.dirname(os.path.abspath(__file__))

width, height = 1280, 720
display_width, display_height = 1280, 720
internal_width, internal_height = 1280, 720

FPS = 25.0
FRAME_DELAY_MS = max(1, int(1000 / FPS))

current_dir = get_current_dir()
log_file_path = os.path.join(current_dir, "log.txt")

print(current_dir)
print(log_file_path)

log = open(log_file_path, "a")
log.write("Start time of app: " + str(datetime.datetime.now()) + '\n')
log.write("Target resolution: " + str(internal_width) + "x" + str(internal_height) + " (display " + str(display_width) + "x" + str(display_height) + ")\n")
log.write("Camera FPS: " + str(FPS) + '\n')
log.write("Display: OpenCV imshow (no Tkinter, no PIL)\n")
log.write("OpenCV: " + cv2.__version__ + '\n')
log.flush()

log_file_size = os.path.getsize(log_file_path)
print(log_file_size)

def open_camera():
    """Try camera backends in order until one actually delivers a frame.

    GStreamer first (fast MJPG path for USB cameras), then plain V4L2,
    then ANY with property sets, then a plain open. Each candidate is
    verified with a real test read because isOpened() lies on some
    backends (notably GStreamer).
    """
    attempts = [
        ("GStreamer", cv2.CAP_GSTREAMER, True),
        ("V4L2", cv2.CAP_V4L2, True),
        ("ANY", cv2.CAP_ANY, True),
        ("ANY plain", cv2.CAP_ANY, False),
    ]
    for label, backend, with_props in attempts:
        cap = cv2.VideoCapture(0, backend)
        if not cap.isOpened():
            log.write("Camera " + label + ": failed to open\n")
            log.flush()
            cap.release()
            continue
        if with_props:
            cap.set(cv2.CAP_PROP_FPS, FPS)
            cap.set(cv2.CAP_PROP_FOURCC, cv2.VideoWriter.fourcc('M','J','P','G'))
            cap.set(cv2.CAP_PROP_FRAME_WIDTH, internal_width)
            cap.set(cv2.CAP_PROP_FRAME_HEIGHT, internal_height)
        ret, test_frame = cap.read()
        if ret and test_frame is not None:
            h, w = test_frame.shape[:2]
            log.write("Camera opened via " + label + " at " + str(w) + "x" + str(h) + "\n")
            log.flush()
            print("Camera backend: " + label + " (" + str(w) + "x" + str(h) + ")")
            return cap, label
        log.write("Camera " + label + ": opened but test read failed\n")
        log.flush()
        cap.release()
    return None, None

vid, backend_label = open_camera()
if vid is None:
    log.write("ERROR: Camera failed to open on all backends\n")
    log.flush()
    print("ERROR: Camera failed to open on all backends")
    sys.exit(1)

mask_dir1 = os.path.join(current_dir, "assets/images/mask1.png")
mask_dir2 = os.path.join(current_dir, "assets/images/mask2.png")
mask_dir3 = os.path.join(current_dir, "assets/images/mask3.png")
mask_dir4 = os.path.join(current_dir, "assets/images/mask4.png")

log.write("Mask Location:" + mask_dir1 + '\n')
log.flush()

if log_file_size > 1000:
    log.close()
    os.remove(log_file_path)
    log = open(log_file_path, "a")
    log.write("Log Cleared. Over 1000 Bytes" + '\n')
    log.write("Start time of app: " + str(datetime.datetime.now()) + '\n')
    log.write("Target resolution: " + str(internal_width) + "x" + str(internal_height) + " (display " + str(display_width) + "x" + str(display_height) + ")\n")
    log.write("Camera FPS: " + str(FPS) + '\n')
    log.write("Display: OpenCV imshow (no Tkinter, no PIL)\n")
    log.flush()

# Load a mask PNG once (via cv2, no PIL) into precomputed uint16 blend
# arrays. The frame stays BGR end-to-end, so the blend math is identical
# and no color conversion is needed at all.
def load_mask_arrays(path):
    m = cv2.imread(path, cv2.IMREAD_UNCHANGED)
    if m is None:
        raise IOError("cv2.imread failed: " + path)
    if m.ndim == 2:
        m = cv2.cvtColor(m, cv2.COLOR_GRAY2BGR)
    m = cv2.resize(m, (internal_width, internal_height), interpolation=cv2.INTER_LANCZOS4)
    if m.shape[2] == 4:
        rgb = m[:, :, :3]
        a = m[:, :, 3:4]
    else:
        rgb = m
        a = np.full((internal_height, internal_width, 1), 255, dtype=np.uint8)
    rgb16 = rgb.astype(np.uint16)
    a = a.astype(np.uint16)
    a3 = np.repeat(a, 3, axis=2)
    inv3 = (255 - a3).astype(np.uint16)
    return rgb16, a3, inv3

try:
    mask_blend = load_mask_arrays(mask_dir1)
except Exception as e:
    log.write("Mask load error: " + str(e) + '\n')
    log.flush()
    mask_blend = None

stop_event = threading.Event()
worker_thread = None
frame_lock = threading.Lock()
latest_frame = {"img": None, "gen": 0}
render_ms = {"v": 0.0}
fatal = {"msg": None}

def cleanup_and_exit():
    stop_event.set()
    if worker_thread is not None:
        worker_thread.join(timeout=3)
    try:
        log.write("End time of app: " + str(datetime.datetime.now()) + '\n')
        log.flush()
        log.close()
    except Exception:
        pass
    try:
        vid.release()
    except Exception:
        pass
    try:
        cv2.destroyAllWindows()
    except Exception:
        pass

def changeImage(currentMask):
    global mask_blend
    try:
        if currentMask == 1:
            print("Updating Mask")
            mask_blend = load_mask_arrays(mask_dir1)
        elif currentMask == 2:
            mask_blend = load_mask_arrays(mask_dir2)
        elif currentMask == 3:
            mask_blend = load_mask_arrays(mask_dir3)
        elif currentMask == 4:
            mask_blend = load_mask_arrays(mask_dir4)
    except Exception as e:
        log.write("Mask load error: " + str(e) + '\n')
        log.flush()
        mask_blend = None

def capture_worker():
    """Reads and processes frames off the display thread, as fast as the
    camera allows. Only the newest finished frame is kept, so slow frames
    are dropped instead of queueing up.
    """
    next_t = time.monotonic()
    err_count = 0
    last_err_log = 0.0
    while not stop_event.is_set():
        t0 = time.monotonic()
        try:
            ret, frame = vid.read()
        except Exception as e:
            log.write("Exception during read: " + str(e) + '\n')
            log.flush()
            fatal["msg"] = str(e)
            stop_event.set()
            return
        if not ret or frame is None:
            time.sleep(0.01)
            continue
        h, w = frame.shape[:2]
        if (w, h) != (internal_width, internal_height):
            frame = cv2.resize(frame, (internal_width, internal_height), interpolation=cv2.INTER_LINEAR)
        out = frame
        try:
            mb = mask_blend
            if mb is not None:
                # Per-pixel alpha blend in uint16 fixed point (NEON-optimized):
                # out = (frame*(255-a) + mask*a) / 255
                mask_rgb16, a3, inv3 = mb
                f16 = frame.astype(np.uint16)
                t1 = cv2.multiply(f16, inv3)
                t2 = cv2.multiply(mask_rgb16, a3)
                out = cv2.convertScaleAbs(cv2.add(t1, t2), alpha=1.0 / 255.0)
        except Exception as e:
            # Never let a blend error kill the worker thread (silent gray
            # screen). Fall back to the raw frame and log the error.
            err_count += 1
            if err_count <= 5 or time.monotonic() - last_err_log > 2.0:
                last_err_log = time.monotonic()
                log.write("Frame blend error (" + str(err_count) + "x): " + str(e) + '\n')
                log.flush()
                print("Frame blend error: " + str(e))
        with frame_lock:
            latest_frame["img"] = out
            latest_frame["gen"] += 1
        render_ms["v"] = (time.monotonic() - t0) * 1000.0
        # Pace the worker to the target FPS so a faster camera does not waste CPU
        next_t = max(next_t, t0) + 1.0 / FPS
        delay = next_t - time.monotonic()
        if delay > 0:
            time.sleep(delay)

win_name = "Harmonograph"
cv2.namedWindow(win_name, cv2.WINDOW_NORMAL)
cv2.resizeWindow(win_name, 1920, 1080)
# Borderless fullscreen on X11 (removes the title bar via override-redirect)
cv2.setWindowProperty(win_name, cv2.WND_PROP_FULLSCREEN, cv2.WINDOW_FULLSCREEN)

fps_visible = False
last_shown_gen = 0
last_display_t = None
ema_fps = 0.0

def run_display():
    """Display thread: show the newest processed frame at the target rate.

    cv2.imshow + waitKey replaces the Tkinter PhotoImage/X path entirely.
    """
    global fps_visible, last_shown_gen, last_display_t, ema_fps
    while not stop_event.is_set():
        if fatal["msg"] is not None:
            print("Camera error: " + fatal["msg"])
            break
        now = time.monotonic()
        with frame_lock:
            img = latest_frame["img"]
            gen = latest_frame["gen"]
        if img is not None and gen != last_shown_gen:
            last_shown_gen = gen
            if last_display_t is not None:
                inst = 1.0 / max(0.001, now - last_display_t)
                ema_fps = inst if ema_fps == 0.0 else ema_fps * 0.9 + inst * 0.1
            last_display_t = now
            if fps_visible:
                text = "FPS: %.1f  (%.0f ms/frame)" % (ema_fps, render_ms["v"])
                cv2.putText(img, text, (30, 80), cv2.FONT_HERSHEY_SIMPLEX, 2.0, (0, 0, 255), 4, cv2.LINE_AA)
                cv2.putText(img, text, (30, 80), cv2.FONT_HERSHEY_SIMPLEX, 2.0, (255, 255, 255), 2, cv2.LINE_AA)
            cv2.imshow(win_name, img)
        key = cv2.waitKey(FRAME_DELAY_MS) & 0xFF
        if key == 27:  # Esc
            break
        elif key == ord('1'):
            changeImage(1)
        elif key == ord('2'):
            changeImage(2)
        elif key == ord('3'):
            changeImage(3)
        elif key == ord('4'):
            changeImage(4)
        elif key == ord('5'):
            fps_visible = not fps_visible
    cv2.destroyAllWindows()

worker_thread = threading.Thread(target=capture_worker, daemon=True)
worker_thread.start()

run_display()
cleanup_and_exit()
