# Garage Door Controller
**Parul Jain**

Garage door open, close and status solution using smartthings hub, ZWave or ZigBee tilt or door/window sensor, and ESP8266 based WiFi relay.

## Parts List:

1. WeMos D1 Mini ESP8266 development board
2. Relay shield for WeMos D1 Mini
3. Android phone charger (5V micro 1 Amp USB wall power supply for the WeMos)
4. Smartthings hub
5. Zwave tilt sensor (Ecolink, Monoprice etc.). You can alternatively use a door/window sensor. However correct mounting of such a sensor is much harder than a tilt sensor

## Steps

1. Solder the Relay Shield using provided headers to the WeMos D1 Mini
2. Install and configure Arduino IDE on your Windows computer
3. Download the webRelay.ino sketch and edit it with the Arduino IDE to change the WiFi configuration
3. Upload the webRelay.ino sketch to WeMos D1 Mini using the Arduino IDE and USB connection
4. Install the WeMos D1 Mini with the Relay and phone charger on top of your garage door opener
5. Connect the relay NO contacts to the garage door opener in parallel to your wall mounted garage door switch
6. Create and publish the garageDoorControlDH.groovy Device Handler using the smartthings IDE
7. Using the same IDE create a device to use this handler. Set the Device Network ID as the IP Address:port that you configured in the Arduino sketch. For example 192.168.1.50:80
8. Create and publish the garageDoorApp.groovy smartapp using the smartthings IDE
9. Install the smartapp using the smartthings app on your phone
10. Enjoy!

