#include <cassert>
#include <iostream>
#include <LittleFS.h>
#include "storage/pending-measurement-store.h"
SerialStub Serial;
LittleFSStub LittleFS;
std::map<std::string,std::string> diskFiles;
bool mountFails=false,partialWrite=false;
int main() {
    PendingMeasurementStore pending;
    PendingMeasurementStore undated("/undated.jsonl","/undated.tmp");
    assert(pending.begin() && undated.begin());
    assert(pending.enqueue("{\"timestamp\":1794850000}"));
    assert(undated.enqueue("{\"timestamp\":null,\"boot_id\":123}"));
    assert(pending.count()==1 && undated.count()==1);
    std::string payload;
    assert(pending.peek(payload) && payload.find("1794850000")!=std::string::npos);
    assert(undated.peek(payload) && payload.find("null")!=std::string::npos);
    assert(pending.removeFirst() && pending.count()==0 && undated.count()==1);
    // Cada fila recupera apenas seu temporário após interrupção de energia.
    diskFiles.erase("/undated.jsonl"); diskFiles["/undated.tmp"]="local-record\n";
    PendingMeasurementStore recovered("/undated.jsonl","/undated.tmp");
    assert(recovered.begin() && recovered.peek(payload) && payload=="local-record");
    assert(pending.count()==0 && !diskFiles.count("/pending.tmp"));
    const std::string local=diskFiles.at("/undated.jsonl");
    assert(!pending.enqueue(std::string(8000,'x')));
    assert(diskFiles.at("/undated.jsonl")==local);
    // Ao encher a fila de upload, a fila sem data não é removida ou enviada.
    for(int i=0;i<100;++i) assert(pending.enqueue(std::string(100,'p')));
    assert(LittleFS.usedBytes()<=7000 && pending.count()<100);
    assert(diskFiles.at("/undated.jsonl")==local);
    // Espaço ocupado pela outra fila gera erro, em vez de apagar esse arquivo.
    while(pending.removeFirst()) {}
    diskFiles["/undated.jsonl"]=std::string(6999,'u');
    assert(!pending.enqueue("new"));
    assert(diskFiles.at("/undated.jsonl").size()==6999);
    // Falha ao compactar não deve substituir a fila original por cópia parcial.
    diskFiles["/undated.jsonl"]="";
    assert(pending.enqueue("first") && pending.enqueue("second"));
    const std::string original=diskFiles.at("/pending.jsonl");
    partialWrite=true; assert(!pending.removeFirst()); partialWrite=false;
    assert(diskFiles.at("/pending.jsonl")==original && !diskFiles.count("/pending.tmp"));
    while(pending.removeFirst()) {}
    // Uma escrita parcial não pode ser reportada como uma coleta salva.
    diskFiles["/undated.jsonl"]=""; partialWrite=true;
    assert(!pending.enqueue("record")); partialWrite=false;
    mountFails=true; PendingMeasurementStore failed; assert(!failed.begin());
    assert(!failed.enqueue("record"));
    std::cout << "Independent local queues, recovery, shared space and write failure checks passed\n";
}
