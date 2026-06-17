import socket
import json
import RPi.GPIO as GPIO
import numpy as np
import time

# pin
BASE_PIN, ARM_PIN, LAUNCH_PIN = 17, 27, 22
PORT = 5005

# for safety: Prevent the servo from passing a limit
# THE ROBOT BASE 
MIN_BASE = 0
MAX_BASE = 180
CENTER_BASE = 90.0  # Facing forward

# THE ARM (Tilt)
MIN_ARM = 0
MAX_ARM = 45
CENTER_ARM = 0.0    # Resting/Level position

# Current positions (Start variables at their specific centers)
cur_x = CENTER_BASE
cur_y = CENTER_ARM

# Speed Control: 
MOVE_STEP = 0.8 

# Fire Control
LAST_SHOT_TIME = 0
SHOT_COOLDOWN = 2.0 # Seconds between shots

# GPIO Setup
GPIO.setmode(GPIO.BCM)
GPIO.setup([BASE_PIN, ARM_PIN, LAUNCH_PIN], GPIO.OUT)

base_pwm = GPIO.PWM(BASE_PIN, 50)
arm_pwm = GPIO.PWM(ARM_PIN, 50)
launcher_pwm = GPIO.PWM(LAUNCH_PIN, 50)

# Start at Center : SAFE STARTUP PROCEDURE 
def set_angle_safe(pwm, angle, pin_type):
    """Clamps angle based on which motor it is"""
    if pin_type == "base":
        clamped = np.clip(angle, MIN_BASE, MAX_BASE)
    else: # arm
        clamped = np.clip(angle, MIN_ARM, MAX_ARM)
        
    duty = 2.5 + (clamped / 180.0) * 10.0
    pwm.ChangeDutyCycle(duty)

def soft_start_servos():
    global cur_x, cur_y
    print("Safe Wake-up Sequence Initiated...")
    
    base_pwm.start(0)
    arm_pwm.start(0)
    launcher_pwm.start(0)
    
    # move Arm to its Center 
    set_angle_safe(arm_pwm, CENTER_ARM, "arm")
    time.sleep(0.5)

    # sweep Base to its Center (90)
    # We move from 0 to 90 slowly
    temp_base = 0.0
    while temp_base <= CENTER_BASE:
        set_angle_safe(base_pwm, temp_base, "base")
        temp_base += 1.0
        time.sleep(0.02)
    
    cur_x = CENTER_BASE
    cur_y = CENTER_ARM
    print("Turret Ready. Base: {cur_x}, Arm: {cur_y}")


def shoot():
    global LAST_SHOT_TIME
    current_time = time.time()
    if current_time - LAST_SHOT_TIME > SHOT_COOLDOWN:
        print("SAFE SHOOT!")
        set_angle_safe(launcher_pwm, 120)
        time.sleep(0.3) # Time for trigger to move
        set_angle_safe(launcher_pwm, 10) # Return to safe rest
        LAST_SHOT_TIME = current_time

# MAIN

# Run Soft Start
soft_start_servos()
# UDP Setup
sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
sock.bind(("0.0.0.0", PORT))
sock.setblocking(False)

print("Pi Safety-Receiver Active. Step Speed: {MOVE_STEP}")

try:
    while True:
        try:
            message, addr = sock.recvfrom(1024)
            data = json.loads(message.decode())
            
            # 1. Calculate the POTENTIAL angles from the XYZ data
            yaw = np.degrees(np.arctan2(data['x'], data['z']))
            pitch = np.degrees(np.arctan2(-data['y'], np.sqrt(data['x']**2 + data['z']**2)))

            # 2. Add the base/center offsets
            raw_target_x = CENTER_BASE + yaw
            raw_target_y = CENTER_ARM + pitch

            # 3. THE PHYSICAL CHECK (The Safety Shield)
            # np.clip : freeze the movement at A physical wall
            safe_target_x = np.clip(raw_target_x, MIN_BASE, MAX_BASE)
            safe_target_y = np.clip(raw_target_y, MIN_ARM, MAX_ARM)

            # 4. Feedback (Print a warning if hitting a wall)
            if raw_target_y > MAX_ARM:
               print(f"Limit reached! Target was {raw_target_y:.1f}°, capping at {MAX_ARM}°")

            # 5. move towards the SAFE targets
            if abs(cur_x - safe_target_x) > 0.5:
               diff_x = np.clip(safe_target_x - cur_x, -MOVE_STEP, MOVE_STEP)
               cur_x += diff_x
               set_angle_safe(base_pwm, cur_x, "base")

            if abs(cur_y - safe_target_y) > 0.5:
               diff_y = np.clip(safe_target_y - cur_y, -MOVE_STEP, MOVE_STEP)
               cur_y += diff_y
               set_angle_safe(arm_pwm, cur_y, "arm")

            # 6. Precision Shooting
            if abs(yaw) < 1.5 and abs(pitch) < 1.5:
                shoot()

        except BlockingIOError:
            pass # Wait for next UDP packet
        
        time.sleep(0.01) # Maintains ~100Hz loop for smooth movement

except KeyboardInterrupt:
    print("\nShutting down safely...")
finally:
    print("\nShutting down safely...")
    
    # 1. Return to specific positions
    # Base to 90, Arm to 0
    set_angle_safe(base_pwm, CENTER_BASE, "base")
    set_angle_safe(arm_pwm, CENTER_ARM, "arm")
    
    # 2. Give it enough time to physically reach home
    time.sleep(1.0)
    
    # 3. Kill the signals
    base_pwm.stop()
    arm_pwm.stop()
    launcher_pwm.stop()
    
    # 4. Release the pins for other programs
    GPIO.cleanup()
    print("Cleanup complete. Turret is at rest.")
