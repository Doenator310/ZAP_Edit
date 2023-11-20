import QtQuick
import QtQuick.Window
import QtQuick.Controls
import QtQuick.Layouts
import QtQuick.Controls.Material
import QtMultimedia
import AudioEditorItem

Rectangle {
    id: localRoot
    color: Material.backgroundColor
    property AudioEditorItem audioItemRef: null

    MouseArea{
        anchors.fill: parent
        acceptedButtons: Qt.RightButton | Qt.LeftButton
    }

    DropArea {
        anchors.fill: parent
        onDropped: (drop) => {
            console.log(drop)
        }
    }


    Button{
        id: applyBtn

        anchors{
            top: parent.top
            right: closeBtn.left
            rightMargin: 10
        }
        width: 50
        height: 50
        flat: true

        enabled: false
        icon.source:  "qrc:/images/apply_icon.png"
        icon.width: 40
        icon.height: 40
        icon.color: enabled ? "green" : "gray"
        onClicked: localRoot.applyChanged();
    }

    Button{
        id: closeBtn

        anchors{
            top: parent.top
            right: parent.right
        }
        width: 50
        height: 50
        flat: true

        icon.source:  "qrc:/images/x_icon.png"
        icon.width: 40
        icon.height: 40


        onClicked: localRoot.closeThis()
    }

    onAudioItemRefChanged: {
        audioItemRef.player.positionChanged.connect(
            (position) => {
                var secondsNow = position
                if(audioProgressSlider.pressed == false)
                    audioProgressSlider.value = secondsNow
            }
        )

        audioItemRef.availableTracksChanged.connect(
            () => {
                songListView.updateContent();
            }
        )

        audioItemRef.hasChangesThatCanBeApplyedChanged.connect(
            () => { applyBtn.enabled = audioItemRef.hasChangesThatCanBeApplyed }
        )
    }

    // ************

    Rectangle{
        anchors{
            top: closeBtn.bottom
            right: parent.right
            left: parent.left
            bottom: bottomBar.top
        }
        color: "transparent"
        clip: true

        ListView{
            id: songListView
            model: []
            anchors.fill: parent
            delegate: Item
            {
                id: localItem
                width: songListView.width
                height: 50

                property bool isSelected : songListView.currentIndex == index

                Rectangle {
                    width: parent.width
                    height: parent.height
                    color: localItem.isSelected ? Material.listHighlightColor : Material.backgroundColor

                    RowLayout {
                        anchors.fill: parent
                        anchors.leftMargin: 20
                        anchors.rightMargin: 20
                        spacing: 20
                        Label { text: modelData.name; width: 100; Layout.fillWidth: true; Layout.alignment: Qt.AlignLeft; }
                        Label { text: modelData.modified ? "modified" : ""; width: 100; Layout.alignment: Qt.AlignLeft; color:"red"; }
                        Label { text: modelData.size; width: 100; Layout.alignment: Qt.AlignLeft; }
                    }
                }

                Rectangle{
                    id: foreground

                    color: "#42FFFFFF"
                    anchors.fill: parent
                    visible: mouseArea.pressed
                }

                MouseArea {
                    id: mouseArea
                    anchors.fill: parent

                    acceptedButtons: Qt.RightButton | Qt.LeftButton

                    onDoubleClicked:(mouse) => {
                        if (mouse.button == Qt.RightButton) {
                            return;
                        }
                        if(songListView.currentIndex == index)return;
                        localRoot.audioItemRef.preparePlayTrackAtIndex(index)
                        localRoot.audioItemRef.player.play()
                        songListView.currentIndex = index
                    }

                    onClicked: (mouse) => {
                        if (mouse.button == Qt.RightButton) {
                            var globalPos = mapToGlobal(Qt.point(mouse.x, mouse.y));
                            var targetPos = windowSpace.mapFromGlobal(globalPos.x, globalPos.y);
                            contextMenu.showAt(targetPos.x, targetPos.y, index)
                            return;
                        }
                    }
                }
            }


            function updateContent(){
                let newArray = []
                let tracks = localRoot.audioItemRef.availableTracks
                for(let i = 0; i < tracks.length; i++){
                    newArray.push(
                    {
                        "name" : "track_" + (i + 1),
                        "size" : formatBytes(tracks[i]["size"]),
                        "modified" : tracks[i]["modified"]
                    }
                    )
                }
                this.model = newArray
            }
        }
    }

    // ************

    RowLayout{
        id: bottomBar
        anchors{
            bottom: parent.bottom
            left: parent.left
            right: parent.right
        }
        height: 50

        Rectangle{
            Layout.preferredWidth: 100
            height: 50
            color: Qt.lighter(Material.backgroundColor, 2)
            Button{
                flat: true
                height: 50
                width: 50
                anchors.centerIn: parent
                onClicked: {
                    if(audioItemRef.player.playing)
                        audioItemRef.player.pause()
                    else
                        audioItemRef.player.play()
                }

                icon.source:  audioItemRef.player.playbackState != MediaPlayer.PlayingState ? "qrc:/images/play_button.png" : "qrc:/images/pause_button.png"
                icon.width: 40
                icon.height: 40
            }
        }

        Rectangle{
            Layout.fillWidth: true
            Layout.fillHeight: true
            color: Qt.lighter(Material.backgroundColor, 2)
            Slider{
                id: audioProgressSlider
                anchors.fill: parent
                from: 0
                to: audioItemRef.player.duration
                live: true
                Material.accent: "orange"

                onValueChanged:{
                    if(this.pressed){
                        audioItemRef.player.position = parseInt(this.value)
                    }
                }

                Component.onCompleted: handle.height *= 0.5
            }
        }

        Rectangle{
            Layout.preferredWidth: 100
            Layout.fillHeight: true
            color: Qt.lighter(Material.backgroundColor, 2)
            Slider{
                id: control
                anchors.fill: parent
                from: 0
                to: 100
                value: 50
                live: true
                Material.accent: "gray"
                onValueChanged:{
                    localRoot.audioItemRef.player.audioOutput.volume = (this.value) / 100.0
                }
            }
        }
    }



    Menu {
        id: contextMenu

        property int indexOfSelection: -1;


        function showAt(x,y, index){
            this.indexOfSelection = index

            if(x + 5 + width >= root.width){
                x -= width + 5
            }
            else
                x += 5

            if(y + height >= root.height){
                y -= height
            }
            this.x = x
            this.y = y
            this.open()
        }

        MenuItem {
            text: "Extract Audio file"
            onTriggered: {
                localRoot.audioItemRef.saveTrackFromList(contextMenu.indexOfSelection)
            }
        }

        MenuItem {
            text: "Replace this Audio file"
            onTriggered: {
                localRoot.audioItemRef.replaceAudioFile(contextMenu.indexOfSelection)
            }
        }
    }


    function formatBytes(bytes, decimals = 2) {
        if (bytes === 0) return '0 B';

        const k = 1024;
        const dm = decimals < 0 ? 0 : decimals;
        const sizes = ['B', 'KB', 'MB', 'GB', 'TB', 'PB', 'EB', 'ZB', 'YB'];

        const i = Math.floor(Math.log(bytes) / Math.log(k));

        return parseFloat((bytes / Math.pow(k, i)).toFixed(dm)) + ' ' + sizes[i];
    }

    function applyChanged(){
        audioItemRef.saveChanges()
    }

    function closeThis(){
        this.visible = false
        audioItemRef.player.stop()
        songListView.currentIndex = -1;
    }

    function show(){
        this.visible = true
    }
}
