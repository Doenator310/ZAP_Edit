#pragma once
#ifndef ZAP_FILE_H
#define ZAP_FILE_H

#include <QObject>
#include "oggformat.h"
#include <QFile>
#include <QDir>
#include <QByteArray>
#include <QSharedPointer>
#include <QDebug>
#include <QStack>
namespace ZAP {
#pragma pack(push,1)

// Some OGG Helper functions:
static QPair<char*, size_t> GetOGGAtLocalIndex(char* data_ptr, quint32 index, char* dbg_buff_start){
    quint64 distance1 = 0;
    size_t buff_size = 0;
    while(index > 0){
        distance1 = (data_ptr - dbg_buff_start);
        buff_size = OGG::get_size_of_ogg(data_ptr);
        data_ptr += buff_size;
        index--;
    }
    size_t _final_size = OGG::get_size_of_ogg(data_ptr);
    int x = 0;
    x++;
    return QPair<char*, size_t>(data_ptr, _final_size);
}

static QList<QPair<char*, size_t>> GetAllOGGFilesInDataArea(char* data_ptr, char* dbg_buff_start){

    quint64 distance1 = 0;
    size_t buff_size = 0;
    QList<QPair<char*, size_t>> listOfEntries;
    if(data_ptr == nullptr)return listOfEntries;
    while(true){
        distance1 = (data_ptr - dbg_buff_start);
        buff_size = OGG::get_size_of_ogg(data_ptr);
        if(buff_size == 0)break;
        listOfEntries << QPair<char*, size_t>(data_ptr, buff_size);
        data_ptr += buff_size;
    }
    return listOfEntries;
}

#define IS_DIR_TYPE 0
#define IS_FILE_TYPE 1

inline QString NormalizeName(QString text){
    return text.replace("/", "").replace("..","").trimmed();
}

// To store the parsed ZAP Files, the Data is stored in "data" and of course we save the name
template<typename T>
struct Resource{
    T data;
    QString name;

    Resource(T& data, QString name){
        this->data = data;
        this->name = name;
    }

    static Resource EMPTY(){
        T a{};
        return Resource(a, "????");
    }
};

// Result struct for storing the info about a file in the zap.
struct SpecialFeatureResult {
    bool hasOGGAudio = false;
    int  trackCount = 0;
};

struct ResourceData {
    char* data = nullptr;
    size_t size = 0;

    size_t length() {
        return size;
    }

    void free(){
        if(this->data != nullptr)
            delete this->data;
        else{
            qDebug() << "Free Called but it was already freed!";
        }
        this->data = nullptr;
        size = 0;
    }

    void set(char* data, size_t size){
        if(this->data != nullptr){
            qDebug() << "WARNING! SET CALLED BUT DATA WAS ALREADY LOADED. IT MIGHT BE GONE NOW";
        }
        this->data = data;
        this->size = size;
    }

    void createFromData(const char* src, size_t size){
        this->data = new char[size];
        memcpy(this->data, src, size);
        this->size = size;
    }

    static ResourceData FromData(const char* src, size_t size){
        ResourceData data;
        data.createFromData(src, size);
        return data;
    }

    SpecialFeatureResult analyze();
};

// This here are the actual ZAP File structs
struct ZAP_HEADER {
    char ZAP_ID[4];
    unsigned long block_size;
    long entries;
    long fileOffsetChecksum;
    char line_break_padding[2];
};

// Well the zap starts with a Table of contents. So this is just an entry of that
struct ZAP_FILE{
    long SIZE; // I dont know if a file could be invalid by having a size of -1. So thats why i did not make it unsigned yet
    quint16 fNameLen; // includes nullbyte
    char _name_start;

    const char* getNextPos() const{
        return (reinterpret_cast<const char*>(this) + 4) + fNameLen + 1 /*Nullbyte*/ + 2 /*LineBreakPadding*/ + 1/* actual next pos */;
    }

    QString  getName() const{
        const char* _nameInMem = &_name_start;
        return QLatin1String(_nameInMem);
    }
};

struct REF_ZAP_FILE{
    ZAP_FILE* file_ref;
    QString name;
    int   index = 0;
    void* data_ptr;
    quint64 length;
    int local_index = 0;
};

#pragma pack(pop)

inline quint64 GetNextAlignedAddress(quint64 fileOffsetAddress, quint16 offset){
    quint64 addressValue = (quint64)fileOffsetAddress;
    addressValue = (addressValue + offset - 1) / offset;
    return addressValue * offset;
}

inline quint64 GetAlignedAddress(quint64 fileOffsetAddress, quint16 offset){
    quint64 addressValue = (quint64)fileOffsetAddress;
    addressValue = (addressValue) / offset;
    return addressValue * offset;
}

// If we want to modify the zap file, we need dump the content again. This is for the index in the zap file
inline QByteArray GetNameAsByteArray(QString name, long fileSize){
    QByteArray _text = name.toLatin1();
    quint16 fNameLen = (quint16)_text.length() + 1;
    QByteArray arr;
    arr.append((char*)&fileSize, sizeof(fileSize));
    arr.append((char*)&fNameLen, sizeof(quint16));
    arr.append(_text);
    arr.append(1, 0x00);// add nullbyte
    arr.append("\r\n", 2);
    return arr;
}

// The zap file gets parsed and something in this program memory should store the "Folders".
struct Folder{
    static QList<Folder*> allFoldersCreated;

    QList<Folder*> subFolders;
    QList<Resource<ResourceData>> files;
    QString name;
    Folder* parentFolder = nullptr;

    Folder(QString name = "INVALID INVALID INVALID!!", Folder* parent = nullptr){
        this->name = name;
        parentFolder = parent;
        allFoldersCreated << this;
    }

    ~Folder(){
        allFoldersCreated.removeAll(this);
    }

    void addFile(Resource<ResourceData>& toAdd){
        files << toAdd;
    }

    void addFolder(Folder* f){
        this->subFolders << f;
    }

    Folder* getParentFolder(){
        return this->parentFolder;
    }

    static Folder* createNew(QString name, Folder* parent = nullptr){
        if(name.length() != 0 && name.endsWith("/") == true){
            qDebug() << name;
            qDebug() << "MISTAKE! A folder shouldnt end with an /";
            exit(0);
        }
        return new Folder(name, parent);
    }

    QStringList getAllFolderNames();
    QStringList getAllFileNames();


    Folder* findSubFolderByName(QString name){
        for(auto el : subFolders)
            if(el->name == name)
                return el;
        return nullptr;
    }

    int findFileIndexByName(QString name){
        for(int i = 0; i < files.length(); i++)
            if(files[i].name == name)
                return i;
        return -1;
    }

    Folder& operator=(Folder& other) = delete;     // copy assignment operator
    Folder operator=(Folder other) = delete;       // pass-by-value is allowed

    static void DeleteFolderAndContent(Folder* folderToDelete);
};


struct InMemZAP {
    size_t block_size = 1024;
    Folder* rootFolder = nullptr;

    int getFileCount();
    void cleanUp();
    bool isValid();
};


static int GetRecursiveFolderEntryCount(Folder* f, bool isRoot = false){
    int count = f->files.count();
    for(auto el : f->subFolders){
        count += GetRecursiveFolderEntryCount(el);
    }
    if(!isRoot)
        count += 2; // Name/ and ../ are also valid
    return count;
}

inline int InMemZAP::getFileCount(){
    return GetRecursiveFolderEntryCount(this->rootFolder, true);
}

// Load the zap file into memory, dissect it and save it in a hierarchy (InMemZAP, Folder, Resource<T>)
static InMemZAP LOAD_ZAP_FILE(QString pathToFile){
    InMemZAP zap;
    QFileInfo info(pathToFile);

    QFile f(pathToFile);
    f.open(QIODevice::ReadOnly);
    QByteArray fileContent = f.readAll();

    QByteArray::ConstIterator iter = fileContent.cbegin();
    if(fileContent.size() < 0xFF){
        return zap;
    }

    // Check if its really a ZAP File
    if(QString::fromLocal8Bit(iter, 4) != QString("ZAP")){
        return zap;
    }

    // 1) just place this "template" struct ontop the raw zap data
    const ZAP_HEADER* rootHeader = (ZAP_HEADER*)iter;

    // lets print some facts about the zap file
    qDebug() << rootHeader->block_size;
    qDebug() << rootHeader->entries;
    qDebug() << rootHeader->fileOffsetChecksum;

    const char* nameSizeTableStart = reinterpret_cast<const char*>(rootHeader) + sizeof(ZAP_HEADER);

    // this here is just internal. it just reads the zap file name
    QString _path = info.fileName().split(".").takeFirst() + "/";
    QList<QString> path_parts;
    path_parts << _path;
    // Get the offset where the actual files start in the zap file. Somehow the fileOffsetChecksum helps with that...
    quint64 actualFileLocationOffset = GetNextAlignedAddress(rootHeader->fileOffsetChecksum, rootHeader->block_size);
    REF_ZAP_FILE* last_file = nullptr;
    quint64 this_size = 0;
    int debug_local_index = 0;
    Folder* rootFolder = Folder::createNew("");
    Folder* currentFolder = rootFolder;

    for(int i = 0; i < rootHeader->entries; i++)
    {
        // Get Info about the file from the NameSizeTable
        const ZAP_FILE* zap_file = reinterpret_cast<const ZAP_FILE*>(nameSizeTableStart);
        QString _name = zap_file->getName();
        // If the file has a valid size not zero, then extract it:
        if(zap_file->SIZE != 0){
            actualFileLocationOffset += this_size;
            actualFileLocationOffset = GetNextAlignedAddress(actualFileLocationOffset, rootHeader->block_size);
            this_size = zap_file->SIZE;
            debug_local_index = 0;
        }
        // Write down when we leave a folder
        if(_name.contains("..")){
            path_parts.pop_back();
            currentFolder = currentFolder->getParentFolder() ? currentFolder->getParentFolder() : currentFolder;
        }
        // Enter a new folder
        else if(_name.contains("/")){
            path_parts << _name;
            auto newF = Folder::createNew(_name.replace("/", ""), currentFolder);
            currentFolder->addFolder(newF);
            currentFolder  = newF;
        }
        else{
            // Where does the actual file data start?
            const char* file_start = (char*)(fileContent.cbegin() + actualFileLocationOffset);
            /*
            // Well we already have "ALL" the infos we need to extract it, but while developing I did not know
            // if i would need this struct here. Well turns out we dont need it. skip
            REF_ZAP_FILE* _file = new REF_ZAP_FILE{
                (ZAP_FILE*)zap_file,
                zap_file->getName(),
                i,
                (void*)file_start,
                static_cast<quint64>(zap_file->SIZE),
                local_index
            };
            */
            // We now know where the data is in the zap file and how big the size is. So we
            // Extract it into an newly allocated Array. FromData is just copying it out of the file buffer
            ResourceData data = ResourceData::FromData(file_start, static_cast<size_t>(zap_file->SIZE));
            currentFolder->files << Resource<ResourceData>(data, zap_file->getName());
            debug_local_index++;
        }

        nameSizeTableStart = zap_file->getNextPos();
    }
    // Also write down the block size
    zap.block_size = rootHeader->block_size;
    // And most important, save the hierarchy we built into the Struct.
    zap.rootFolder = rootFolder;
    return zap;
}

inline void recursiveDumpFolderContent(Folder* nowFolder, QString startDir, bool isRootDir = false){
    if(nowFolder == nullptr){ qDebug() << "FOLDER IS ZERO!"; return; }
    QDir d(startDir);
    if(!isRootDir){
        d.mkdir(nowFolder->name);
        d.cd(nowFolder->name);
    }
    for(auto el : nowFolder->files)
    {
        QFileInfo path(d, el.name);
        QFile f(path.absoluteFilePath());
        f.open(QIODevice::WriteOnly);
        f.write(el.data.data, el.data.size);
    }

    for(auto f : nowFolder->subFolders){
        recursiveDumpFolderContent(f, d.absolutePath());
    }
}

inline void processFolders(Folder* nowFolder, QList<Resource<ResourceData>>& allFiles, QByteArray& outBuffer, bool skipFolder = false){
    qDebug() << "Entering folder" << nowFolder->name;
    if(!skipFolder)
        outBuffer.append(GetNameAsByteArray(nowFolder->name + "/", 0));
    for(auto file : nowFolder->files){
        qDebug() << file.name;
        allFiles << file;
        outBuffer.append(GetNameAsByteArray(file.name, file.data.size));
    }
    qDebug() << "Leaving folder " << nowFolder->name << " to parent folder";
    for(auto f : nowFolder->subFolders){
        processFolders(f, allFiles, outBuffer);
    }
    if(!skipFolder)
        outBuffer.append(GetNameAsByteArray("..", 0));
}

/* Well here we just dump the "hierarchy" into a Zap file */
static void SAVE_ZAP_FILE(InMemZAP& file, QFileInfo dest){
    QFile f;
    f.setFileName(dest.absoluteFilePath());
    f.open(QIODevice::WriteOnly);

    // Set the zap header
    ZAP_HEADER header;
    memset(&header, 0x00, sizeof(ZAP_HEADER));
    header.entries = (long)file.getFileCount();
    memcpy(header.ZAP_ID, "ZAP", 4);
    memcpy(header.line_break_padding, "\r\n", 2);
    header.block_size = file.block_size;

    // We write everything into a byte Buffer first, because we might need to change something in them later on here.
    QByteArray outputBuffer;
    QByteArray headerBuffer;

    QByteArray nameBuffer;

    Folder* rootFolder = file.rootFolder;
    Folder* nowFolder = rootFolder;

    QList<Resource<ResourceData>> allFiles;

    QList<Folder*> processed;
    // create a list of all files that end up in the ZapFile on disk. (But in the correct order)
    processFolders(nowFolder, allFiles, nameBuffer, true);
    // Some weird checksum that i found out about with my notepad skills.
    header.fileOffsetChecksum = nameBuffer.length() - header.entries * 0x08;

    // fill missing bytes for block completeness
    nameBuffer.append(file.block_size - (sizeof(ZAP_HEADER) + nameBuffer.length()) % file.block_size, 0x00);

    headerBuffer.append((const char*)&header, sizeof(ZAP_HEADER));

    // combine all our files into a giant byteBuffer / byteArray
    QMap<QString, size_t> file_offset;
    QByteArray files_data;
    Q_FOREACH(auto fileContainer, allFiles){
        auto fileLoaded = fileContainer.data;
        int len = fileLoaded.size;
        const int needed_len = file.block_size - fileLoaded.size % file.block_size; //((files_data.size() + file.block_size) / file.block_size) * file.block_size - files_data.length();
        files_data.append(fileLoaded.data, fileLoaded.length());
        if(needed_len)
            files_data.append(needed_len, 0x00);
        file_offset.insert(fileContainer.name, files_data.length());
    }
    f.write(headerBuffer);
    f.write(nameBuffer);
    f.write(files_data);
}

}
#endif // ZAP_FILE_H
