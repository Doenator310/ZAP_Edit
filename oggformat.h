#include <QObject>
#pragma once
#ifndef OGGFORMAT_H
#define OGGFORMAT_H
#pragma pack(push,1)

namespace OGG{
    enum HEADER_TYPE : quint8{
        CONTINUATION = 0x01,
        BEG_OF_STREAM = 0x02,
        END_OF_STREAM = 0x04
    };
    // this helped https://www.xiph.org/ogg/doc/rfc3533.txt [Page 8]
    struct Header{
        char capture_pattern[4];
        quint8 version;
        HEADER_TYPE headerType;
        quint64 granule_position;
        quint32 bitstream_serial_number;
        quint32 page_sequence_number;
        quint32 checksum;
        quint8 segments;
        quint8 segment_table_start;
        inline quint32 getSegmentTableOffset(){
            quint32 total_offset = 0;
            quint8* page_len = &segment_table_start;
            for(int i = 0; i < segments; i++){
                total_offset += page_len[i];
            }
            return total_offset;
        }

        inline bool hasValidCapturePattern(){
            return memcmp(capture_pattern, "OggS", 4)==0;
        }

        inline Header* getNextHeader(){
            quint64 offset = segments + getSegmentTableOffset();
            return (Header*)(((char*)&segment_table_start) + offset);
        }
    };

    inline size_t get_size_of_ogg(const char* ogg_start){
        size_t size = 0;
        OGG::Header* ogg_header = (OGG::Header*)ogg_start;
        OGG::Header* last_ogg_header = nullptr;
        if(ogg_header->hasValidCapturePattern() == false){
            return 0;
        }
        do{
            last_ogg_header = ogg_header;
            OGG::Header* next_header = ogg_header->getNextHeader();
            if(next_header->hasValidCapturePattern() == false){
                break;
            }
            ogg_header = next_header;
            int x = 3;
            x++;
        }while(ogg_header->headerType != OGG::END_OF_STREAM);
        OGG::Header* end_of_ogg = ogg_header->getNextHeader();
        size = (quint64)end_of_ogg - (quint64)ogg_start;
        return size;
    }


    // This function ensures that the ogg audio does have the required End of Stream Header Type at the end!
    inline void prepareExternalOGGStream(char* ogg_start){
        size_t size = 0;
        OGG::Header* ogg_header = (OGG::Header*)ogg_start;
        OGG::Header* last_ogg_header = nullptr;
        if(ogg_header->hasValidCapturePattern() == false){
            return;
        }
        do{
            last_ogg_header = ogg_header;
            OGG::Header* next_header = ogg_header->getNextHeader();
            if(next_header->hasValidCapturePattern() == false){
                break;
            }
            ogg_header = next_header;
            int x = 3;
            x++;
        }while(ogg_header->headerType != OGG::END_OF_STREAM);
        ogg_header->headerType = HEADER_TYPE::END_OF_STREAM;
    }

    inline bool is_ogg(const char* ogg_start, int bufferSize){
        if(bufferSize < sizeof(OGG::Header))return false; // Not even big enough for a ogg file lol

        OGG::Header* ogg_header = (OGG::Header*)ogg_start;

        return ogg_header->hasValidCapturePattern();
    }




}
#pragma pack(pop)
#endif // OGGFORMAT_H
