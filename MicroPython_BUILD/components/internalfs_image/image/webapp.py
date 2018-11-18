from microWebSrv import MicroWebSrv
from machine import reset, Timer
import wifi
import json
import leds 
import re

# ----------------------------------------------------------------------------


def _httpHandlerSetNetwork(httpClient, httpResponse) :
    formData  = httpClient.ReadRequestPostedFormData()
    ssid = formData["ssid"]
    passphrase = formData["passphrase"]
    # TODO: Проверить длину и валидность имени и пароля в соответствии со стандартами
    wifi.setWiFi(ssid, passphrase)


def _httpHandlerReset(httpClient, httpResponse) :
    
    #Get client request total path   
    print("Get client request total path", httpClient.GetRequestTotalPath())

    #reset()


def web_callback(webSocket, data):
    webSocket.SendText(json.dumps(data))


def _acceptWebSocketCallback(webSocket, httpClient) :
    print("WS ACCEPT")
    webSocket.RecvTextCallback   = _recvTextCallback
    webSocket.RecvBinaryCallback = _recvBinaryCallback
    webSocket.ClosedCallback     = _closedCallback
    #ctrl.RegWebCallback(web_callback, webSocket)


def _recvTextCallback(webSocket, msg) :
    obj = json.loads(msg)
    cmd = obj.get("cmd")
    if cmd!=None:
        if cmd=="setWiFi":
            wifi.setWiFi(obj.get("ssid"), obj.get("passphrase"))

        if cmd=="setColorHSB":
            leds.setAllHSB(obj.get("H"), obj.get("S"), obj.get("B"),)
            print(obj)


def _recvBinaryCallback(webSocket, data) :
    print("WS RECV DATA : %s" % data)


def _closedCallback(webSocket) :
    print("WS CLOSED")
    #ctrl.UnregWebCallback()

# ----------------------------------------------------------------------------
routeHandlers = [
	( "/setnetwork","POST",	_httpHandlerSetNetwork),
	( "/(.)*","GET",	_httpHandlerReset)
]

srv = MicroWebSrv(routeHandlers=routeHandlers)
srv.MaxWebSocketRecvLen     = 256
srv.WebSocketThreaded	    = True 
srv.AcceptWebSocketCallback = _acceptWebSocketCallback
srv.Start(threaded=True)
# ----------------------------------------------------------------------------
