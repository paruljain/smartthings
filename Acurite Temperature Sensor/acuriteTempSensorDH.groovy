metadata {
    definition(name: "Acurite 433MHz Temperature Sensor", namespace: "paruljain", author: "Parul Jain") {
        capability "Temperature Measurement"
        capability "Battery"
        capability "Relative Humidity Measurement"
    }
    tiles(scale: 2) {
        valueTile("temperature", "device.temperature", width: 2, height: 2, decoration: "flat") {
            state "temperature", label: '${currentValue}°',  icon: "st.alarm.temperature.normal", backgroundColor: "#1ddc00"
        }
        valueTile("humidity", "device.humidity", width: 2, height: 2, decoration: "flat") {
            state "humidity", label: '${currentValue}%', backgroundColor: "#00dc78"
        }
        standardTile("battery", "batteryState", width: 2, height: 2, decoration: "flat") {
            state "ok", label: "OK", icon:"st.arlo.sensor_battery_4"
            state "low", label: "Low", icon:"st.arlo.sensor_battery_0"
        }
        main("temperature")
        details("temperature", "humidity", "battery")
    }
}

def parse(description) {
	def msg = parseLanMessage(description)
    log.debug(msg.xml.temperature.text())
    //Change the following 2630 to the serial number of your sensor
    if (msg.xml.serial != "2630") return
    def batt = 90
    def battState = "ok"
    if (msg.xml.lowbatt.text() == "1") {
    	batt = 1
        battState = "low"
    }
    return [
        createEvent(name: "temperature", value: msg.xml.temperature.text().toFloat().round(), unit: "F"),
        createEvent(name: "battery", value: batt),
        createEvent(name: "humidity", value: msg.xml.humidity.text().toInteger()),
        createEvent(name: "batteryState", value: battState)
    ]
}

private getCallBackAddress() {
    return device.hub.getDataValue("localIP") + ":" + device.hub.getDataValue("localSrvPortTCP")
}

//Following is mostly redundant code
private subscribeAction(path, callbackPath="") {
    log.debug("Calling subscribe")
    def address = getCallBackAddress()

    def result = new physicalgraph.device.HubAction(
        method: "SUBSCRIBE",
        path: path,
        headers: [
            HOST: "192.168.1.22:80",
            CALLBACK: "<http://${address}/notify$callbackPath>",
            NT: "upnp:event",
            TIMEOUT: "Second-28800"
        ]
    )

    return result
}

private Integer convertHexToInt(hex) {
    return Integer.parseInt(hex,16)
}

private String convertHexToIP(hex) {
    return [convertHexToInt(hex[0..1]),convertHexToInt(hex[2..3]),convertHexToInt(hex[4..5]),convertHexToInt(hex[6..7])].join(".")
}

private getDeviceIPAddress() {
    def parts = device.deviceNetworkId.split(":")
    if (parts.length == 2) {
        return convertHexToIP(parts[0]) + ":" + convertHexToInt(parts[1])
    } else {
        log.warn "Can't figure out ip and port for device: ${device.id}"
    }
}
