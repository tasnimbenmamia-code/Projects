import cv2 
from ultralytics import YOLO
import socket
import json
import numpy as np


PI_IP = "192.168.1.XX"  #  Pi's IP
PORT = 5005
REAL_BOTTLE_HEIGHT = 0.25 
FOCAL_LENGTH = 600
CONF_THRESHOLD = 0.5

# Offsets (Meters)
OFFSET_X, OFFSET_Y, OFFSET_Z = 0.0, -0.10, 0.05

# Networking Setup
sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)

model = YOLO("yolo11n.pt")
cap = cv2.VideoCapture(0)
cx = int(cap.get(cv2.CAP_PROP_FRAME_WIDTH) // 2)
cy = int(cap.get(cv2.CAP_PROP_FRAME_HEIGHT) // 2)

try:
    while cap.isOpened():
        ret, frame = cap.read()
        if not ret: break

        results = model.predict(source=frame, conf=CONF_THRESHOLD, verbose=False)
        for box in results[0].boxes:
            if model.names[int(box.cls[0])] == "bottle":
                x1, y1, x2, y2 = box.xyxy[0].cpu().numpy()
                h_pix = y2 - y1
                
                # 3D Math
                Z = (FOCAL_LENGTH * REAL_BOTTLE_HEIGHT) / h_pix
                X = ((int((x1 + x2) / 2)) - cx) * Z / FOCAL_LENGTH
                Y = ((int((y1 + y2) / 2)) - cy) * Z / FOCAL_LENGTH

                # the Offset
                data = {"x": X + OFFSET_X, "y": Y + OFFSET_Y, "z": Z + OFFSET_Z}
                
                # send via UDP
                sock.sendto(json.dumps(data, default=float).encode(), (PI_IP, PORT))
                
                cv2.rectangle(frame, (int(x1), int(y1)), (int(x2), int(y2)), (0, 255, 0), 2)
                break 

        cv2.imshow("UDP Brain (PC)", frame)
        if cv2.waitKey(1) & 0xFF == ord('q'): break
finally:
    cap.release()
    cv2.destroyAllWindows()
