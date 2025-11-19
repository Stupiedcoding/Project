import pyttsx3
import RPi.GPIO as GPIO
import time

trigger = 4  
echo = 17    
SPEED_OF_SOUND_HALF = 17150

def getDistance():

    GPIO.output(trigger, GPIO.LOW)
    time.sleep(0.000002)
    GPIO.output(trigger, GPIO.HIGH)
    time.sleep(0.00001) 
    GPIO.output(trigger, GPIO.LOW)
    
    pulse_start = time.time()
    
    timeout = time.time() + 0.02
    while GPIO.input(echo) == GPIO.LOW:
        pulse_start = time.time()
        if time.time() > timeout:
            return -1  # timeout

    timeout = time.time() + 0.02
    while GPIO.input(echo) == GPIO.HIGH:
        pulse_end = time.time()
        if time.time() > timeout:
            return -1

    pulse_duration = pulse_end - pulse_start
    distance = pulse_duration * SPEED_OF_SOUND_HALF
    
    return distance

def main():
    engine = pyttsx3.init()
    
    try:
        GPIO.setmode(GPIO.BCM)
        GPIO.setup(trigger, GPIO.OUT)
        GPIO.setup(echo, GPIO.IN) 
        
        GPIO.output(trigger, GPIO.LOW)
        time.sleep(0.5)
        
        while True:
            fDistance = getDistance()
            print(f"Distance: {fDistance:.2f} cm")
            if fDistance < 30 and fDistance > 0.1: 
                engine.say("안녕하세요, 라즈베리파이입니다.")
                engine.runAndWait()
                time.sleep(2)
            else:
                time.sleep(0.1) 
                
    except KeyboardInterrupt:
        print("프로그램 종료")
    finally:
        GPIO.cleanup()
        
if __name__ == '__main__':
    main()
