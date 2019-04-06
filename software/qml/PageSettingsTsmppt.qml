import QtQuick 1.1
import com.victron.velib 1.0
import "utils.js" as Utils

MbPage {
	title: qsTr("TriStar MPPT 60 Solar Charger")
	property string settings: "com.victronenergy.settings"

	model: VisualItemModel {

		MbEditBox {
			id: ipaddress
			description: qsTr("Hostname/IP Address")
			item.bind: Utils.path(settings, "/Settings/TristarMPPT/IPAddress")
			item.value: hostItem.value

			VBusItem {
				id: hostItem
				bind: Utils.path(settings, "/Settings/TristarMPPT/IPAddress")
			}
		}
		
		MbEditBox {
			id: portnumber
			description: qsTr("Modbus IP Port")
			matchString: "0123456789"
			item.value: portNumberItem.valid ? Utils.pad(portNumberItem.value, 5) : '--'
			onEditDone: portNumberItem.setValue(parseInt(text, 10));

			VBusItem {
				id: portNumberItem
				bind: Utils.path(settings, "/Settings/TristarMPPT/PortNumber")
			}
		}
	}
}
