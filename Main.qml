import QtQuick
import QtQuick.Window
import QtQuick.Controls
import QtQuick.Layouts
import QtQuick.Controls.Material
import ZapFileExplorer

Window {
    id: root

    width: 800
    height: 480
    visible: true
    title: qsTr("ZAP File Manager [PC | Thrillville Off the Rails]")

    Material.theme: Material.Dark // Setzen des Material Themes
    Material.accent: Material.Blue // Festlegen der Akzentfarbe

    Component.onCompleted: {
        explorer.loadExample()
    }

    Item{
        id: windowSpace;
        anchors.fill: parent
    }

    ZapFileExplorer{
        id: explorer
        onFilesChanged:{
            fileList.updateContent()
        }
        onFoldersChanged:{
            fileList.updateContent()
        }
        onFolderPathChanged:{

        }
        onRefreshRequested:{
            fileList.updateContent()
        }

        function onFileDropped(file, optional_folder = ""){
            console.log(">>!", optional_folder, file)
            this.addFileFromDisk(file, optional_folder)
        }

        Component.onCompleted: fileList.updateContent()
    }

    DropArea {
        anchors.fill: parent
        onDropped: (drop) => {
                       if (drop.hasUrls)
                       for (var i = 0; i < drop.urls.length; ++i)
                       explorer.onFileDropped(drop.urls[i])
                   }
    }


    MouseArea {
        anchors.fill: parent
        acceptedButtons: Qt.RightButton

        onClicked: (mouse) => {
                       contextMenu.showAt(mouse.x, mouse.y, "", "folder", -1, false, true)
                   }
    }


    Rectangle{
        anchors.fill: parent
        color: Material.backgroundColor
    }

    ColumnLayout {
        anchors.fill: parent
        anchors.margins: 10

        RowLayout{
            Layout.fillWidth: true
            Button{
                icon.source: "qrc:/images/folder_up_icon.png"
                icon.width: 40
                icon.height: 40
                width: 40
                height: 40
                flat: true
                onClicked:{
                    explorer.upDir()
                }
            }
            TextField {
                id: pathInput
                Layout.fillWidth: true
                text: explorer.folderPath
                readOnly: true
            }

            Button{
                icon.source: "qrc:/images/load_file_icon.png"
                icon.width: 40
                icon.height: 40
                width: 40
                height: 40
                flat: true
                onClicked:{
                    explorer.loadFile();
                }
            }
            Button{
                icon.source: "qrc:/images/save_file_icon.png"
                icon.width: 40
                icon.height: 40
                width: 40
                height: 40
                flat: true
                onClicked:{
                    explorer.saveFile();
                }
            }
        }

        ListView {
            id: fileList
            Layout.fillWidth: true
            Layout.fillHeight: true
            clip: true
            reuseItems: true
            snapMode: ListView.SnapToItem

            maximumFlickVelocity: 100000 // Standardwert ist normalerweise höher
            // boundsBehavior: Flickable.StopAtBounds // Flickable.StopAtBounds
            // boundsMovement: Flickable.StopAtBounds
            rebound: Transition {
                NumberAnimation {
                    properties: "x,y"
                    duration: 50
                    easing.type: Easing.OutExpo;
                }
            }

            onContentYChanged: {
                if (contentY < 0) {
                    positionViewAtIndex(0, ListView.SnapPosition)
                }
            }

            function renameElement(nameOfEl, type, newName){
                let _y = fileList.contentY
                explorer.rename(nameOfEl, type, newName)
                fileList.contentY = _y
            }


            function formatBytes(bytes, decimals = 2) {
                if (bytes === 0) return '0 B';

                const k = 1024;
                const dm = decimals < 0 ? 0 : decimals;
                const sizes = ['B', 'KB', 'MB', 'GB', 'TB', 'PB', 'EB', 'ZB', 'YB'];

                const i = Math.floor(Math.log(bytes) / Math.log(k));

                return parseFloat((bytes / Math.pow(k, i)).toFixed(dm)) + ' ' + sizes[i];
            }


            function updateContent(){
                var images = {
                    "file" : "qrc:/images/file_icon.png",
                    "folder" : "qrc:/images/folder_icon.png",
                    "ogg_audio" : "qrc:/images/audio_icon.png"
                }
                // this.model.clear()
                let newArray = []
                let folders = explorer.folders
                for(let i = 0; i < folders.length; i++){
                    newArray.push({
                                      "name" : folders[i], "type":"folder",
                                      "size" : "0",
                                      "iconUrl" : images["folder"],
                                      "fileFacts" : "", // folder does not have file facts yet
                                      "special-mode" : false
                                  })
                }

                let files = explorer.files
                for(var i = 0; i < files.length; i++){
                    let info = explorer.getFileInfo(files[i])
                    var iconUrl = images["file"]
                    var bonusFileFacts = ""
                    var specialMode = false
                    if(info["is_ogg_audio"])
                    {
                        iconUrl = images["ogg_audio"]
                        bonusFileFacts = "embeded tracks: " + info["ogg_facts"]["embedded_tracks"].toString()
                        specialMode = true
                    }
                    // console.log(JSON.stringify(info))
                    newArray.push({
                                      "name" : files[i], "type":"file",
                                      "size" : this.formatBytes(info["size"]),
                                      "iconUrl" : iconUrl,
                                      "fileFacts" : bonusFileFacts,
                                      "special-mode" : specialMode
                                  })
                }
                this.model = newArray
            }

            model: []

            delegate: Item
            {
                id: localRoot
                width: fileList.width
                height: 50

                property bool isSelected : fileList.currentIndex == index

                Rectangle {
                    width: parent.width
                    height: parent.height
                    color: localRoot.isSelected ? Material.listHighlightColor : Material.backgroundColor

                    RowLayout {
                        anchors.fill: parent
                        spacing: 20
                        Image{
                            width: height
                            height: parent.height
                            sourceSize.width: height
                            sourceSize.height: height
                            source: modelData.iconUrl
                            asynchronous: true
                            cache: true
                        }
                        Label { text: modelData.name; Layout.fillWidth: true }
                        Label { text: modelData.fileFacts; visible: modelData.type == "file"; width: 150; horizontalAlignment: Qt.AlignLeft; }
                        Label { text: modelData.size; visible: modelData.type == "file"; width: 100; }
                    }

                    DropArea {
                        anchors.fill: parent
                        onDropped: (drop) => {
                                       if (drop.hasUrls)
                                       for (var i = 0; i < drop.urls.length; ++i)
                                       explorer.onFileDropped(drop.urls[i], modelData.type == "file" ? undefined : modelData.name)
                                   }
                    }
                }

                MouseArea {
                    anchors.fill: parent

                    acceptedButtons: Qt.RightButton | Qt.LeftButton

                    onClicked:(mouse) => {
                                  if (mouse.button == Qt.RightButton) {
                                      var globalPos = mapToGlobal(Qt.point(mouse.x, mouse.y));
                                      var targetPos = windowSpace.mapFromGlobal(globalPos.x, globalPos.y);
                                      contextMenu.showAt(targetPos.x, targetPos.y, modelData.name, modelData.type,model.index, modelData["special-mode"], false)
                                  }
                                  fileList.currentIndex = index
                              }

                    onDoubleClicked: {
                        if(modelData.type == "folder"){
                            explorer.chdir(modelData.name);
                        }
                    }
                }


            }
        }
    }


    // ------------------------- POPUP AREA ---------------------------

    Menu {
        id: contextMenu

        property string target: "";
        property string type: "";
        property int indexOfSelection: -1;
        property bool isCurrentRootFolder: false
        property bool specialMode: false


        function showAt(x,y, _file, _type, _indexOfSelection, _specialMode, _isCurrentRootFolder = false){
            this.target = _file
            this.type   = _type
            this.specialMode = _specialMode
            this.indexOfSelection = _indexOfSelection
            this.isCurrentRootFolder = _isCurrentRootFolder

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
            text: "Rename"
            enabled: !contextMenu.isCurrentRootFolder
            onTriggered: {
                renamePopup.showIt(contextMenu.target, contextMenu.type)
            }//explorer.rename(contextMenu.target, contextMenu.type)
        }

        MenuItem {
            text: contextMenu.type == "file" ? "Extract File" : (contextMenu.type == "folder" ? "Extract Folder" : "???")
            onTriggered:{ explorer.extract(contextMenu.target, contextMenu.type);  }
        }

        MenuItem {
            enabled: contextMenu.type == "file" && contextMenu.specialMode
            text: "Special-Extractor"
            onTriggered:{ explorer.extractOGGAudio(contextMenu.target, contextMenu.type);  }
        }

        MenuItem {
            enabled: contextMenu.type == "file" && contextMenu.specialMode
            text: "Edit-Audio"
            onTriggered:{
                if(explorer.sendFileToAudioEditorItem(contextMenu.target)){
                    oggSurgeon.show()
                }
            }
        }

        MenuItem {
            enabled: contextMenu.type == "file"
            text: "Replace"
            onTriggered: {
                let _y = fileList.contentY
                if(contextMenu.type === "file")
                    explorer.replaceFile(contextMenu.target)
                fileList.contentY = _y
            }
        }

        MenuItem {
            text: "Delete"
            enabled: !contextMenu.isCurrentRootFolder
            onTriggered: {
                let _y = fileList.contentY
                let deleted = explorer.deleteIt(contextMenu.target, contextMenu.type);
                if(!deleted)return;
                fileList.contentY = _y
            }
        }


    }

    Popup {
        id: renamePopup

        anchors.centerIn: parent
        width: 500
        height: 150
        modal: true
        focus: true
        closePolicy: Popup.CloseOnEscape | Popup.CloseOnPressOutsideParent

        property string renameName;
        property string type;

        function showIt(_name, _type){
            this.type = _type
            this.renameName = _name.replace("..", "").replace("/", "").trim()
            nameField.text  = this.renameName
            this.open()
        }

        Column {
            anchors.fill: parent
            anchors.margins: 10
            Label {
                Layout.fillWidth: true
                text: "Enter a new Name!"
            }
            TextField {
                id: nameField
                width: parent.width - 20
                text: ""
                onAccepted: {
                    parent.currentName = text
                    renamePopup.close()
                }
            }

            Row {
                Layout.fillWidth: true
                spacing: 10
                layoutDirection:  Qt.RightToLeft
                width: parent.width
                Button {
                    text: "OK"
                    onClicked: {
                        fileList.renameElement(renamePopup.renameName, renamePopup.type, nameField.text)
                        renamePopup.close()
                    }
                    flat: true
                    Layout.alignment: Qt.AlignRight
                }
                Button {
                    text: "Close"
                    onClicked: renamePopup.close()
                    flat: true
                    Layout.alignment: Qt.AlignRight

                }
            }
        }
    }

    OGGSurgeon{
        id: oggSurgeon
        anchors.fill: parent
        audioItemRef: explorer.audioEditorItem
        visible: false
    }
}




