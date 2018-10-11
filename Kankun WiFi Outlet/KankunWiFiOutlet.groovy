metadata {
    definition(name: "Kankun WiFi Outlet", namespace: "paruljain", author: "Parul Jain") {
        capability "Switch"
    }
    tiles(scale: 2) {
        // standard tile with actions named
        standardTile("switch", "device.switch", width: 2, height: 2, canChangeIcon: true) {
            state "off", label: "off", icon: "st.switches.switch.off", backgroundColor: "#ffffff", action: "switch.on"
		    state "on", label: "on", icon: "st.switches.switch.on", backgroundColor: "#00a0dc", action: "switch.off"
        }
        // the "switch" tile will appear in the Things view
        main("switch")
    }
}

def on() {
    def hubAction = new physicalgraph.device.HubAction (
        method: "GET",
        path: "/cgi-bin/relay.cgi?on",
        headers: [
            HOST: device.deviceNetworkId
        ]
    )
    sendEvent(name: 'switch', value: 'on')
    return hubAction
}

def off() {
    def hubAction = new physicalgraph.device.HubAction (
        method: "GET",
        path: "/cgi-bin/relay.cgi?off",
        headers: [
            HOST: device.deviceNetworkId
        ]
    )
    sendEvent(name: 'switch', value: 'off')
    return hubAction
}
