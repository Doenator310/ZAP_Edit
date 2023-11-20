#include "audioeditoritem.h"
#include <QAudioOutput>
#include <QFile>
#include <QFileDialog>
QMediaPlayer* AudioEditorItem::getPlayer()
{
    return &player;
}

void AudioEditorItem::swapAudio(QByteArray data)
{
    player.stop();
    QBuffer placeHolder;
    player.setSourceDevice(&placeHolder);
    // Write überschreibt diesen komplett glaube ich
    this->audioBuffer.open(QIODevice::WriteOnly);
    this->audioBuffer.reset();
    this->audioBuffer.write(data);
    this->audioBuffer.close();

    player.setSourceDevice(&this->audioBuffer);
}


void AudioEditorItem::setTargetFile(const ZAP::Resource<ZAP::ResourceData> &newTargetFile)
{
    targetFile = newTargetFile;
    loadFromTargetFile();
    setHasChangesThatCanBeApplyed(false);
}

void AudioEditorItem::loadFromTargetFile()
{
    this->trackDataList.clear();
    QVariantList list;
    auto allFound = ZAP::GetAllOGGFilesInDataArea(this->targetFile.data.data, nullptr);
    for(auto el : allFound){
        trackDataList << QByteArray::fromRawData(el.first, el.second);
        QVariantMap map;
        map["size"] = el.second;
        map["modified"] = false;
        list << map;
    }
    setAvailableTracks(list);
    preparePlayTrackAtIndex(0);
}

bool AudioEditorItem::getHasChangesThatCanBeApplyed() const
{
    return hasChangesThatCanBeApplyed;
}

void AudioEditorItem::setHasChangesThatCanBeApplyed(bool newHasChangesThatCanBeApplyed)
{
    if (hasChangesThatCanBeApplyed == newHasChangesThatCanBeApplyed)
        return;
    hasChangesThatCanBeApplyed = newHasChangesThatCanBeApplyed;
    emit hasChangesThatCanBeApplyedChanged();
}

QVariantList AudioEditorItem::getAvailableTracks() const
{
    return availableTracks;
}

void AudioEditorItem::setAvailableTracks(const QVariantList &newAvailableTracks)
{
    if (availableTracks == newAvailableTracks)
        return;
    availableTracks = newAvailableTracks;
    emit availableTracksChanged();
}

void AudioEditorItem::resetAvailableTracks()
{
    setAvailableTracks({}); // TODO: Adapt to use your actual default value
}

void AudioEditorItem::preparePlayTrackAtIndex(int trackIndex)
{
    lastLoadedIndex = trackIndex;
    if(trackIndex >= trackDataList.count()){
        qDebug() << "FAILED! Track does not exist at" << trackIndex;
        return;
    }
    this->swapAudio(trackDataList[trackIndex]);
}

void AudioEditorItem::replaceAudioFile(int indexOfReplacement)
{
    if(indexOfReplacement >= trackDataList.count()){
        qDebug() << "FAILED D: Track does not exist at" << indexOfReplacement;
        return;
    }

    QFileInfo fileName = QFileInfo(QFileDialog::getOpenFileName(nullptr, "Load Replacement File", "", "OGG file (*.ogg)"));
    QFile f;
    f.setFileName(fileName.absoluteFilePath());
    f.open(QIODevice::ReadOnly);
    if(f.isOpen() == false){
        qDebug() << "replaceFile Failed because replacement file could not be opened";
        return;
    }
    QByteArray data = f.readAll();
    // check for valid ogg file
    if(!OGG::is_ogg(data.data(), data.size())){
        qDebug() << "The replacement file is not a valid ogg file..";
        return;
    }
    if(data.size() < 100){qDebug() << "OGG is too small"; return;}
    OGG::prepareExternalOGGStream(data.data());
    trackDataList[indexOfReplacement] = data;
    if(lastLoadedIndex == indexOfReplacement){
        preparePlayTrackAtIndex(indexOfReplacement);
    }

    // update meta data :)

    QVariantList list = getAvailableTracks();
    QVariantMap map = list[indexOfReplacement].value<QVariantMap>();
    map["size"] = data.size();
    map["modified"] = true;
    list[indexOfReplacement] = map;
    setAvailableTracks(list);
    setHasChangesThatCanBeApplyed(true);
}

void AudioEditorItem::saveTrackFromList(int indexOfFile)
{
    if(indexOfFile >= trackDataList.count()){
        qDebug() << "FAILED! Track does not exist at" << indexOfFile;
        return;
    }
    QString name = this->targetFile.name + ".track_" + QString::number(indexOfFile+1) + ".ogg";
    QFileInfo fileName = QFileInfo(QFileDialog::getSaveFileName(nullptr, "Save File", name, "OGG File (*.ogg)"));
    if(fileName.fileName().isEmpty())return;
    QByteArray arr = this->trackDataList[indexOfFile];
    QFile f;
    f.setFileName(fileName.absoluteFilePath());
    f.open(QIODevice::WriteOnly);
    f.write(arr);
}

void AudioEditorItem::saveChanges()
{
    emit onAudioResourceUpdated(targetFile.name, this->trackDataList);
    setHasChangesThatCanBeApplyed(false);
}

AudioEditorItem::AudioEditorItem(QObject *parent)
    : QObject{parent}, targetFile(ZAP::Resource<ZAP::ResourceData>::EMPTY())
{
    player.setAudioOutput(new QAudioOutput());
    lastLoadedIndex = -1;
    hasChangesThatCanBeApplyed = false;
    setHasChangesThatCanBeApplyed(false);
}
