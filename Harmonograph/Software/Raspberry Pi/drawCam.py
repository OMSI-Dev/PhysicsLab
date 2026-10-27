import sys, os
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

app = tk.Tk()

vid = cv2.VideoCapture(0, cv2.CAP_ANY)
FPS = 60.0
vid.set(cv2.CAP_PROP_FPS, FPS)
vid.set(cv2.CAP_PROP_FOURCC, cv2.VideoWriter.fourcc('M','J','P','G'))
vid.set(cv2.CAP_PROP_FRAME_WIDTH, 1920)
vid.set(cv2.CAP_PROP_FRAME_HEIGHT, 1080)

current_dir = get_current_dir()
log_file_path = os.path.join(current_dir, "log.txt")

print(current_dir)
print(log_file_path)

log = open(log_file_path, "a")
log.write("Start time of app: " + str(datetime.datetime.now()) + '\n')
log.write("Camera resolution: " + str(width) + "x" + str(height) + '\n')
log.write("Camera FPS: " + str(FPS) + '\n')
log.flush()

log_file_size = os.path.getsize(log_file_path)
print(log_file_size)

mask_dir = current_dir + "/assets/images/mask.png"
log = open(log_file_path, "a")
log.write("Mask Location:" + mask_dir + '\n')
log.flush()

if log_file_size > 1000:
    log.close()
    os.remove(log_file_path)
    log = open(log_file_path, "a")
    log.write("Log Cleared. Over 1000 Bytes" + '\n')
    log.write("Start time of app: " + str(datetime.datetime.now()) + '\n')
    log.write("Camera resolution: " + str(width) + "x" + str(height) + '\n')
    log.write("Camera FPS: " + str(FPS) + '\n')
    log.flush()

label_Cam = tk.Label(app, width=width, height=height)
label_Cam.pack()

# Load mask once, resize to frame size, keep alpha
try:
    mask_img = Image.open(mask_dir).convert("RGBA")
    mask_img = mask_img.resize((width, height), Image.LANCZOS)
except Exception as e:
    log.write("Mask load error: " + str(e) + '\n')
    log.flush()
    mask_img = None

def cleanup_and_exit():
    log.write("End time of app: " + str(datetime.datetime.now()) + '\n')
    log.flush()
    log.close()
    vid.release()
    app.destroy()

def camRead():
    try:
        ret, frame = vid.read()
    except Exception as e:
        log.write("Exception during read: " + str(e) + '\n')
        log.flush()
        cleanup_and_exit()
        sys.exit(1)
        return

    if not ret or frame is None:
        label_Cam.after(5, camRead)
        return

    cvImg = cv2.cvtColor(frame, cv2.COLOR_BGR2RGB)
    capImg = Image.fromarray(cvImg).convert("RGBA")

    if mask_img is not None:
        # Composite mask over camera frame
        composite = Image.alpha_composite(capImg, mask_img)
    else:
        composite = capImg

    tkImage = ImageTk.PhotoImage(image=composite)
    label_Cam.photo = tkImage
    label_Cam.configure(image=tkImage, width=width, height=height)
    label_Cam.after(5, camRead)

app.bind('<Escape>', lambda e: cleanup_and_exit())
app.protocol("WM_DELETE_WINDOW", cleanup_and_exit)

app.geometry("1920x1080")
app.overrideredirect(True)

camRead()

try:
    app.mainloop()
except KeyboardInterrupt:
    cleanup_and_exit()
