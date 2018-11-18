import machine, config

leds = machine.Neopixel(machine.Pin(config.pin_led), config.count_leds, machine.Neopixel.TYPE_RGB)
leds.timings(((900,350), (350, 900), 60000))

import time
def test():
    for pos in range(1, 500000):
        leds.set(1, 0xFF0000)
        leds.show()
        time.sleep_ms(50)


  ##################################################################################################
 #                                                                                                  #
#  Группа функций устанавливающая цвет светодиодов и получающая в качестве параметра задающего цвет  #
#  24 битное число, в котором каждая группа из 8 бит задает значение одного из компонентов RGB       #
 #                                                                                                  #
  ##################################################################################################

def setAllRGB(color):
    """Устанавливает цвет всех светодиодов в девайсе"""
    leds.set(1, color, 0, config.count_leds)


def setHoursRGB(color):
    """Устанавливет цвет диодов внутреннего кольца"""
    for led in config.getLedNums4Hours():
        leds.set(led, color, 0, 1, False)
    leds.show()


def setMinutesRGB(color):
    """Устанавливет цвет диодов внешнего кольца"""
    for led in config.getLedNums4Minutes():
        leds.set(led, color, 0, 1, False)
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

