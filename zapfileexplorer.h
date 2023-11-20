#ifndef ZAPFILEEXPLORER_H
#define ZAPFILEEXPLORER_H

#include <QObject>
#include "ZAP_FILE.h"
#include <optional>
#include <QQuickWindow>
#include "audioeditoritem.h"
using namespace ZAP;
class ZapFileExplorer : public QObject
{
    Q_OBJECT

private:
    InMemZAP loadedZAP;

    Q_PROPERTY(QStringList files READ getFiles NOTIFY filesChanged FINAL)
    Q_PROPERTY(QStringList folders READ getFolders NOTIFY foldersChanged FINAL)
    Q_PROPERTY(QString folderPath READ getFolderPath WRITE setFolderPath NOTIFY folderPathChanged FINAL)
    Q_PROPERTY(AudioEditorItem* audioEditorItem READ getAudioEditorItem CONSTANT)

    QStringList folders;
    QStringList files;
    AudioEditorItem audioEditorItem;

    QString folderPath;
    Folder* activeFolder;

public:
    explicit ZapFileExplorer(QObject *parent = nullptr);

    Q_INVOKABLE void loadExample(){
        // Well this is just for me personally
        // setLoadedZAP(LOAD_ZAP_FILE("X:\\Games\\Thrillville Off The Rails\\auddata\\music.zap"));
    }
    void unload();

    InMemZAP getLoadedZAP() const;
    void setLoadedZAP(const InMemZAP &newLoadedZAP);

    QStringList getFiles() const;
    void setFiles(const QStringList &newFiles);
    QStringList getFolders() const;
    void setFolders(const QStringList &newFolders);

    Folder *getActiveFolder() const;
    void setActiveFolder(Folder *newActiveFolder);

    QString getFolderPath() const;
    void setFolderPath(const QString &newFolderPath);

    Q_INVOKABLE void chdir(QString dirName);

    Q_INVOKABLE void upDir();

    Q_INVOKABLE QVariantMap getFileInfo(QString fName);

    Q_INVOKABLE bool deleteIt(QString fName, QString type);

    Q_INVOKABLE bool rename(QString fName, QString type, QString newName);

    Q_INVOKABLE void extract(QString fName, QString type);

    Q_INVOKABLE void replaceFile(QString fName);


    Q_INVOKABLE void loadFile();

    Q_INVOKABLE void saveFile();

    Q_INVOKABLE void addFileFromDisk(QString url, QString targetFolder);

    Q_INVOKABLE void extractOGGAudio(QString fName, QString type);

    Q_INVOKABLE bool sendFileToAudioEditorItem(QString fname);

    static QQuickWindow* activeWindow;


    AudioEditorItem *getAudioEditorItem();

signals:
    void filesChanged();
    void foldersChanged();
    void folderPathChanged();
    void refreshRequested();

private slots:
    void onAudioFileApplyChanges(QString name, QList<QByteArray> trackData);

};

#endif // ZAPFILEEXPLORER_H
