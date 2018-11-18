import network, machine
from time import sleep_ms

"""
Здесь мы устанавливаем соединение с существующей точкой WiFi. 
Если это не удается в течение некоторого времени, создаем свою точку WiFi.
После чего запускаем сервера telnet и ftp.
Никакой заботы о восстановлении соединения в случае его потери не проявляем. Система автоматически восстанавливает соединение.
"""



 #####################################
#                                     #
#  Константы и глобальные переменные  #
#                                     #
 #####################################
"""
Ключ, для хранения в энергонезависимой памяти, названия существующей (к которой будем коннектиться) сети wifi
"""
KEY_STATION_SSID = "STA_SSID"

"""
Ключ, для хранения в энергонезависимой памяти, пароля для подключения к существующей сети wifi
"""
KEY_STATION_PASSWORD = "STA_PWD"



 #######################
#                       #
#  Определение функций  #
#                       #
 #######################

def create_access_point():
    """
    Функция создает собственную точку вайфай.
    ssid и пароль жестко закодированы в теле функции
    """
    station = network.WLAN(network.AP_IF)
    station.active(True)
    station.config(essid='indicator', authmode=3, password="indicator.fun")
    print("Access Point created")
    print(station.ifconfig())


def connect_to_exist_access_point(station_ssid, station_password):
    """
    Функция получает в качестве параметров название существующей сети wifi, пароль к ней, и пытается установить соединение.
    Если соединение установить не удается, передается управление функции создающей собственную точку доступа.
    """
    station = network.WLAN(network.STA_IF)
    station.active(True)
    station.connect(station_ssid, station_password)

    # Подождем некоторое время (указанное в милисекундах), пока производится попытка установления связи 
    delay=0      
    while delay < 50000 and station.isconnected() == False:
        sleep_ms(50)
        delay+=50

    # если связь установить не удалось, деактивируем объект station и перейдем к созданию собственной точки доступа
    if station.isconnected() == False:
        station.active(False)
        create_access_point()
        return

    print("Connection to exist access point successful")
    print(station.ifconfig())
    

def start_mDNS():
    """
    Функция стартует сервис mdns, что позволяет искать девайс в сети по имени (не зная изначально его IP адрес)
    """
    # TODO: Выполнить проверку на коллизию имен инстансов
    try:
        board = "BigCircle-32"
        instance = "indicator"
        mdns = network.mDNS()
        mdns.start(instance, "indicator.fun device")
        mdns.addService('_ftp', '_tcp', 21, instance + "-FTP", {"board": board, "service": "FTP File transfer", "passive": "True"})
        mdns.addService('_telnet', '_tcp', 23, instance + "-TELNET", {"board": board, "service": "Telnet REPL"})
        mdns.addService('_http', '_tcp', 80, instance + "-WEB", {"board": board, "service": "Web server"})
    except:
        print("mDNS not started")


def setWiFi(ssid, passphrase):
    """
    Установка новых имени сети (к которой девайс коннектится при старте) и пароля
    """
    machine.nvs_setstr("STA_SSID", ssid)
    machine.nvs_setstr("STA_PWD", passphrase)



 #######################################
#                                       # 
#  Код выполняемый при загрузке модуля  #
#                                       #
 #######################################

# Пытаемся извлечь из энергонезависимой памяти (по известным ключам), название сети и пароль доступа к ней 
station_ssid = machine.nvs_getstr(KEY_STATION_SSID)
station_password = machine.nvs_getstr(KEY_STATION_PASSWORD)

if station_ssid != None and station_password != None:
    # Если имя сети или пароль к ней известны (были ранее сохранены), пытаемся подключиться к существующей сети
    connect_to_exist_access_point(station_ssid, station_password)
else:
    # Иначе создаем собственную точку доступа wifi
    create_access_point()

network.telnet.start()
network.ftp.start()
    
# Стартуем сервис mdns. 
start_mDNS()
