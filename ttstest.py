import pyttsx3
import RPi.GPIO as GPIO
import time

trigger= 4
echo = 17

engine = pyttsx3.init()
engine.say("안녕하세요, 라즈베리파이입니다.")
engine.runAndWait()

def getDistance():
  GPIO.output(trigger, GPIO.LOW)
  time.sleep(0.000002)
  GPIO.output(trigger, GPIO.HIGH)
  time.sleep(0.000002)
  GPIO.output(trigger, GPIO.LOW)
  
  pulse_start=time.time()
  
  timeout = pulse_start + 0.1
  while GPIO.input(echo) == GPIO.LOW and pulse_start < timeout:
    pulse_start = time.time()
  pulse_end = time.time()
  timeout = pulse_end + 0.1
  while GPIO.input(echo) == GPIO.HIGH and pulse_end < timeout:
    pulse_end = time.time()
  pulse_duration = pulse_end - pulse_start
  distance = pulse_duration * 17150
  return distance

def __main__():
  GPIO.setmode(GPIO.BCM):
  GPIO.setup(trigger, GPIO.OUT)
  GPIO.setup(echo.GPIO.IN)
