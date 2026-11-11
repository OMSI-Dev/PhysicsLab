import sys, os
import time
import threading
import cv2
import tkinter as tk
from PIL import Image, ImageTk
import datetime

def get_current_dir():
    if getattr(sys, 'frozen', False):
        return sys._MEIPASS
    else:
        return os.path.dirname(os.path.abspath(__file__))

width, height = 1920, 1080
display_width, display_height = 1920, 1080
internal_width, internal_height = 1280, 720

app = tk.Tk()

vid = cv2.VideoCapture(0, cv2.CAP_GSTREAMER)
FPS = 25.0
FRAME_DELAY_MS = max(1, int(1000 / FPS))
vid.set(cv2.CAP_PROP_FPS, FPS)
vid.set(cv2.CAP_PROP_FOURCC, cv2.VideoWriter.fourcc('M','J','P','G'))
vid.set(cv2.CAP_PROP_FRAME_WIDTH, internal_width)
vid.set(cv2.CAP_PROP_FRAME_HEIGHT, internal_height)

current_dir = get_current_dir()
log_file_path = os.path.join(current_dir, "log.txt")

print(current_dir)
print(log_file_path)

log = open(log_file_path, "a")
log.write("Start time of app: " + str(datetime.datetime.now()) + '\n')
log.write("Camera resolution: " + str(internal_width) + "x" + str(internal_height) + " (display " + str(display_width) + "x" + str(display_height) + ")\n")
log.write("Camera FPS: " + str(FPS) + '\n')
log.write("Pipeline: threaded capture, latest-frame display\n")
log.flush()

log_file_size = os.path.getsize(log_file_path)
print(log_file_size)

if not vid.isOpened():
    log.write("ERROR: Camera failed to open\n")
    log.flush()
    print("ERROR: Camera failed to open")
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
    log.write("Camera resolution: " + str(internal_width) + "x" + str(internal_height) + " (display " + str(display_width) + "x" + str(display_height) + ")\n")
    log.write("Camera FPS: " + str(FPS) + '\n')
    log.flush()

label_Cam = tk.Label(app)
label_Cam.pack(fill="both", expand=True)

fps_label = tk.Label(app, text="", font=("arial", 48), bg="black", fg="white")
fps_label.place(x=50, y=50, anchor="nw")

# Load mask once, resize to internal frame size, keep alpha
try:
    mask_img = Image.open(mask_dir1).convert("RGBA")
    mask_img = mask_img.resize((internal_width, internal_height), Image.LANCZOS)
except Exception as e:
    log.write("Mask load error: " + str(e) + '\n')
    log.flush()
    mask_img = None

def grab_focus():
    app.lift()
    app.focus_force()
    app.grab_set()
    app.after(100, app.focus_force)

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
        app.destroy()
    except Exception:
        pass

def changeImage(currentMask):
    global mask_img
    try:
        if currentMask == 1:
            print("Updating Mask")
            mask_img = Image.open(mask_dir1).convert("RGBA")
            mask_img = mask_img.resize((internal_width, internal_height), Image.LANCZOS)
        elif currentMask == 2:
            mask_img = Image.open(mask_dir2).convert("RGBA")
            mask_img = mask_img.resize((internal_width, internal_height), Image.LANCZOS)
        elif currentMask == 3:
            mask_img = Image.open(mask_dir3).convert("RGBA")
            mask_img = mask_img.resize((internal_width, internal_height), Image.LANCZOS)
        elif currentMask == 4:
            mask_img = Image.open(mask_dir4).convert("RGBA")
            mask_img = mask_img.resize((internal_width, internal_height), Image.LANCZOS)
    except Exception as e:
        log.write("Mask load error: " + str(e) + '\n')
        log.flush()
        mask_img = None

def capture_worker():
    """Reads and processes frames off the UI thread, as fast as the camera allows.

    The UI thread only ever sees the newest finished frame, so slow frames
    are dropped instead of queueing up.
    """
    next_t = time.monotonic()
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
        cvImg = cv2.cvtColor(frame, cv2.COLOR_BGR2RGB)
        capImg = Image.fromarray(cvImg).convert("RGBA")
        mask = mask_img
        if mask is not None:
            composite = Image.alpha_composite(capImg, mask)
        else:
            composite = capImg
        if (display_width, display_height) != (internal_width, internal_height):
            # NEAREST is ~40x faster than LANCZOS/BILINEAR on the Pi;
            # fine for a 1.5x upscale of live camera video.
            composite = composite.resize((display_width, display_height), Image.NEAREST)
        with frame_lock:
            latest_frame["img"] = composite
            latest_frame["gen"] += 1
        render_ms["v"] = (time.monotonic() - t0) * 1000.0
        # Pace the worker to the target FPS so a faster camera does not waste CPU
        next_t = max(next_t, t0) + 1.0 / FPS
        delay = next_t - time.monotonic()
        if delay > 0:
            time.sleep(delay)

fps_visible = False

def toggle_fps(event):
    global fps_visible
    fps_visible = not fps_visible
    if not fps_visible:
        fps_label.place_forget()
    else:
        fps_label.place(x=50, y=50, anchor="nw")

last_shown_gen = 0
last_display_t = None
ema_fps = 0.0

def show_frame():
    """UI thread: display the newest processed frame at the target rate."""
    global last_shown_gen, last_display_t, ema_fps
    if fatal["msg"] is not None:
        print("Camera error: " + fatal["msg"])
        cleanup_and_exit()
        return
    now = time.monotonic()
    with frame_lock:
        img = latest_frame["img"]
        gen = latest_frame["gen"]
    if img is not None and gen != last_shown_gen:
        last_shown_gen = gen
        tkImage = ImageTk.PhotoImage(image=img)
        label_Cam.photo = tkImage
        label_Cam.configure(image=tkImage)
        if last_display_t is not None:
            inst = 1.0 / max(0.001, now - last_display_t)
            ema_fps = inst if ema_fps == 0.0 else ema_fps * 0.9 + inst * 0.1
        last_display_t = now
        if fps_visible:
            fps_label.config(text=f"FPS: {ema_fps:.1f}  ({render_ms['v']:.0f} ms/frame)")
    label_Cam.after(FRAME_DELAY_MS, show_frame)

app.bind('<Escape>', lambda e: cleanup_and_exit())
app.bind('<KeyPress-1>', lambda e: changeImage(1))
app.bind('<KeyPress-2>', lambda e: changeImage(2))
app.bind('<KeyPress-3>', lambda e: changeImage(3))
app.bind('<KeyPress-4>', lambda e: changeImage(4))
app.bind('<KeyPress-5>', toggle_fps)
app.protocol("WM_DELETE_WINDOW", cleanup_and_exit)

app.geometry("1920x1080")
app.overrideredirect(True)

worker_thread = threading.Thread(target=capture_worker, daemon=True)
worker_thread.start()

show_frame()

try:
    app.after(50, grab_focus)
    app.mainloop()
except KeyboardInterrupt:
    cleanup_and_exit()
