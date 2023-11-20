#include "zapfileexplorer.h"
#include <algorithm>
#include <QFileDialog>
#include <QString>
#include <QApplication>
#include <QQuickWindow>
#include <QDir>
#include <QFile>

inline void WaitForNewFrame(){
    qDebug() << ">> Waiting for Frame";
    bool done = false;
    auto con = QObject::connect(ZapFileExplorer::activeWindow, &QQuickWindow::frameSwapped, [&]() {
        done = true;
    });
    ZapFileExplorer::activeWindow->requestUpdate();
    while(done == false){
        qApp->processEvents(QEventLoop::ProcessEventsFlag::AllEvents, 0);
    }
    QObject::disconnect(con);
    qDebug() << "<< Waiting for Frame";
}

QQuickWindow* ZapFileExplorer::activeWindow = nullptr;

InMemZAP ZapFileExplorer::getLoadedZAP() const
{
    return loadedZAP;
}

void ZapFileExplorer::setLoadedZAP(const InMemZAP &newLoadedZAP)
{
    loadedZAP.cleanUp();
    loadedZAP = newLoadedZAP;
    setActiveFolder(loadedZAP.rootFolder);
}

QStringList ZapFileExplorer::getFolders() const
{
    return folders;
}

void ZapFileExplorer::setFolders(const QStringList &newFolders)
{
    if(newFolders == folders)return;
    folders = newFolders;
    emit foldersChanged();
}

QStringList ZapFileExplorer::getFiles() const
{
    return files;
}

void ZapFileExplorer::setFiles(const QStringList &newFiles)
{
    if (files == newFiles)
        return;
    files = newFiles;
    emit filesChanged();
}

Folder *ZapFileExplorer::getActiveFolder() const
{
    return activeFolder;
}

void ZapFileExplorer::setActiveFolder(Folder *newActiveFolder)
{
    activeFolder = newActiveFolder;
    if(activeFolder){
        QStringList pathNow;
        Folder* tmp = activeFolder;
        pathNow << tmp->name;
        while(tmp->getParentFolder()){
            tmp = tmp->getParentFolder();
            pathNow.push_front(tmp->name);
        }
        QString newFolderPath = pathNow.join("/"); // folder names already contain  a /
        setFolderPath(newFolderPath + "/");
        setFolders(activeFolder->getAllFolderNames());
        setFiles(activeFolder->getAllFileNames());
    }
    else{
        setFolders({});
        setFiles({});
        setFolderPath("/");
    }
}

QString ZapFileExplorer::getFolderPath() const
{
    return folderPath;
}

void ZapFileExplorer::setFolderPath(const QString &newFolderPath)
{
    if (folderPath == newFolderPath)
        return;
    folderPath = newFolderPath;
    emit folderPathChanged();
}

void ZapFileExplorer::chdir(QString dirName)
{
    if(!activeFolder){
        qDebug() << "Cannot CHDIR if no valid folder is in memory";
        return;
    }
    for(auto folder : this->activeFolder->subFolders){
        if(folder->name == dirName)
        {
            setActiveFolder(folder);
            return;
        }
    }
}

void ZapFileExplorer::upDir()
{
    if(!activeFolder){
        qDebug() << "Cannot UPDIR if no valid folder is in memory";
        return;
    }

    auto parent = this->activeFolder->getParentFolder();
    if(parent == nullptr)parent = this->activeFolder;// root Folder reached again
    setActiveFolder(parent);
}

QVariantMap ZapFileExplorer::getFileInfo(QString fName)
{
    QVariantMap map;
    if(!activeFolder){
        qDebug() << "No Valid Folder for GetFileInfo here!";
        return map;
    }

    for(auto el :  activeFolder->files)
        if(el.name == fName){
            map["size"] = el.data.size;
            // its quick so it does not matter if we call it here without any form of baking
            auto analyzationResult = el.data.analyze();
            map["is_ogg_audio"] = analyzationResult.hasOGGAudio; // has ogg file "magic" identifier
            QVariantMap oggFacts;
            oggFacts["embedded_tracks"] = analyzationResult.trackCount;
            map["ogg_facts"] = oggFacts;
            break;
        }
    return map;
}

bool ZapFileExplorer::deleteIt(QString fName, QString type)
{
    if(!activeFolder){
        qDebug() << "No Valid Folder for " << __func__ << " here!";
        return false;
    }

    if(type == "folder"){
        Folder* f = activeFolder->findSubFolderByName(fName);
        if(f){
            activeFolder->subFolders.removeAll(f);
            Folder::DeleteFolderAndContent(f);
            setActiveFolder(activeFolder);
            return true;
        }
    }
    if(type == "file"){
        int index = activeFolder->findFileIndexByName(fName);
        if(index >= 0){
            activeFolder->files.takeAt(index).data.free();
            setActiveFolder(activeFolder);
            return true;
        }
    }
    return false;
}

bool ZapFileExplorer::rename(QString fName, QString type, QString newName)
{
    newName = ZAP::NormalizeName(newName);
    // does the name already exist?
    if(activeFolder->findSubFolderByName(newName.trimmed()) != nullptr ||
        activeFolder->findFileIndexByName(newName.trimmed()) != -1)
    {
        qDebug() << "Cannot rename the file/folder if there is already a file with that name in that folder.";
        return false;
    }
    bool changed = false;
    if(type == "file"){
        int index = activeFolder->findFileIndexByName(fName);
        if(index >= 0){
            activeFolder->files[index].name = newName;
            changed = true;
        }
    }
    if(type == "folder"){
        Folder* f = activeFolder->findSubFolderByName(fName);
        if(f != nullptr){
            f->name = newName;
            changed = true;
        }
    }
    if(changed)
        setActiveFolder(activeFolder);
    return false;
}

void ZapFileExplorer::extract(QString fName, QString type)
{
    if(activeFolder == nullptr)return;

    if(type == "file"){
        int index = activeFolder->findFileIndexByName(fName);
        if(index >= 0){
            auto file = activeFolder->files[index].data;
            QFileInfo fileName = QFileInfo(QFileDialog::getSaveFileName(nullptr, "Save File", fName, "Alle Dateien (*)"));
            if(fileName.fileName().isEmpty())return;
            QFile f;
            f.setFileName(fileName.absoluteFilePath());
            f.open(QIODevice::WriteOnly);
            f.write(file.data, file.size);
        }
        else
            qDebug() << "Extract Failed No File Found to extract O.o";
    }
    if(type == "folder"){
        Folder* f = activeFolder->findSubFolderByName(fName);
        if(fName.isEmpty())
        {
            qDebug() << "Special case found. We use the root Folder!";
            f = activeFolder;
        }
        if(f == nullptr){
            qDebug() << "Folder was not found..";
            return;
        }
        QString dir = QFileDialog::getExistingDirectory(nullptr, "Select a destination Folder", "", QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks);
        if(dir.isEmpty())return;
        recursiveDumpFolderContent(f, dir, false);
    }
}

void ZapFileExplorer::replaceFile(QString fName)
{
    if(activeFolder == nullptr)return;

    int index = activeFolder->findFileIndexByName(fName);
    if(index < 0)return;
    QFileInfo fileName = QFileInfo(QFileDialog::getOpenFileName(nullptr, "Load Replacement File", fName, "Alle Files (*)"));
    QFile f;
    f.setFileName(fileName.absoluteFilePath());
    f.open(QIODevice::ReadOnly);
    if(f.isOpen() == false){
        qDebug() << "replaceFile Failed because replacement file could not be opened";
        return;
    }
    QByteArray fileData = f.readAll();
    Resource<ResourceData>& fileRef = activeFolder->files[index];
    fileRef.data.free();
    fileRef.data = ResourceData::FromData(fileData.data(), fileData.length());
    emit refreshRequested();
}

void ZapFileExplorer::loadFile()
{
    QFileInfo fileName = QFileInfo(QFileDialog::getOpenFileName(nullptr, "Load Replacement File", "", "ZAP File (*.zap)"));
    if(fileName.exists() == false)
    {
        qDebug() << "File does not exist";
        return;
    }
    setLoadedZAP(LOAD_ZAP_FILE(fileName.absoluteFilePath()));
}

void ZapFileExplorer::saveFile()
{
    if(loadedZAP.isValid() == false){
        qDebug() << "Cannot save file if .zap is invalid";
        return;
    }
    QFileInfo fileName = QFileInfo(QFileDialog::getSaveFileName(nullptr, "Save File", "", "ZAP File (*.zap)"));
    if(fileName.fileName().isEmpty())return;
    auto zap = getLoadedZAP();
    SAVE_ZAP_FILE(zap, fileName);
}

void ZapFileExplorer::addFileFromDisk(QString url, QString targetFolder)
{
    Folder* target = activeFolder;
    if(targetFolder.isEmpty() == false)
        target = target->findSubFolderByName(targetFolder);
    if(target == nullptr)
    {
        qDebug() << "addFileFromDisk Failed. Target Folder could not be found :/";
        return;
    }
    QFileInfo info(QUrl::fromUserInput(url).toLocalFile());

    QFile f(info.absoluteFilePath());
    if(!f.open(QIODevice::ReadOnly))
    {
        qDebug() << "Was not able to open file at: " << info.absoluteFilePath();
        return;
    }
    QByteArray arr = f.readAll();
    ResourceData toLoad = ResourceData::FromData(arr.data(), arr.length());
    QString fName = NormalizeName(info.fileName().trimmed());
    Resource<ResourceData> newResource{toLoad, fName};

    if(target->findFileIndexByName(fName) != 0)
        this->deleteIt(fName, "file");
    target->addFile(newResource);
    setActiveFolder(activeFolder);
}

void ZapFileExplorer::extractOGGAudio(QString fName, QString type)
{
    if(activeFolder == nullptr)return;
    int index = activeFolder->findFileIndexByName(fName);
    if(index >= 0){
        QString dir = QFileDialog::getExistingDirectory(nullptr, "Select a destination Folder", "", QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks);
        if(dir.isEmpty())return;
        auto file = activeFolder->files[index].data;

        auto allOGGFiles = GetAllOGGFilesInDataArea(file.data, nullptr);

        QString baseName = fName;
        int counter = 0;
        Q_FOREACH(auto oggF, allOGGFiles){
            counter++;
            QString nowName = fName + "_part" + QString::number(counter)+ ".ogg";
            QFileInfo targetLocation(dir, nowName);
            QFile f;
            f.setFileName(targetLocation.absoluteFilePath());
            f.open(QIODevice::WriteOnly);
            f.write(oggF.first, oggF.second);
        }
    }
    else
        qDebug() << "Extract Failed No File Found to extract O.o";
}

bool ZapFileExplorer::sendFileToAudioEditorItem(QString fName)
{
    if(activeFolder == nullptr)return false;
    int index = activeFolder->findFileIndexByName(fName);
    if(index <= -1)return false;
    auto resource = activeFolder->files[index];
    auto result = resource.data.analyze();
    if(!result.hasOGGAudio){
        qDebug() << "File does not seem to have ogg audio!";
        return false;
    }
    audioEditorItem.setTargetFile(resource);
    return true;
}

AudioEditorItem *ZapFileExplorer::getAudioEditorItem()
{
    return &this->audioEditorItem;
}

void ZapFileExplorer::onAudioFileApplyChanges(QString name, QList<QByteArray> trackData)
{
    if(activeFolder == nullptr)return;
    int index = activeFolder->findFileIndexByName(name);
    if(index <= -1)return;

    QByteArray allTracks = trackData.join();

    activeFolder->files[index].data.free();
    ResourceData newAudioData = ResourceData::FromData(allTracks.data(), allTracks.size());
    activeFolder->files[index].data = newAudioData;
    audioEditorItem.setTargetFile(activeFolder->files[index]);
}

ZapFileExplorer::ZapFileExplorer(QObject *parent)
    : QObject{parent}
{
    connect(&this->audioEditorItem, &AudioEditorItem::onAudioResourceUpdated, this, &ZapFileExplorer::onAudioFileApplyChanges);
}

void ZapFileExplorer::unload()
{
    loadedZAP.cleanUp();
    setLoadedZAP(InMemZAP());
    setActiveFolder(nullptr);
}
