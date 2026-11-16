#include "pending-measurement-store.h"

#include <LittleFS.h>


namespace
{
    constexpr size_t
        MAX_FILESYSTEM_USAGE_PERCENT =
            70;
}


bool PendingMeasurementStore::begin()
{
    Serial.println();
    Serial.println(
        "[Queue] Inicializando LittleFS..."
    );

    if(!LittleFS.begin(true))
    {
        Serial.println(
            "[Queue] Falha ao montar LittleFS."
        );

        ready = false;

        return false;
    }

    ready = true;

    recoverTemporaryFile();

    /*
    * Garante que o arquivo principal da fila
    * exista desde a inicialização.
    */
    if(!LittleFS.exists(queueFilePath))
    {
        File queueFile =
            LittleFS.open(
                queueFilePath,
                FILE_WRITE
            );

        if(!queueFile)
        {
            Serial.println(
                "[Queue] Não foi possível criar o arquivo da fila."
            );

            ready = false;

            return false;
        }

        queueFile.close();

        Serial.println(
            "[Queue] Arquivo da fila criado."
        );
    }

    const size_t totalBytes =
        LittleFS.totalBytes();

    const size_t usedBytes =
        LittleFS.usedBytes();


    maxQueueBytes =
        (
            totalBytes
            * MAX_FILESYSTEM_USAGE_PERCENT
        ) / 100;


    Serial.print(
        "[Queue] LittleFS total: "
    );

    Serial.print(totalBytes);

    Serial.println(" bytes");


    Serial.print(
        "[Queue] LittleFS usado: "
    );

    Serial.print(usedBytes);

    Serial.println(" bytes");


    Serial.print(
        "[Queue] Limite da fila: "
    );

    Serial.print(maxQueueBytes);

    Serial.println(" bytes");


    Serial.print(
        "[Queue] Medições pendentes: "
    );

    Serial.println(
        count()
    );


    return true;
}


void PendingMeasurementStore::
recoverTemporaryFile()
{
    File tempFile =
        LittleFS.open(
            tempFilePath,
            FILE_READ
        );

    if(!tempFile)
    {
        return;
    }

    tempFile.close();


    /*
     * Se existe fila principal,
     * ela continua sendo a fonte
     * válida. O temporário veio de
     * uma operação interrompida antes
     * da substituição da fila.
     */
    if(
        LittleFS.exists(
            queueFilePath
        )
    )
    {
        LittleFS.remove(
            tempFilePath
        );

        return;
    }


    /*
     * Se a fila principal sumiu mas
     * o temporário existe, provavelmente
     * faltou energia entre remove()
     * e rename().
     */
    if(
        LittleFS.rename(
            tempFilePath,
            queueFilePath
        )
    )
    {
        Serial.println(
            "[Queue] Fila temporária recuperada."
        );
    }
}


bool PendingMeasurementStore::enqueue(
    const std::string& payload
)
{
    if(!ready)
    {
        Serial.println(
            "[Queue] LittleFS não disponível."
        );

        return false;
    }


    if(payload.empty())
    {
        return false;
    }


    const size_t requiredBytes = payload.size() + 2; // println grava CRLF.
    if (requiredBytes > maxQueueBytes) return false;


    /*
     * Se atingirmos o limite reservado
     * para a fila, removemos as medições
     * mais antigas para preservar as
     * mais recentes.
     */
    while(
        LittleFS.exists(
            queueFilePath
        )
    )
    {
        File currentFile =
            LittleFS.open(
                queueFilePath,
                FILE_READ
            );


        if(!currentFile)
        {
            return false;
        }


        const size_t currentSize =
            currentFile.size();


        currentFile.close();


        if(
            currentSize
            + requiredBytes
            <= maxQueueBytes
            && LittleFS.usedBytes() + requiredBytes <= maxQueueBytes
        )
        {
            break;
        }


        Serial.println(
            "[Queue] Fila cheia. Removendo medição mais antiga."
        );


        if(!removeFirst())
        {
            return false;
        }
    }


    File file =
        LittleFS.open(
            queueFilePath,
            FILE_APPEND
        );


    if(!file)
    {
        Serial.println(
            "[Queue] Não foi possível abrir a fila."
        );

        return false;
    }


    const size_t written =
        file.println(
            payload.c_str()
        );


    file.flush();
    file.close();


    if(written != requiredBytes)
    {
        Serial.println(
            "[Queue] Falha ao gravar medição."
        );

        return false;
    }


    Serial.print(
        "[Queue] Medição armazenada. Pendentes: "
    );

    Serial.println(
        count()
    );


    return true;
}


bool PendingMeasurementStore::peek(
    std::string& payload
)
{
    payload.clear();


    if(
        !ready
        || !LittleFS.exists(
            queueFilePath
        )
    )
    {
        return false;
    }


    File file =
        LittleFS.open(
            queueFilePath,
            FILE_READ
        );


    if(!file)
    {
        return false;
    }


    while(file.available())
    {
        String line =
            file.readStringUntil(
                '\n'
            );


        if(line.endsWith("\r"))
        {
            line.remove(
                line.length() - 1
            );
        }


        if(line.length() == 0)
        {
            continue;
        }


        payload =
            std::string(
                line.c_str()
            );


        file.close();

        return true;
    }


    file.close();

    return false;
}


bool PendingMeasurementStore::removeFirst()
{
    if(
        !ready
        || !LittleFS.exists(
            queueFilePath
        )
    )
    {
        return false;
    }


    File source =
        LittleFS.open(
            queueFilePath,
            FILE_READ
        );


    if(!source)
    {
        return false;
    }


    File destination =
        LittleFS.open(
            tempFilePath,
            FILE_WRITE
        );


    if(!destination)
    {
        source.close();

        return false;
    }


    bool removedFirst =
        false;

    bool hasRemaining =
        false;


    while(source.available())
    {
        String line =
            source.readStringUntil(
                '\n'
            );


        if(line.endsWith("\r"))
        {
            line.remove(
                line.length() - 1
            );
        }


        if(line.length() == 0)
        {
            continue;
        }


        if(!removedFirst)
        {
            removedFirst = true;

            continue;
        }


        const size_t copied = destination.println(line);
        if (copied != line.length() + 2)
        {
            source.close();
            destination.close();
            LittleFS.remove(tempFilePath);
            Serial.println("[Queue] Copia incompleta; fila original preservada.");
            return false;
        }

        hasRemaining =
            true;
    }


    destination.flush();

    source.close();
    destination.close();


    if(!removedFirst)
    {
        LittleFS.remove(
            tempFilePath
        );

        return false;
    }


    if(
        !LittleFS.remove(
            queueFilePath
        )
    )
    {
        LittleFS.remove(
            tempFilePath
        );

        return false;
    }


    if(hasRemaining)
    {
        if(
            !LittleFS.rename(
                tempFilePath,
                queueFilePath
            )
        )
        {
            Serial.println(
                "[Queue] Falha ao substituir arquivo da fila."
            );

            return false;
        }
    }
    else
    {
        /*
        * A última medição foi removida.
        * Mantemos o arquivo principal
        * existente, mas vazio.
        */
        LittleFS.remove(
            tempFilePath
        );

        File emptyQueue =
            LittleFS.open(
                queueFilePath,
                FILE_WRITE
            );

        if(!emptyQueue)
        {
            Serial.println(
                "[Queue] Falha ao recriar arquivo vazio da fila."
            );

            return false;
        }

        emptyQueue.close();
    }

    return true;
}


size_t PendingMeasurementStore::count()
{
    if(
        !ready
        || !LittleFS.exists(
            queueFilePath
        )
    )
    {
        return 0;
    }


    File file =
        LittleFS.open(
            queueFilePath,
            FILE_READ
        );


    if(!file)
    {
        return 0;
    }


    size_t total =
        0;


    while(file.available())
    {
        String line =
            file.readStringUntil(
                '\n'
            );


        if(line.length() > 1)
        {
            total++;
        }
    }


    file.close();


    return total;
}


bool PendingMeasurementStore::
isReady() const
{
    return ready;
}