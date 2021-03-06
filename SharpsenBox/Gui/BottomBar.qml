import QtQuick 2.12

//import QtWinExtras 1.1
Rectangle {
    id: bottomBar
    color: "#292929"

    //max height
    property int len
    height: 0

    //states of instatation
    property int none: -1
    property int checking: 0
    property int downloading: 1
    property int installing: 2
    property int pause: 3
    property int error: 4
    property int stopped: 5
    property int completed: 6

    //stete
    property int state: _LoadingBar.state

    property color barColor: "green"

    //porgress property
    //progress settings
    property int updateInterval: 50
    property int infoInterval: 100
    property int speedInterval: 300

    property real progress: 0
    property real actual: 0
    property real total: 0
    property real speed: 0
    property real speedAvg: 0

    property string actualStr: "0"
    property string totalStr: "0"
    property string speedStr: "0"
    property string percenStr: "0"
    property string speedAvgStr: "0"

    property string statusInfo: "Initialization"

    Timer {
        id: info
        interval: infoInterval
        onTriggered: {
            speedAvgStr = (speedAvg / (1024 * 1024)).toFixed(2) // B/s -> MB/s
            speedStr = speed.toFixed(2)
            if (actual > 1000)
                actualStr = (actual / 1000).toFixed(2) + "GB"
            else
                actualStr = actual.toFixed(2) + "MB"
            if (total > 1000)
                totalStr = (total / 1000).toFixed(2) + "GB"
            else
                totalStr = total.toFixed(2) + "MB"
        }
        repeat: true
        running: state == downloading
    }
    function writeInfo() {
        bottomBar.statusInfo = "Downloading: " + actualStr + "/" + totalStr + " (Average Speed: "
                + speedAvgStr + "MB/s / Current Speed: " + speedStr + "MB/s )"
    }

    Timer {
        id: prog
        interval: updateInterval
        onTriggered: {
            //MB
            actual = _LoadingBar.actual
            total = _LoadingBar.total
            speedAvg = _LoadingBar.speed //B/s
            writeInfo()
        }
        repeat: true
        running: false
    }
    Timer {
        id: percentage
        interval: updateInterval
        onTriggered: {
            progress = _LoadingBar.progress
            percenStr = qsTr(progress.toFixed(1) + "%")
        }
        repeat: true
        running: false
    }
    Timer {
        id: speedTimer
        interval: speedInterval
        property real lastActual: actual
        onTriggered: {
            speed = 1000 * (actual - lastActual) / speedInterval
            lastActual = actual
            writeInfo()
        }
        repeat: true
        running: false
    }
    //bar states
    property int hidden: 0
    property int showed: 1
    property int minimalized: 2
    property int visibleCInput: _LoadingBar.visibleState
    onVisibleCInputChanged: {
        switch (visibleCInput) {
        case 0:
            hideBar.start()
            break
        case 1:
            visibleState = 1
            break
        case 2:
            visibleState = 2
            break
        }
    }

    property int visibleState: 0

    property bool hideLock: false
    onStateChanged: {
        switch(state){
        case checking:
            prog.stop()
            percentage.stop()
            speedTimer.stop()
            bottomBar.barColor = "white"
            loader.visible = true
            anim.start()
            bottomBar.progress = 100
            if (_LoadingBar.uninstall) {
                bottomBar.statusInfo = "Uninstalling"
            } else {
                bottomBar.statusInfo = "Searching for updates"
            }
            break;
        case downloading:
            prog.start()
            percentage.start()
            speedTimer.start()
            bottomBar.barColor = "orange"
            loader.visible = false
            anim.stop()
            break
        case installing:
            prog.stop()
            percentage.start()
            speedTimer.stop()
            bottomBar.statusInfo = "Installing"
            loader.visible = true
            anim.start()
            bottomBar.barColor = "orange"
            break;
        case pause:
            prog.stop()
            percentage.stop()
            speedTimer.stop()
            anim.stop()
            bottomBar.barColor = "yellow"
            break;
        case error:
            prog.stop()
            percentage.stop()
            speedTimer.stop()
            anim.stop()
            bottomBar.barColor = "red"
            break;
        case stopped:
            prog.stop()
            percentage.stop()
            speedTimer.stop()
            anim.stop()
            bottomBar.barColor = "red"
            bottomBar.statusInfo = "Update cancelled"
            break;
       case completed:
           percentage.stop()
           speedTimer.stop()
           progress = 100
           percenStr = "100%"
           anim.stop()
           loader.visible = false
           if (_LoadingBar.uninstall)
               bottomBar.statusInfo = "Uninstallation complete"
           else
               bottomBar.statusInfo = "Instalation complete"
           bottomBar.barColor = "green"
           prog.stop()
           break;
       case none:
           progress = 0
           actual = 0
           total = 0
           speed = 0
           speedAvg = 0

           actualStr = "0"
           totalStr = "0"
           speedStr = "0"
           percenStr = "0"
           speedAvgStr = "0"

           statusInfo: "Initialization"
           break;
       default:
           break;
        }
    }

    Timer {
        id: hideBar
        interval: 1000
        onTriggered: visibleState = hidden
        repeat: false
        running: false
    }
    //hiding showing animation
    Behavior on height {
        NumberAnimation {
            duration: 200
            easing.type: Easing.InOutQuad
            onRunningChanged: {
                if (!running
                        && bottomBar.visibleState === bottomBar.minimalized)
                    minimalizedBar.height = 10
            }
        }
    }

    onVisibleStateChanged: {
        if (visibleState === minimalized) {
            bottomBar.height = 0
        } else if (visibleState === showed) {
            if (minimalizedBar.height > 0)
                minimalizedBar.height = 0
            else
                bottomBar.height = bottomBar.len
        } else {
            bottomBar.height = 0
            minimalizedBar.height = 0
        }
    }

    //tmp timers for test purposes

    //Timer {
    //    interval: 5000
    //    onTriggered: {
    //        bottomBar.state = ((bottomBar.state + 1) % 7)
    //    }
    //    repeat: true
    //    running: true
    //}

    //minimalized bar
    Rectangle {
        id: minimalizedBar
        anchors {
            left: bottomBar.left
            right: bottomBar.right
            bottom: bottomBar.bottom
        }
        visible: true
        property int minLen: 6
        color: "grey"
        Rectangle {
            id: minimalizedPrBar
            anchors {
                top: parent.top
                left: parent.left
                bottom: parent.bottom
            }
            color: bottomBar.barColor
            width: (bottomBar.progress / 100) * parent.width
        }
        //normal mouse area
        MouseArea {
            anchors.fill: parent
            onClicked: if (!bottomBar.hideLock)
                           bottomBar.visibleState = bottomBar.showed
        }

        Behavior on height {
            NumberAnimation {
                duration: 100
                onRunningChanged: {
                    if (running === false
                            && bottomBar.visibleState === bottomBar.showed)
                        bottomBar.height = bottomBar.len
                }
            }
        }
    }

    // TaskbarButton {
    //     id: winPRogress
    //     progress {
    //         visible: true
    //         value: progress
    //         minimum: 0
    //         maximum: 100
    //         paused: state === pause
    //     }
    // }
    Item {
        id: controls
        property int size: 30
        anchors {
            top: parent.top
            left: parent.left
            bottom: parent.bottom
        }
        width: 100
        Item {
            id: goPause
            anchors {
                left: parent.left
                top: parent.top
                topMargin: 10
                leftMargin: 20
            }
            width: parent.size
            height: parent.size
            property bool play: _LoadingBar.pauseResume

            property real opac: 0.6
            Image {
                anchors.fill: parent
                source: "Icons/play-button.png"
                visible: parent.play
                opacity: parent.opac
                Behavior on opacity {
                    OpacityAnimator {
                        duration: 100
                    }
                }
            }
            Image {
                anchors.fill: parent
                source: "Icons/pause-button.png"
                visible: !parent.play
                opacity: parent.opac
                Behavior on opacity {
                    OpacityAnimator {
                        duration: 100
                    }
                }
            }
            MouseArea {
                hoverEnabled: true
                anchors.fill: parent
                onClicked: {
                    if (goPause.play)
                        _LoadingBar.resume()
                    else
                        _LoadingBar.pause()
                }
                onEntered: goPause.opac = 1
                onExited: goPause.opac = 0.6
                onPressed: goPause.opac = 0.6
                onReleased: goPause.opac = 1
            }
        }
        Item {
            id: stop
            anchors {
                left: goPause.right
                top: parent.top
                topMargin: 10
                leftMargin: 20
            }
            width: parent.size
            height: parent.size
            Image {
                id: stopImag
                source: "Icons/plain-square.png"
                anchors.fill: parent
                opacity: 0.6
                Behavior on opacity {
                    OpacityAnimator {
                        duration: 100
                    }
                }
            }
            MouseArea {
                hoverEnabled: true
                anchors.fill: parent
                onClicked: {
                    _LoadingBar.stop()
                }
                onEntered: stopImag.opacity = 1
                onExited: stopImag.opacity = 0.6
                onPressed: stopImag.opacity = 0.6
                onReleased: stopImag.opacity = 1
            }
        }
    }

    Rectangle {
        id: loadingBar
        anchors {
            left: controls.right
            leftMargin: 20
            rightMargin: 20
            right: parent.right
            top: parent.top
            topMargin: 10
        }
        height: 10
        color: "grey"
        border {
            color: "black"
            width: 0
        }
        smooth: true
        Rectangle {
            id: outter
            anchors {
                top: parent.top
                left: parent.left
                bottom: parent.bottom
                margins: parent.border.width
            }
            clip: true
            color: bottomBar.barColor
            property int max: parent.width - 2 * parent.border.width
            width: (bottomBar.progress / 100) * max
            radius: parent.radius
            BorderImage {
                id: loader
                source: "Icons/loader.png"
                x: parent.x
                y: parent.y
                height: parent.height
                width: parent.width * 2
                horizontalTileMode: BorderImage.Repeat
                opacity: 0.5
                visible: true

                NumberAnimation on x {
                    id: anim
                    from: outter.x - loader.sourceSize.width
                    to: outter.x
                    duration: 2000
                    loops: Animation.Infinite
                    running: true
                }
            }
        }
    }

    //status text
    Text {
        id: status
        anchors {
            left: loadingBar.left
            top: loadingBar.bottom
            topMargin: 9
        }
        font.family: latoFont.name
        font.pixelSize: 17
        color: "white"
        text: bottomBar.statusInfo
    }
    Text {
        id: percent
        anchors {
            right: loadingBar.right
            top: loadingBar.bottom
            topMargin: 9
        }
        font.family: latoFont.name
        font.pixelSize: 17
        color: "white"
        text: percenStr
        visible: bottomBar.state !== bottomBar.none && bottomBar.state !== bottomBar.checking
    }
    //minimalize mouse area
    MouseArea {
        anchors {
            top: parent.top
            bottom: parent.bottom
            right: parent.right
            left: loadingBar.left
        }

        onClicked: if (!bottomBar.hideLock)
                       bottomBar.visibleState = bottomBar.minimalized
    }
}
