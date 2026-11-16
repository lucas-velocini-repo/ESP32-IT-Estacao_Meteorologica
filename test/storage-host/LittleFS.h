#pragma once
#include "Arduino.h"
#include <map>
#include <algorithm>
#include <cstring>
constexpr const char* FILE_READ="r";
constexpr const char* FILE_WRITE="w";
constexpr const char* FILE_APPEND="a";
extern std::map<std::string,std::string> diskFiles;
extern bool mountFails,partialWrite;
struct File {
    std::string name;
    size_t position=0;
    bool valid=false;
    File()=default;
    explicit File(std::string path):name(path),valid(true) {}
    explicit operator bool() const { return valid; }
    size_t size() { return diskFiles[name].size(); }
    bool available() { return valid && position<size(); }
    String readStringUntil(char delimiter) {
        const auto& data=diskFiles[name];
        const size_t end=data.find(delimiter,position);
        const size_t count=(end==std::string::npos?data.size():end)-position;
        String value(data.substr(position,count));
        position+=(count+(end==std::string::npos?0:1));
        return value;
    }
    size_t println(const char* text) {
        std::string line=std::string(text)+"\r\n";
        if(partialWrite) line.resize(line.size()/2);
        diskFiles[name]+=line;
        return line.size();
    }
    size_t println(const String& text) { return println(text.c_str()); }
    void flush() {}
    void close() { valid=false; }
};
struct LittleFSStub {
    bool begin(bool) { return !mountFails; }
    bool exists(const char* path) { return diskFiles.count(path)>0; }
    File open(const char* path,const char* mode) {
        if(std::strcmp(mode,FILE_READ)==0 && !exists(path)) return File();
        if(std::strcmp(mode,FILE_WRITE)==0) diskFiles[path]="";
        if(!exists(path)) diskFiles[path]="";
        return File(path);
    }
    size_t totalBytes() { return 10000; }
    size_t usedBytes() {
        size_t bytes=0;
        for(const auto& file:diskFiles) bytes+=file.second.size();
        return bytes;
    }
    bool remove(const char* path) { return diskFiles.erase(path)>0; }
    bool rename(const char* from,const char* to) {
        if(!exists(from)) return false;
        diskFiles[to]=diskFiles[from]; diskFiles.erase(from); return true;
    }
};
extern LittleFSStub LittleFS;
