#ifndef AUDIOEDITORITEM_H
#define AUDIOEDITORITEM_H

#include <QObject>
#include <QBuffer>
#include <QMediaPlayer>
#include "ZAP_FILE.h"
class AudioEditorItem : public QObject
{
    Q_OBJECT


    QMediaPlayer player;
    QBuffer audioBuffer;
    QByteArray loadedSong;

    ZAP::Resource<ZAP::ResourceData> targetFile;
    QList<QByteArray> trackDataList;

    void loadFromTargetFile();

    QVariantList availableTracks;

    int lastLoadedIndex;
    bool hasChangesThatCanBeApplyed;

public:
    explicit AudioEditorItem(QObject *parent = nullptr);
    QMediaPlayer *getPlayer();

    void setTargetFile(const ZAP::Resource<ZAP::ResourceData> &newTargetFile);

    QVariantList getAvailableTracks() const;
    void setAvailableTracks(const QVariantList &newAvailableTracks);
    void resetAvailableTracks();

    Q_INVOKABLE void preparePlayTrackAtIndex(int trackIndex);

    Q_INVOKABLE void replaceAudioFile(int indexOfReplacement);

    Q_INVOKABLE void saveTrackFromList(int indexOfFile);

    Q_INVOKABLE void saveChanges();

    bool getHasChangesThatCanBeApplyed() const;
    void setHasChangesThatCanBeApplyed(bool newHasChangesThatCanBeApplyed);

signals:
    void availableTracksChanged();

    void hasChangesThatCanBeApplyedChanged();

    void onAudioResourceUpdated(QString name, QList<QByteArray> tracks);

private:
    Q_PROPERTY(QMediaPlayer* player READ getPlayer CONSTANT FINAL)
    Q_PROPERTY(QVariantList availableTracks READ getAvailableTracks WRITE setAvailableTracks RESET resetAvailableTracks NOTIFY availableTracksChanged FINAL)

    void swapAudio(QByteArray data);

    Q_PROPERTY(bool hasChangesThatCanBeApplyed READ getHasChangesThatCanBeApplyed WRITE setHasChangesThatCanBeApplyed NOTIFY hasChangesThatCanBeApplyedChanged FINAL)
};

#endif // AUDIOEDITORITEM_H
