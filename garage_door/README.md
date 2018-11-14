# Garage Door Controller
**Parul Jain**

Garage door open, close and status solution using smartthings hub, ZWave or ZigBee tilt or door/window sensor, and ESP8266 based WiFi relay.

## Parts List

1. WeMos D1 Mini ESP8266 development board

```https://www.ebay.com/itm/HOT-D1-Mini-NodeMcu-4M-bytes-Lua-WIFI-Development-Board-ESP8266-by-WeMos/292657136823?epid=2232496538&hash=item4423b984b7:g:Ec0AAOSwfttbWrrr:rk:39:pf:0```

2. Relay shield for WeMos D1 Mini

```https://www.ebay.com/itm/Relay-Shield-for-Arduino-WeMos-D1-Mini-ESP8266-Development-Board-KZ/183492065390?hash=item2ab8fab86e:g:NVYAAOSwAfVbyH-G:rk:2:pf:0```

3. Android phone charger (5V micro 1 Amp USB wall power supply for the WeMos)

```https://www.ebay.com/itm/1-2-4-Port-5V-USB-AC-Wall-Charger-Home-Travel-AC-Fast-Charger-Adapter-US-EU-Plug/222981507785?hash=item33eabbcac9:m:mZ_hoVICeEMmPf9CEt18lGg:rk:10:pf:0```

4. Smartthings hub

```https://www.amazon.com/Samsung-SmartThings-Generation-GP-U999SJVLGDA-Automation/dp/B07FJGGWJL/ref=sr_1_3?ie=UTF8&qid=1542158658&sr=8-3&keywords=smartthings+hub+v3&dpID=219HZLd2RZL&preST=_SY300_QL70_&dpSrc=srch```

5. Zwave tilt sensor (Ecolink, Monoprice etc.). You can alternatively use a door/window sensor. However correct mounting of such a sensor is much harder than a tilt sensor

```https://www.amazon.com/Z-Wave-Plated-Reliability-Garage-TILT-ZWAVE2-5-ECO/dp/B01MRZB0NT/ref=sr_1_3?ie=UTF8&qid=1542158628&sr=8-3&keywords=zwave+tilt+sensor&dpID=31XYD4TV4EL&preST=_SX300_QL70_&dpSrc=srch```

![alt text](https://github.com/paruljain/smartthings/blob/master/garage_door/IMG_20181113_153058631.jpg "WeMos installed on the opener")

## Steps

1. Solder the Relay Shield using provided headers to the WeMos D1 Mini
2. Install and configure Arduino IDE on your Windows computer
3. Download the webRelay.ino sketch and edit it with the Arduino IDE to change the WiFi configuration. Make sure that the version of your esp8266 board in the Arduino IDE is 2.4.0 or higher
3. Upload the webRelay.ino sketch to WeMos D1 Mini using the Arduino IDE and USB connection
4. Install the WeMos D1 Mini with the Relay and phone charger on top of your garage door opener
5. Connect the relay NO contacts to the garage door opener in parallel to your wall mounted garage door switch
6. Install and pair (include) to the smartthings hub the Zwave tilt sensor on your garage door
7. Create and publish the garageDoorControlDH.groovy Device Handler using the smartthings IDE
8. Using the same IDE create a device to use this handler. Set the Device Network ID as the IP Address:port that you configured in the Arduino sketch. For example 192.168.1.50:80
9. Create and publish the garageDoorApp.groovy smartapp using the smartthings IDE
10. Install the smartapp using the smartthings app on your phone
11. Enjoy!

