Kankun WiFi outlets were a cheap option using the TPLink chipset. People hacked these outlets to allow SSH access to the Linux like OS
running on the device. The device had an inbuilt webserver with a CGI interface. It was then trivial to create a REST API to turn the outlet on and off.

Here are details on how to configure the device:
http://www.anites.com/2015/01/hacking-kankun-smart-wifi-plug.html

Once you have got the REST API working, you can use this device handler to control the outlet from smartthings. You must create a
new device using the IDE. For device address you should use the IP address of the outlet with the port number 80. Example: 192.168.1.20:80.
Thereafter use the Classic app to control the outlet.

To do:

The handler sets state (sendEvent) on or off without waiting for the REST API to respond. This means that the app will show the new
state even if there was an error. Ideally the state should be set within parse() which is called when response is recieved from the REST API. For some reason parse() is not being called with this setup. Perhaps the response from the CGI script is not in proper HTTP response form? Needs investigation.
