import pyttsx3
import RPi.GPIO as GPIO
import time

trigger= 4
echo = 17

engine = pyttsx3.init()
engine.say("안녕하세요, 라즈베리파이입니다.")
engine.runAndWait()

def getDistance():
  #GPIO.setmode(GPIO.BCM)
  GPIO.setup(GPIO_TRIGGER, GPIO.OUT)
  time.sleep(0.000002)
  GPIO.setup(GPIO_ECHO, GPIO.IN)
  time.sleep(0.000002)
  GPIO.output(GPIO_TRIGGER, GPIO.LOW)
  #GPIO setting
  
  pulse_start=time.time()
  
  timeout = pulse_start + 0.1
  while GPIO.input(GPIO_ECHO) == GPIO.LOW and :
    start_time = 
