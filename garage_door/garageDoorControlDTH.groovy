metadata {
    definition(name: "Garage Door Control", namespace: "paruljain", author: "Parul Jain") {
        capability "Door Control"
        command "setOpen"
        command "setClosed"
    }
    tiles(scale: 2) {
        standardTile("door", "device.door", width: 2, height: 2, canChangeIcon: true, decoration: "flat") {
            state "closed", label: "CLOSED", icon: "st.Transportation.transportation14", backgroundColor: "#00a0dc", action: "doorControl.open", nextState: "opening"
			state "open", label: "OPEN", icon: "st.Transportation.transportation13", backgroundColor: "#dc0032", action: "doorControl.close", nextState: "closing"
            state "opening", label: "Opening", icon: "st.Transportation.transportation14", backgroundColor: "#dc0032"
            state "closing", label: "Closing", icon: "st.Transportation.transportation13", backgroundColor: "#dc0032"
        }
        main("door")
    }
}

def open() {
	def hubAction = new physicalgraph.device.HubAction (
        method: "GET",
        path: "/relay",
        headers: [
            HOST: device.deviceNetworkId
        ]
    )
    return hubAction
}

def close() {
	open()
}

def parse(String description) {
}

def setOpen() {
	sendEvent(name:"door", value:"open")
}

def setClosed() {
	sendEvent(name:"door", valued:"closed")
}
