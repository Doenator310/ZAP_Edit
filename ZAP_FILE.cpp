#include "ZAP_FILE.h"
using namespace ZAP;
QList<Folder*> Folder::allFoldersCreated = QList<Folder*>();


void recursiveCleanUp(Folder* f){
    for(auto file : f->files){
        file.data.free();
    }
    f->files.clear();
    for(auto subFolder : f->subFolders){
        recursiveCleanUp(subFolder);
    }
    delete f;
}


void InMemZAP::cleanUp()
{
    if(this->rootFolder){
        recursiveCleanUp(this->rootFolder);
        this->rootFolder = nullptr;
    }
}

bool InMemZAP::isValid(){
    return this->rootFolder != nullptr;
}

QStringList Folder::getAllFolderNames()
{
    QStringList list;
    for(auto el :  this->subFolders)
        list << el->name;
    return list;
}

QStringList Folder::getAllFileNames()
{
    QStringList list;
    for(auto el :  this->files)
        list << el.name;
    return list;
}

void Folder::DeleteFolderAndContent(Folder *folderToDelete)
{
    recursiveCleanUp(folderToDelete);
}


SpecialFeatureResult ResourceData::analyze()
{
    SpecialFeatureResult result;
    if(this->data == nullptr || this->size < 1024){
        return result; // skip
    }

    if(OGG::is_ogg(this->data, this->size)){
        result.hasOGGAudio = true;
        auto embedded_audio_tracks = GetAllOGGFilesInDataArea(this->data, nullptr);
        result.trackCount = embedded_audio_tracks.size();
    }

    return result;
}
































