# Firmware — CASA

Firmware das estações de monitoramento ambiental do projeto **CASA — Central de Acompanhamento de Saúde e Ambiente**.

O firmware é executado em um **ESP32-S3** e é responsável pela aquisição dos sensores ambientais, configuração da estação via Bluetooth Low Energy (BLE), conexão Wi-Fi, sincronização de horário, comunicação com a API do CASA e armazenamento local temporário das medições quando o servidor não está disponível.

## Visão geral

Cada estação realiza medições ambientais periodicamente e envia os dados para o servidor central do CASA.

O fluxo principal é:

```text
Sensores
   ↓
ESP32-S3
   ↓
Timestamp sincronizado via NTP
   ↓
Medição
   ↓
Tentativa de envio HTTP
   │
   ├── Sucesso → servidor CASA
   │
   └── Falha → fila persistente no LittleFS
                    ↓
              reenvio automático
                    ↓
              servidor disponível
```

A configuração inicial da estação é realizada pelo aplicativo do projeto através de BLE.

O aplicativo permite configurar:

* identificação da estação;
* credenciais Wi-Fi;
* endereço do servidor;
* provisionamento da credencial utilizada pela estação para autenticação na API.

Depois de configurada, a estação opera de forma autônoma.

---

## Hardware

O firmware foi desenvolvido para o **ESP32-S3-WROOM-1**.

### Sensores utilizados

Atualmente a estação utiliza:

| Sensor | Grandeza                                                   |
| ------ | ---------------------------------------------------------- |
| SPS30  | Material particulado e concentração numérica de partículas |
| AHT20  | Temperatura e umidade relativa                             |
| BMP280 | Pressão atmosférica                                        |
| BH1750 | Iluminância                                                |

As grandezas de material particulado incluem:

* PM1.0;
* PM2.5;
* PM4.0;
* PM10.

Também são obtidas concentrações numéricas para diferentes faixas de tamanho de partículas e o tamanho típico das partículas.

### GNSS/GPS

O firmware utiliza um NEO-6M por UART com TinyGPSPlus, Serial1 a 9600 baud. O TX do GPS conecta ao GPIO 18 (RX do ESP32); o RX do GPS conecta ao GPIO 17 (TX do ESP32).

A implementação foi projetada para ser **opcional**: uma estação sem módulo GPS conectado deve continuar funcionando normalmente.

O `GNSSManager` realiza a leitura de forma não bloqueante. A ausência de um fix válido não interrompe:

* leitura dos sensores;
* BLE;
* conexão Wi-Fi;
* sincronização NTP;
* armazenamento local;
* envio das medições.

A leitura do GPS continua no loop. O `LocationManager` aceita uma posição nova
na inicialização e a cada 24 horas após a última obtenção bem-sucedida. Cada busca
dura até cinco minutos; se falhar, uma nova busca começa 15 minutos após o fim
da tentativa. O relógio monotônico de 64 bits evita mudanças de agenda por NTP
ou pelo estouro de `millis()`. O módulo permanece alimentado.

A posição aceita precisa ter sido recebida após o começo da busca e ter no máximo
cinco segundos de idade. Seu horário vem do GPS em UTC; se indisponível, usa o relógio
sincronizado por NTP descontando a idade da leitura. Sem horário válido, a busca
continua. Isso permite obter e persistir posição na inicialização sem internet
quando o GPS já fornece data e hora válidas.

A última posição, seu horário e o envio pendente ficam em um único registro NVS.
São preservados após reinicialização, e uma nova busca sempre é iniciada ao ligar.
Sem posição, medições enviam latitude/longitude como `null`; com posição anterior,
elas continuam enviando a última conhecida. Perder o sinal não apaga essa posição.
Uma troca de `device_id` descarta a localização de outro cadastro.

O envio usa `PATCH /api/devices/location` e o token já provisionado. A URL é derivada
da URL existente de medições, que deve terminar em `/measurements` (barra final
opcional). Não é necessário mudar o protocolo BLE. Tentativas HTTP de localização
ocorrem no máximo uma vez por minuto enquanto há conexão. Falhas preservam o registro
pendente; uma nova posição substitui a anterior pendente. O registro só é confirmado
após HTTP 2xx. Se houver falha de armazenamento, o firmware tenta novamente antes
de enviar. Essa persistência é independente da fila ambiental em LittleFS.

O backend deve ser atualizado antes do firmware. A recepção física do NEO-6M,
a qualidade do sinal e os testes de desligamento/religamento precisam ser validados
na estação real. Os envios HTTP continuam síncronos; as requisições de localização têm timeouts
de conexão e leitura de três segundos; a janela de busca do GPS não contém espera bloqueante.

---

## Configuração de pinos

A pinagem do hardware está centralizada em:

```text
src/config/config.h
```

Isso permite utilizar o mesmo firmware em diferentes revisões das estações alterando apenas as definições de hardware.

Entre os pinos configuráveis estão:

```text
I2C SDA
I2C SCL

GNSS RX
GNSS TX

LED1 RED
LED1 BLUE

LED2 RED
LED2 BLUE

LED3 RED
LED3 BLUE
```

Os pinos dos LEDs estão reservados para implementação futura da sinalização visual da estação.

> Algumas unidades de protótipo possuem pinagens diferentes. Sempre confira `config.h` antes de gravar o firmware em uma nova estação.

---

## Aquisição de dados

Por padrão, uma nova medição é realizada a cada:

```text
5 minutos
```

O intervalo é definido em `src/config/config.h`.

Cada medição contém dados como:

```text
device_id
timestamp

temperature
humidity
pressure
light

pm1
pm25
pm4
pm10

nc05
nc10
nc25
nc40
nc100

typical_particle_size
```

O timestamp é associado à medição no momento da aquisição e é preservado caso seja necessário armazenar e reenviar a medição posteriormente.

---

## Sincronização de horário

O ESP32 utiliza sincronização **NTP** para obter a referência de tempo.

O horário correto é importante porque o servidor utiliza o timestamp gerado pela estação para posicionar cada medição na série temporal.

Caso uma medição precise permanecer armazenada localmente durante uma indisponibilidade de rede ou servidor, seu timestamp original é mantido.

---

## Bluetooth Low Energy

BLE é utilizado para comunicação entre a estação e o aplicativo de configuração.

Através dessa comunicação é possível realizar operações como:

```text
get_status
save_identity
configure_wifi
```

Entre as informações disponibilizadas ao aplicativo estão:

* estado da conexão;
* hardware ID;
* identificação da estação;
* rede Wi-Fi;
* endereço IP;
* estado da credencial de autenticação.

A comunicação utiliza características BLE para escrita e notificações entre aplicativo e ESP32.

---

## Identificação da estação

Cada ESP32 possui um identificador de hardware utilizado durante o primeiro cadastro.

Durante o provisionamento, o servidor associa esse hardware a um identificador interno no formato:

```text
CASA-000001
CASA-000002
CASA-000003
...
```

Esse identificador é utilizado posteriormente nas comunicações com a API.

Para o usuário final, o aplicativo trabalha principalmente com o **nome atribuído à estação**, deixando o identificador `CASA-XXXXXX` como informação interna do sistema.

---

## Autenticação

As estações utilizam uma credencial individual para autenticação junto à API.

Durante o provisionamento:

```text
Aplicativo
    ↓
Servidor CASA
    ↓
device_id + credencial
    ↓
Aplicativo
    ↓ BLE
ESP32
```

A credencial é armazenada persistentemente no ESP32.

Nas requisições posteriores, ela é utilizada para que o backend consiga verificar se a estação está autorizada a enviar dados.

A credencial não deve ser armazenada em código-fonte ou enviada ao repositório Git.

---

## Wi-Fi

As credenciais Wi-Fi são configuradas através do aplicativo utilizando BLE.

Depois da configuração, o ESP32 tenta manter a conexão automaticamente.

Caso a conexão seja perdida, o firmware realiza novas tentativas sem interromper permanentemente o restante da aplicação.

Isso permite que situações como:

* reinicialização do roteador;
* perda temporária de sinal;
* indisponibilidade da Internet;

sejam tratadas sem necessidade de reiniciar manualmente a estação.

---

## Comunicação com o servidor

As medições são enviadas para a API do CASA através de HTTP/HTTPS, dependendo do ambiente utilizado.

Durante o desenvolvimento local, um endereço semelhante a:

```text
http://192.168.x.x/api/measurements
```

pode ser utilizado.

Na implantação em produção, o endereço local será substituído pelo endereço definitivo do servidor.

Uma resposta de sucesso da API confirma que a medição foi armazenada pelo backend.

---

## Armazenamento offline

O firmware possui uma fila persistente utilizando **LittleFS**.

Se uma medição não puder ser enviada ao servidor, ela é armazenada localmente em:

```text
/pending.jsonl
```

Cada entrada preserva os dados originais da medição, incluindo seu timestamp.

Quando a comunicação volta a funcionar, o firmware começa a reenviar automaticamente as medições pendentes.

O comportamento esperado é:

```text
Medição
   ↓
POST
   ↓
Falhou?
   │
   ├── Não → finaliza
   │
   └── Sim
        ↓
      LittleFS
        ↓
      fila
        ↓
servidor volta
        ↓
reenvio
        ↓
resposta de sucesso
        ↓
remove da fila
```

A medição somente é removida da fila depois da confirmação de que foi aceita pelo servidor.

### Persistência

A fila foi projetada para sobreviver a:

* perda de conexão Wi-Fi;
* indisponibilidade do servidor;
* reinicialização do ESP32;
* reset físico;
* desligamento da alimentação.

Assim, uma interrupção temporária não deve causar perda das medições já armazenadas.

---

## Estrutura do firmware

A estrutura geral do projeto segue a separação por responsabilidades:

```text
src/
├── config/
│   └── config.h
│
├── sensors/
│   ├── sensor-manager.*
│   └── gnss-manager.*
│
├── storage/
│   └── pending-measurement-store.*
│
├── ...
│
└── main.cpp
```

Além desses componentes, o firmware possui módulos responsáveis por:

* BLE;
* Wi-Fi;
* comunicação HTTP;
* armazenamento de configurações;
* gerenciamento de horário;
* autenticação;
* aquisição dos sensores.

Essa separação evita concentrar toda a lógica em `main.cpp` e facilita manutenção e expansão do sistema.

---

## Ambiente de desenvolvimento

O projeto utiliza **PlatformIO**.

O desenvolvimento pode ser realizado pelo Visual Studio Code com a extensão PlatformIO instalada.

### Clonar o repositório

```bash
git clone <URL_DO_REPOSITORIO>
cd ESP32-IT-Estacao_Meteorologica
```

### Compilar

Pelo terminal do PlatformIO:

```bash
pio run
```

### Gravar no ESP32

Com a placa conectada via USB:

```bash
pio run --target upload
```

### Monitor serial

```bash
pio device monitor
```

A velocidade do monitor deve corresponder à configurada no firmware.

Também é possível realizar compilação, upload e monitoramento através dos botões da extensão PlatformIO no VS Code.

---

## Dependências

As bibliotecas utilizadas pelo firmware são declaradas no:

```text
platformio.ini
```

Entre elas estão bibliotecas relacionadas a:

* BLE;
* JSON;
* sensores ambientais;
* GNSS/GPS;
* comunicação com o ESP32.

Ao abrir o projeto pelo PlatformIO, as dependências declaradas são instaladas automaticamente.

---

## Configuração de uma nova estação

O fluxo esperado para uma nova unidade é:

```text
1. Gravar o firmware
        ↓
2. Ligar a estação
        ↓
3. Abrir o aplicativo CASA
        ↓
4. Encontrar a estação via BLE
        ↓
5. Dar um nome à estação
        ↓
6. Provisionar a estação
        ↓
7. Configurar Wi-Fi
        ↓
8. ESP32 conecta à rede
        ↓
9. Sincroniza horário
        ↓
10. Inicia aquisição e envio
```

Depois disso, a estação deve funcionar autonomamente.

---

## Logs

O firmware possui mensagens no monitor serial para auxiliar durante desenvolvimento e diagnóstico.

Alguns prefixos utilizados são:

```text
[Sensors]
[Device]
[Station]
[Time]
[HTTP]
[Queue]
[GNSS]
```

Exemplo de envio bem-sucedido:

```text
[HTTP] Enviando dados...
[HTTP] Código de resposta: 201
[HTTP] Dados enviados com sucesso.
```

Exemplo de servidor indisponível:

```text
[HTTP] Falha no POST: connection refused
[Queue] Medição armazenada.
```

Posteriormente:

```text
[Queue] Reenviando medição pendente.
[HTTP] Código de resposta: 201
[Queue] Medição pendente enviada.
```

---

## Testes já realizados

Durante o desenvolvimento foram validados cenários como:

* leitura simultânea dos sensores;
* configuração via BLE;
* configuração e troca de rede Wi-Fi;
* provisionamento da estação;
* autenticação das requisições;
* envio periódico das medições;
* reconexão Wi-Fi;
* indisponibilidade do backend;
* armazenamento de múltiplas medições no LittleFS;
* reinicialização do ESP32 com medições pendentes;
* desligamento e religamento da alimentação;
* reenvio automático após retorno do servidor;
* preservação dos timestamps originais;
* visualização posterior das medições no frontend.

O suporte GNSS está em desenvolvimento e ainda requer validação com o módulo físico.

---

## Próximas etapas

Entre as funcionalidades previstas estão:

* validar o módulo GNSS/GPS fisicamente;
* validar a atualização diária e a recuperação da localização em testes de campo;
* implementar sinalização através dos LEDs;
* padronizar a pinagem entre as unidades;
* migrar a comunicação para a infraestrutura definitiva em nuvem;
* utilizar HTTPS em produção;
* validar o funcionamento simultâneo das estações piloto;
* realizar testes de longa duração.

---

## Projeto CASA

O firmware faz parte de uma arquitetura maior composta por:

```text
┌─────────────────────┐
│   Estação ESP32     │
│ Sensores + Firmware │
└──────────┬──────────┘
           │
       HTTP/HTTPS
           │
           ▼
┌─────────────────────┐
│     Backend API     │
│       FastAPI       │
└──────────┬──────────┘
           │
           ▼
┌─────────────────────┐
│     PostgreSQL      │
└──────────┬──────────┘
           │
           ▼
┌─────────────────────┐
│   Interface Web     │
│       React         │
└─────────────────────┘

        Aplicativo
       React Native
            │
            │ BLE
            ▼
      Estação ESP32
```

A arquitetura foi desenvolvida para permitir que múltiplas estações enviem dados para um servidor central, onde as informações podem ser armazenadas, consultadas e visualizadas.

---

## Status atual

O firmware encontra-se em fase de preparação para a **validação inicial com seis estações**.

As funções essenciais de aquisição, configuração, comunicação, autenticação, reconexão e persistência offline já estão implementadas.

As funcionalidades relacionadas ao GPS, sinalização por LEDs e demais melhorias de hardware serão incorporadas progressivamente durante as próximas etapas de desenvolvimento.


## Testes da agenda de localização

Sem placa, com um compilador C++:

```bash
g++ -std=c++11 -Wall -Wextra -Werror test/location_schedule_test.cpp -o /tmp/location-tests
/tmp/location-tests
```

Cobrem a busca inicial, rejeição de posição antiga, 24 horas, timeout, retentativa,
reinicialização da agenda, uptime além de 32 bits e conversão da data GPS para UTC.
A compilação completa é feita com `pio run`.


O teste de integração no host executa o `LocationManager` real com ArduinoJson
real e adaptadores de NVS, GPS, relógio e rede em `test/host`. Após instalar as
bibliotecas com `pio pkg install`, executar:

```bash
g++ -std=c++11 -Wall -Wextra -Werror \
  -Itest/host -Isrc -I.pio/libdeps/esp32-s3-devkitm-1/ArduinoJson/src \
  test/location_manager_test.cpp src/location/location-manager.cpp \
  -o /tmp/location-manager-tests
/tmp/location-manager-tests
```

Cobre obtenção sem NTP, recuperação após reinicialização, precisão das coordenadas,
retentativa de envio com horário original, confirmação de persistência, atualização
diária, descarte de posição de outro cadastro e falha de gravação antes do envio.
Esses adaptadores não são incluídos no firmware e não substituem os testes na placa.
