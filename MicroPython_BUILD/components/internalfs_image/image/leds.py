import machine, config, indicator

leds = indicator.IFunNeopixel()

adc=machine.ADC(32)
adc.atten(adc.ATTN_6DB)

import time
def test(num_led):
    period = 1800
    for t in range(1, 24, 1):
        print(t)
        #leds.timings(((period-t,t), (t, period-t), 50000))

        for pos in range(1, 20):
            leds.set(num_led, 0x0f8000)
            leds.show()
            time.sleep_ms(100)

def test_mic():
    for t in range(1, 100000, 1):
        v=adc.read()
        v=v/1000
        if(v>1.7):
            setAllRGB(0x000f00)
            print(v, end=' ')
            time.sleep_ms(100)
        else:
            setAllRGB(0x000000)

        time.sleep_ms(10)
            


  ##################################################################################################
 #                                                                                                  #
#  Группа функций устанавливающая цвет светодиодов и получающая в качестве параметра задающего цвет  #
#  24 битное число, в котором каждая группа из 8 бит задает значение одного из компонентов RGB       #
 #                                                                                                  #
  ##################################################################################################

def setAllRGB(color):
    """Устанавливает цвет всех светодиодов в девайсе"""
    leds.set(1, color, config.count_leds)


def setHoursRGB(color):
    """Устанавливет цвет диодов внутреннего кольца"""
    for led in config.getLedNums4Hours():
        leds.set(led, color,  1, False)
    leds.show()


def setMinutesRGB(color):
    """Устанавливет цвет диодов внешнего кольца"""
    for led in config.getLedNums4Minutes():
        leds.set(led, color, 1, False)
    leds.show()


def setCenterRGB(color):
    """Устанавливет цвет центрального светодиода"""
    leds.set(config.getLedNums4Center()[0], color)


  ######################################################################################################
 #                                                                                                      #
#  Группа функций устанавливающая цвет светодиодов и получающая в качестве параметров определяющих цвет  #
#  тон, насыщенность, яркость                                                                            #
 #                                                                                                      #
  ######################################################################################################

def setAllHSB(hue, saturation, brightness):
    """Устанавливает цвет всех светодиодов в девайсе
    принимает в качестве параметров определяющих цвет тон, насыщенность и яркость
    """
    leds.setHSB(1, hue, saturation, brightness, config.count_leds)


def setHoursHSB(hue, saturation, brightness):
    """Устанавливет цвет диодов внутреннего кольца
    принимает в качестве параметров определяющих цвет тон, насыщенность и яркость
    """
    for led in config.getLedNums4Hours():
        leds.setHSB(led, hue, saturation, brightness, 1, False)
    leds.show()


def setMinutesHSB(hue, saturation, brightness):
    """Устанавливет цвет диодов внешнего кольца
    принимает в качестве параметров определяющих цвет тон, насыщенность и яркость
    """
    for led in config.getLedNums4Minutes():
        leds.setHSB(led, hue, saturation, brightness, 1, False)
    leds.show()


def setCenterHSB(hue, saturation, brightness):
    """Устанавливет цвет центрального светодиода
    принимает в качестве параметров определяющих цвет тон, насыщенность и яркость
    """
    leds.setHSB(config.getLedNums4Center()[0], hue, saturation, brightness)

