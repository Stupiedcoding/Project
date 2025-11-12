import speech_recognition as sr
import RPi.GPIO as GPIO

GPIO.setmode(GPIO.BCM)
LED_PIN = 16
GPIO.setup(LED_PIN,GPIO.OUT)
Moter_pin = 12
GPIO.setup(Moter_pin,GPIO.OUT)
pwm = GPIO.PWM(Moter_pin, 1000)
pwm.start(0)

r = sr.Recognizer()
mic= sr.Microphone(device_index=2)
try:
    while(True):
        with mic as source:  # device_index 없이 기본 사용
            print("말씀하세요...")
            audio = r.listen(source,timeout=5,phrase_time_limit=10)
        try:
            text = r.recognize_google(audio, language='ko-KR')
            print("인식 결과:", text)
            if "무척 더워" in text:
                pwm.ChangeDutyCycle(90)
            elif "더워" in text:
                print("더워라고 인식")
                pwm.ChangeDutyCycle(20)
            elif "LED 꺼 줘" in text:
                print("LED 꺼 줘인식" )
                GPIO.output(LED_PIN,GPIO.LOW)
            elif "LED 켜 줘" in text:
                print("LED 켜 줘 인식")
                GPIO.output(LED_PIN,GPIO.HIGH)
            elif "그만" in text:
                break
        except sr.UnknownValueError:
            print("음성을 이해할 수 없습니다.")
        except sr.RequestError as e:
            print(f"API 요청 실패: {e}")
finally:
    print("GPIO 및 PWM을 정리하고 종료합니다.")
    pwm.stop()
    GPIO.cleanup()
