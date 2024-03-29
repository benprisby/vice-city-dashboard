import QtQuick 2.15
import QtQuick.Layouts 1.12
import VCStyles 1.0

Tile {
    id: root

    property var device: null

    Text {
        id: deviceName

        anchors.top: parent.top
        anchors.topMargin: VCMargin.small
        anchors.left: parent.left
        anchors.leftMargin: anchors.topMargin
        font.pixelSize: VCFont.subHeader
        color: VCColor.white
        text: root.device ? root.device["name"] : ""
    }

    Text {
        id: productName

        anchors.top: deviceName.bottom
        anchors.topMargin: VCMargin.tiny
        anchors.left: deviceName.left
        font.pixelSize: VCFont.label
        font.capitalization: Font.Capitalize
        color: VCColor.white
        text: root.device ? root.device["productName"] : ""
    }

    Text {
        id: roomName

        anchors.verticalCenter: productName.verticalCenter
        anchors.left: productName.right
        font.pixelSize: productName.font.pixelSize
        color: productName.color
        text: (root.device && root.device["room"]) ? (qsTr(" in ") + root.device["room"]) : ""
    }

    VCSwitch {
        id: powerSwitch

        property bool on: enabled && root.device["isOn"]

        anchors.top: parent.top
        anchors.topMargin: VCMargin.tiny
        anchors.right: parent.right
        anchors.rightMargin: VCMargin.small
        enabled: root.device && (!notReachableWarning.visible)
        onOnChanged: checked = on
        onClicked: root.device.commandPower(checked)
    }

    Text {
        id: notReachableWarning

        anchors.verticalCenter: powerSwitch.verticalCenter
        anchors.right: powerSwitch.left
        anchors.rightMargin: VCMargin.medium
        font.pixelSize: VCFont.label
        font.capitalization: Font.AllUppercase
        font.bold: true
        color: VCColor.red
        text: qsTr("Not Reachable")
        visible: root.device && (!root.device["isReachable"])
    }

    GridLayout {
        id: controlsLayout

        anchors.top: productName.bottom
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.bottom: parent.bottom
        anchors.margins: VCMargin.small
        columns: 3
        columnSpacing: VCMargin.small
        rowSpacing: VCMargin.tiny

        Text {
            id: brightnessLabel

            Layout.fillHeight: true
            verticalAlignment: Text.AlignVCenter
            font.pixelSize: VCFont.body
            color: VCColor.white
            text: qsTr("Brightness")
            visible: brightnessSlider.visible
        }

        VCSlider {
            id: brightnessSlider

            property int currentValue: visible ? root.device["brightness"] : 0

            Layout.fillWidth: true
            Layout.preferredHeight: height
            Layout.alignment: Qt.AlignVCenter
            visible: root.device && (root.device["brightness"] !== undefined)
            enabled: root.device && !notReachableWarning.visible
            onCurrentValueChanged: {
                if (!pressed) {
                    value = currentValue;
                }
            }
            onPressedChanged: {
                // Ensure the final chosen value is commanded.
                if (!pressed) {
                    root.device.commandBrightness(Math.round(value));
                }
            }
            onMoved: {
                // Throttle requests to ensure the Bridge is not overloaded.
                if (!brightnessSliderHoldOffTimer.running) {
                    root.device.commandBrightness(Math.round(value));
                    brightnessSliderHoldOffTimer.start();
                }
            }

            Timer {
                id: brightnessSliderHoldOffTimer

                running: false
                repeat: false
                interval: 50
            }

        }

        Text {
            id: brightnessSliderValue

            Layout.fillHeight: true
            verticalAlignment: Text.AlignVCenter
            font.pixelSize: VCFont.label
            font.bold: true
            color: VCColor.white
            text: Math.round(brightnessSlider.value) + "%"
            visible: brightnessSlider.visible
        }

        Text {
            id: colorTemperatureLabel

            Layout.fillHeight: true
            verticalAlignment: Text.AlignVCenter
            font.pixelSize: VCFont.body
            color: VCColor.white
            text: qsTr("Temperature")
            visible: colorTemperatureSlider.visible
        }

        VCSlider {
            id: colorTemperatureSlider

            property int currentValue: visible ? root.device["colorTemperature"] : 0

            Layout.fillWidth: true
            Layout.preferredHeight: height
            Layout.alignment: Qt.AlignVCenter
            visible: root.device && (root.device["colorTemperature"] !== undefined) && !colorSlider.visible
            enabled: root.device && !notReachableWarning.visible
            from: visible ? root.device["minColorTemperature"] : 0
            to: visible ? root.device["maxColorTemperature"] : 0
            stepSize: 1
            onCurrentValueChanged: {
                if (!pressed) {
                    value = currentValue;
                }
            }
            onPressedChanged: {
                // Ensure the final chosen value is commanded.
                if (!pressed) {
                    root.device.commandColorTemperature(value);
                }
            }
            onMoved: {
                // Throttle requests to ensure the Bridge is not overloaded.
                if (!colorTemperatureSliderHoldOffTimer.running) {
                    root.device.commandColorTemperature(value);
                    colorTemperatureSliderHoldOffTimer.start();
                }
            }

            Timer {
                id: colorTemperatureSliderHoldOffTimer

                running: false
                repeat: false
                interval: 50
            }

        }

        Text {
            id: colorTemperatureSliderValue

            Layout.fillHeight: true
            verticalAlignment: Text.AlignVCenter
            font.pixelSize: VCFont.label
            font.bold: true
            color: VCColor.white
            text: colorTemperatureSlider.value + "K"
            visible: colorTemperatureSlider.visible
        }

        Text {
            id: colorLabel

            Layout.fillHeight: true
            verticalAlignment: Text.AlignVCenter
            font.pixelSize: VCFont.body
            color: VCColor.white
            text: qsTr("Color Hue")
            visible: colorSlider.visible
        }

        VCSlider {
            id: colorSlider

            property int currentValue: visible ? root.device["hue"] : 0

            Layout.fillWidth: true
            Layout.preferredHeight: height
            Layout.alignment: Qt.AlignVCenter
            visible: root.device && (root.device["color"] !== undefined)
            enabled: root.device && (!notReachableWarning.visible)
            from: 0
            to: 359
            stepSize: 1
            backgroundImage: "qrc:/images/color-scale.png"
            onCurrentValueChanged: {
                if (!pressed) {
                    value = currentValue;
                }
            }
            onPressedChanged: {
                // Ensure the final chosen value is commanded.
                if (!pressed) {
                    root.device.commandColor(value);
                }
            }
            onMoved: {
                // Throttle requests to ensure the Bridge is not overloaded.
                if (!colorSliderHoldOffTimer.running) {
                    root.device.commandColor(value);
                    colorSliderHoldOffTimer.start();
                }
            }

            Timer {
                id: colorSliderHoldOffTimer

                running: false
                repeat: false
                interval: 50
            }

        }

        Text {
            id: colorSliderValue

            Layout.fillHeight: true
            verticalAlignment: Text.AlignVCenter
            font.pixelSize: VCFont.label
            font.bold: true
            color: VCColor.white
            text: colorSlider.value
            visible: colorSlider.visible
        }

    }

}
