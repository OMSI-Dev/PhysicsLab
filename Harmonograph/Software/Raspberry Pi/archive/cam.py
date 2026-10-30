import sys, os
import cv2
import tkinter as tk
from PIL import Image, ImageTk

vid = cv2.VideoCapture(0,cv2.CAP_DSHOW)

width, height = 1920, 1080
vid.set(cv2.CAP_PROP_FRAME_WIDTH, width)
vid.set(cv2.CAP_PROP_FRAME_HEIGHT, height)

#create app interface
app = tk.Tk()
#add escape key
app.bind('<Escape>', lambda e: app.quit())
#set screen resoltion & borderless
app.geometry("1920x1080")
app.overrideredirect(True)

#vars
heighty = 1080
widthx = 1920

log = open("log.txt", "a")

#create a background file
if os.path.exists('./assets/images/background.jpg'):
    log.write("Path is true.." + '\n')
    imPath = os.path.abspath('./assets/images/background.jpg')
    print("in relative path" +'\n')       
else:
    log.write("Path is false.." + '\n')
    #imPath = os.path.abspath('./_internal/assets/images/background.jpg')
    print("in exe path" +'\n')    

print(imPath)
log.write("looking at:" + imPath +'\n')
log.close()

backgroundIm = ImageTk.PhotoImage(file = imPath)

backgroundFrame = tk.Frame(app)
backgroundFrame.pack(fill='both',expand=True)
backgroundFrame.pack_propagate(False)

labelBG = tk.Label(backgroundFrame)
labelBG.pack()
labelBG.pack_propagate(False)

labelBG.photo = backgroundIm
labelBG.configure(image=backgroundIm)

#create a label to hold the camera frames
label_Cam = tk.Label(backgroundFrame)
label_Cam.pack()
label_Cam.place(relx=0.5,rely=0.5,anchor="center")

def camRead():

    ret, frame = vid.read()

    cvImg = cv2.cvtColor(frame, cv2.COLOR_BGR2RGB)
    capImg = Image.fromarray(cvImg)
    tkImage = ImageTk.PhotoImage(image=frame)

    label_Cam.photo = tkImage
    label_Cam.configure(image=tkImage)
    label_Cam.after(5,camRead)

print("Start Camera")
camRead()
tk.mainloop()


vid.release()
