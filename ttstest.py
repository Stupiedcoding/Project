from gtts import gTTS
import os
import sys
print(sys.getdefaultencoding())
text=“라즈베리파이로 합성된 한글 음성입니다.”
tts=gTTS(text=text, lang=‘ko’)
tts.save(“output.mp3”)
os.system(“mpg321 output.mp3”)
