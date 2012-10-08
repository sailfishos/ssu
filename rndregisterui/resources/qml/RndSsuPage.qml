import QtQuick 1.1
import com.nokia.meego 1.0

Page {
    id: rndSsuPage

    Connections {
        target: ssu
        onDone: {
            if (ssu.error()){
                lblRegistrationStatus.text = qsTr("Device registration failed")
                lblRegistrationStatus.text += ssu.lastError()
                registerButton.enabled = true
            } else {
                lblRegistrationStatus.text = qsTr("Device is registered")
            }
            refreshCredentialsButton.enabled = true
        }
        onRegistrationStatusChanged: {
            if (ssu.isRegistered()){
                columnRegister.visible = false
                columnRegistered.visible = true
            } else {
                columnRegister.visible = true
                columnRegistered.visible = false
            }
        }
        onCredentialsChanged: {
            lblCredentialsDate.text = ssu.lastCredentialsUpdate()
        }
        onFlavourChanged: {
            var flavour = ssu.flavour();
            if (flavour == "devel"){
                btnFlvrDevel.checked = true
            } else if (flavour == "testing"){
                btnFlvrTesting.checked = true
            } else if (flavour == "release"){
                btnFlvrRelease.checked = true
            }
        }
    }

    TabGroup{
        id: tabgroup
        currentTab: tab1
        Page {
            id: tab1
            Flickable {
                id: flickT1
                width: parent.width
                height: parent.height
                flickableDirection: Flickable.VerticalFlick
                contentWidth: columnT1.paintedWidth
                contentHeight: columnT1.height

                Column {
                    id: columnT1
                    width: parent.width
                    spacing: 20;

                    Label {
                        id: lblRegistrationHeader
                        width: parent.width
                        text: qsTr("Device registration")
                    }

                    Column {
                        id: columnRegister
                        width: parent.width
                        visible: !ssu.isRegistered()
                        spacing: 20

                        Label {
                            id: lblJollaId
                            text: qsTr("Jolla Username:")
                            width: parent.width
                        }
                        TextField {
                            id: inputJollaId
                            text: ""
                            width: parent.width
                        }
                        Label {
                            id: lblJollaPasswd
                            text: "Jolla password:"
                            width: parent.width
                        }
                        TextField {
                            id: inputJollaPasswd
                            echoMode: TextInput.Password
                            text: ""
                            width: parent.width
                        }

                        Button {
                            id: registerButton

                            function registerButtonClicked(){
                                registerButton.enabled = false
                                ssu.sendRegistration(inputJollaId.text, inputJollaPasswd.text)
                            }
                            onClicked: registerButtonClicked()
                            text: qsTr("Register Device")
                            width: parent.width
                        }
                        Label {
                            id: lblRegistrationStatus
                            width: parent.width
                        }
                    }
                    Column {
                        id: columnRegistered
                        width: parent.width
                        visible: ssu.isRegistered()
                        spacing: 20;

                        Label {
                            id: lblDeviceRegistered
                            text: qsTr("This device is registered")
                            width: parent.width
                        }
                        Button {
                            id: unregisterDeviceButton
                            onClicked: ssu.unregister()
                            text: qsTr("Unregister device")
                            width: parent.width
                        }
                        Button {
                            id: refreshCredentialsButton

                            function refreshCredentialsButtonClicked(){
                                refreshCredentialsButton.enabled = false
                                ssu.updateCredentials(true);
                            }
                            onClicked: refreshCredentialsButtonClicked()
                            text: qsTr("Update credentials")
                            width: parent.width
                        }
                        Label { text: qsTr("Last credentials update:") }
                        Label {
                            id: lblCredentialsDate
                            text: ssu.lastCredentialsUpdate()
                            width: parent.width
                        }
                    }
                }
            }
        }
        Page {
            id: tab2
            Flickable {
                id: flickT2
                width: parent.width
                height: parent.height
                flickableDirection: Flickable.VerticalFlick
                contentWidth: columnT2.paintedWidth
                contentHeight: columnT2.height

                Column {
                    id: columnT2
                    width: parent.width
                    spacing: 20

                }
            }
        }
        Page {
            id: tab3
            Flickable {
                id: flickT3
                width: parent.width
                height: parent.height
                flickableDirection: Flickable.VerticalFlick
                contentWidth: columnT3.paintedWidth
                contentHeight: columnT3.height

                Column {
                    id: columnT3
                    width: parent.width
                    spacing: 20

                    Label { id: lblT3_1; text: "Settings" }
                    Label { id: lblFlvr; text: "Flavour" }
                    ButtonColumn {
                        width: parent.width

                        Button {
                            id: btnFlvrDevel
                            text: "devel"
                            onClicked: ssu.setFlavour("devel")
                            checked: ssu.flavour() == "devel"
                        }
                        Button {
                            id: btnFlvrTesting
                            text: "testing"
                            onClicked: ssu.setFlavour("testing")
                            checked: ssu.flavour() == "testing"
                        }
                        Button {
                            id: btnFlvrRelease
                            text: "release"
                            onClicked: ssu.setFlavour("release")
                            checked: ssu.flavour() == "release"
                        }
                    }

                    Label { id: lblRelease; text: "Release" }
                    ButtonColumn {
                        width: parent.width

                        Button { text: "latest"; onClicked: ssu.setRelease("latest", true) }
                        Button { text: "next"; onClicked: ssu.setRelease("next", true) }
                    }
                }
            }
        }
    }

    tools:ToolBarLayout {
        id: commonTools
        visible: true
        ButtonRow {
            style: TabButtonStyle { }
            TabButton {
                text: qsTr("Registration")
                tab: tab1
            }
            TabButton {
                text: "B"
                tab: tab2
                visible: false
            }
            TabButton {
                text: qsTr("Settings")
                tab: tab3
            }
        }
        ToolIcon {
            platformIconId: "toolbar-view-menu"
            anchors.right: (parent === undefined) ? undefined : parent.right
            onClicked: (myMenu.status == DialogStatus.Closed) ? myMenu.open() : myMenu.close()
        }
    }
    Menu {
        id: myMenu
        visualParent: pageStack
        MenuLayout {
            MenuItem { text: qsTr("About"); }
            MenuItem { text: qsTr("Exit application"); onClicked: root.quit() }
        }
    }
}
