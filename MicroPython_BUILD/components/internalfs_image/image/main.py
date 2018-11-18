from machine import reset # Импортируем для удобства вызова функции reset() из консоли repl
import leds
try:
    # Все что отрабатывает до подъема сети, обернем в блок отлова ошибок
    # Чтобы в любом случае, сеть у нас поднялась
    import led_effects, leds
    # Инициализируем объект эффекта - огонек бегущий по внешнему кольцу. С яркостью в 20%
    effect = led_effects.EffectRunningLight(0, 0.2)
    led_effects.startEffect(effect)
except:
    pass

import wifi
import time

# Остановим эффект. Выключим все светодиоды и включим центральное кольцо, на время поднятия веб-сервера
led_effects.stopEffect()
leds.setHoursHSB(40, 1, 0.2)

#import webapp

# Моргнем всеми светодиодами, зеленым цветом
leds.setAllHSB(120, 1, 1)
time.sleep_ms(500)
leds.setAllRGB(0)

import machine
i2c = machine.I2C(0, sda=4, scl=5)
