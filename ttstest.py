import pyttsx3
import RPi.GPIO as GPIO
import time

# --- GPIO 핀 설정 (BCM 넘버링) ---
trigger = 4  # Trig 핀
echo = 17    # Echo 핀 (⚠️ 3.3V 전압 분배기 필수!)
SPEED_OF_SOUND_HALF = 17150  # 34300 cm/s / 2

def getDistance():
    # 1. Trigger 펄스 발사
    # 센서 안정화를 위해 Low 상태로 초기화 (이미 main에서 했지만 안전을 위해 다시 실행)
    GPIO.output(trigger, GPIO.LOW)
    time.sleep(0.000002)

    # 10us(마이크로초) High 펄스 시작
    GPIO.output(trigger, GPIO.HIGH)
    time.sleep(0.00001)  # 10us = 0.00001s
    GPIO.output(trigger, GPIO.LOW)
    
    # 2. Echo 펄스 시작 시간 (High) 측정
    pulse_start = time.time()
    
    # Low 상태 대기 (시작 시간 기록을 위해)
    # 0.5초 타임아웃을 설정하여 무한 루프 방지
    timeout = pulse_start + 0.5 
    while GPIO.input(echo) == GPIO.LOW and pulse_start < timeout:
        pulse_start = time.time()

    # 3. Echo 펄스 종료 시간 (Low) 측정
    pulse_end = pulse_start  # 초기값 설정
    timeout = pulse_end + 0.5 
    while GPIO.input(echo) == GPIO.HIGH and pulse_end < timeout:
        pulse_end = time.time()

    # 4. 거리 계산
    pulse_duration = pulse_end - pulse_start
    distance = pulse_duration * SPEED_OF_SOUND_HALF  # cm 단위
    
    return distance

def main():
    engine = pyttsx3.init()
    
    # --- GPIO 초기 설정 ---
    try:
        GPIO.setmode(GPIO.BCM)
        GPIO.setup(trigger, GPIO.OUT)
        # 📌 수정된 부분: Echo 핀 설정 (IN)
        GPIO.setup(echo, GPIO.IN) 
        
        GPIO.output(trigger, GPIO.LOW)
        time.sleep(0.5) # 센서 안정화 시간
        
        while True:
            fDistance = getDistance()
            # 거리 출력 (디버깅 용)
            print(f"Distance: {fDistance:.2f} cm")
            
            # 📌 논리 수정: 0.1cm 미만 오차값 제외 및 100cm 미만 감지 시 TTS
            if fDistance < 100 and fDistance > 0.1: 
                engine.say("안녕하세요, 라즈베리파이입니다.")
                engine.runAndWait()
                # 연속 TTS 방지를 위해 잠시 대기
                time.sleep(2) 
            else:
                # 감지되지 않았을 때도 루프 속도 조절을 위해 대기
                time.sleep(0.1) 
                
    except KeyboardInterrupt:
        print("프로그램 종료")
    # 📌 필수: GPIO cleanup은 finally에서 실행
    finally:
        GPIO.cleanup()
        
if __name__ == '__main__':
    main()
