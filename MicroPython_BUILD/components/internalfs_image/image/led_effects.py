import leds, config, machine


class EffectRunningLight:
    """
    Класс реализует эффект - бегущий по внешнему кольцу огонек. Цвет и яркость огонька задаются параметрами
    """
    def __init__(self, hue, brightness):
        """
        Выключаем все светодиоды, включаем один светодиод во внешнем кольце с указанным цветом и яркостью
        """
        leds.setAllRGB(0)
    
        self.ar_mins = config.getLedNums4Minutes()
        self.len_mins = len(self.ar_mins)

        leds.leds.setHSB(self.ar_mins[0], hue, 1, brightness)


    def __deinit__(self):
        # Выключим все светодиоды
        leds.setAllRGB(0)


    def tick(self, arg):
        """
        Данная функция вызывается пеориодически по тику таймера.
        Задача функции - создать визуальный эффект, управляя свечением светодиодов
        """
        for i in range(0, self.len_mins):
            if i==0:
                first_rgb = leds.leds.get(self.ar_mins[i])

            if i==self.len_mins-1:
                leds.leds.set(self.ar_mins[i], first_rgb, 1, False)
            else:
                leds.leds.set(self.ar_mins[i], leds.leds.get(self.ar_mins[i+1]), 1, False)
        leds.leds.show()
    


# Создадим и запустим таймер, периодически вызывающий функцию калбэк (реализуемую объектом эффекта).
# Прямо сейчас мы эту функцию не указываем. Она будет дана объекту таймера во время запуска спецэффекта
m_timer = machine.Timer(1)
m_timer.init(mode=machine.Timer.PERIODIC, period=40)

current_effect = None


def stopEffect():
    """
    Останавливает работу эффекта, путем удаления калбэка таймера (фукция обработки тиков, объекта эффекта, не будет вызываться)
    """
    m_timer.callback(None)
    global current_effect
    # Даем возможность объекту эффекта, привести свои дела в порядок перед смертью
    current_effect.__deinit__()
    del current_effect
    current_effect = None


def startEffect(effect_obj):
    """
    Функция получает объект спецэффекта, сохраняет его как глобальную переменную (для того, чтобы позже иметь возможность его найти и
    деинициализировать. В данном случае мы заморачиваемся с созданием объекта менеджера эффектов а оставляем данные функции как глобальные 
    функции модуля)
    """
    if effect_obj == None:
        print("Null effect object")
        return

    global current_effect
    if current_effect != None:
        # Если в данный момент уже существует активный объект эффекта, грохнем его
        stopEffect()

    # Сохраним переданный в качестве параметра объект эфеекта
    # и укажем его функцию обработки временных тиков в качестве калбэка таймера
    current_effect = effect_obj
    m_timer.callback(current_effect.tick)
    
