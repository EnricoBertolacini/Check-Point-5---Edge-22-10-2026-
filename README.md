# 🍷 Vinheria Agnello – Monitoramento de Adega com ESP32 e MQTT

**Check Point 05 – Edge Computing – FIAP**

Sistema de IoT que mede **temperatura, umidade e luminosidade** de uma adega com um ESP32, envia os dados a um servidor via **MQTT** e permite **ligar e desligar um LED remotamente** pelo celular (app MyMQTT).

---

## 👥 Integrantes do grupo

| Nome completo | RM |
|---|---|
| Enrico Bertolacini| RM 570999 |
| Matheus Sá Teles | RM 571719|
| Julia Lima da Silva  | RM 569203 |
| Pedro Antonio Borges | RM 572549|
| Guilherme Alvejan  | RM 570835|

- **Turma:** 1ESR
- **Professor(a):** Lucas Demetrius
- **Número do dispositivo do grupo:** `device0XX`

---

## 📑 Índice

1. [O que o projeto faz](#-o-que-o-projeto-faz)
2. [Conceitos básicos (para quem nunca viu MQTT)](#-conceitos-básicos-para-quem-nunca-viu-mqtt)
3. [Materiais necessários](#-materiais-necessários)
4. [Montagem do circuito](#-montagem-do-circuito)
5. [Preparação do computador](#-preparação-do-computador)
6. [Configuração do código](#-configuração-do-código)
7. [Gravando o código no ESP32](#-gravando-o-código-no-esp32)
8. [Tópicos MQTT utilizados](#-tópicos-mqtt-utilizados)
9. [Usando o MyMQTT (leitura e escrita)](#-usando-o-mymqtt-leitura-e-escrita)
10. [Como o código funciona](#-como-o-código-funciona)
11. [Testando sem o hardware (Wokwi)](#-testando-sem-o-hardware-wokwi)
12. [Demonstração (Hands-On)](#-demonstração-hands-on)
13. [Solução de problemas](#-solução-de-problemas)
14. [Estrutura do repositório](#-estrutura-do-repositório)

---

## 🎯 O que o projeto faz

A Vinheria Agnello precisa conservar seus vinhos em boas condições. Calor excessivo, umidade inadequada e muita luz prejudicam a bebida. Este projeto monitora esses três fatores continuamente.

| Função | Como funciona |
|---|---|
| **Leitura de temperatura e umidade** | Sensor DHT11 ligado ao ESP32 |
| **Leitura de luminosidade** | Sensor LDR ligado ao ESP32 |
| **Envio dos dados** | O ESP32 publica os valores no servidor MQTT a cada 5 segundos |
| **Leitura no celular** | O app MyMQTT assina os tópicos e mostra os valores |
| **Escrita (comando)** | O app MyMQTT envia um comando e o ESP32 liga ou desliga o LED |
| **Estado do LED** | O ESP32 informa ao servidor se o LED está ligado (`s\|on`) ou desligado (`s\|off`) |

O código foi baseado no exemplo da aula (autores: Fábio Henrique Cabrini e Lucas Demetrius Augusto), com a inclusão do sensor DHT11, dos tópicos de temperatura e umidade e de um tratamento mais tolerante dos comandos.

### Visão geral da arquitetura

```
 [DHT11] ─┐
          ├─► [ESP32] ──Wi-Fi──► [Servidor MQTT 3.135.234.101:1883] ◄──► [App MyMQTT]
 [LDR]  ──┘        ▲                                                         │
                   └───────────── comando do LED (/cmd) ◄────────────────────┘
```

---

## 📚 Conceitos básicos (para quem nunca viu MQTT)

| Termo | O que é |
|---|---|
| **ESP32** | Microcontrolador com Wi-Fi. É o "cérebro" do circuito: lê os sensores e envia os dados. |
| **MQTT** | Protocolo leve de mensagens para IoT. Funciona como um quadro de avisos: quem tem informação "posta" e quem quer saber "assina". |
| **Broker** | O servidor MQTT. Recebe as mensagens e repassa para quem assinou. Aqui é o `3.135.234.101`, porta `1883`. |
| **Tópico** | O "endereço" de uma mensagem, parecido com um caminho de pasta. Ex.: `/TEF/device001/attrs/t`. |
| **Publish (publicar)** | Enviar uma mensagem para um tópico. |
| **Subscribe (assinar)** | Pedir ao broker para receber tudo o que for publicado em um tópico. |
| **Payload** | O conteúdo da mensagem. Ex.: `24.5` (temperatura). |
| **DHT11** | Sensor digital de temperatura (0 a 50 °C, precisão ±2 °C) e umidade (20 a 80 %, precisão ±5 %). |
| **LDR** | Resistor que muda o valor conforme a luz. Quanto mais luz, menor a resistência. |
| **Divisor de tensão** | Dois resistores em série. O ponto entre eles produz uma tensão que o ESP32 consegue ler. |
| **GPIO** | Pino de entrada ou saída do ESP32. |

**Como a comunicação acontece neste projeto:**

1. O ESP32 **publica** a temperatura em `/TEF/device0XX/attrs/t`.
2. O celular **assina** esse tópico e passa a ver o valor.
3. Para ligar o LED, o celular **publica** `device0XX@on|` em `/TEF/device0XX/cmd`.
4. O ESP32 **assinou** esse tópico, recebe a mensagem e liga o LED.

---

## 🧰 Materiais necessários

### Hardware

| Item | Quantidade | Observação |
|---|---|---|
| ESP32 (DevKit) | 1 | Usa o LED que já vem na placa (GPIO 2) |
| Sensor DHT11 | 1 | Módulo de 3 pinos ou sensor solto de 4 pinos |
| LDR (fotoresistor) | 1 | |
| Resistor de 10 kΩ | 1 | Para o divisor de tensão do LDR (marrom, preto, laranja) |
| Resistor de 10 kΩ | 1 | **Só se o DHT11 for o sensor solto de 4 pinos** (veja a nota na montagem) |
| Protoboard | 1 | |
| Jumpers | vários | |
| Cabo USB | 1 | Precisa ser de **dados**; alguns cabos só carregam |

### Software e contas

| Item | Para quê |
|---|---|
| Arduino IDE 2.x | Escrever e gravar o código |
| Celular com o app **MyMQTT** | Ler os dados e enviar comandos |
| Rede Wi-Fi **2.4 GHz** | O ESP32 não conecta em 5 GHz |

---

## 🔌 Montagem do circuito

> ⚠️ **Monte o circuito com o ESP32 desconectado do USB.** Conecte o cabo só depois de conferir tudo.

### Tabela de ligação

| Componente | Pino do componente | Pino do ESP32 |
|---|---|---|
| DHT11 | VCC (ou +) | 3V3 |
| DHT11 | DATA (ou OUT / S) | GPIO 15 |
| DHT11 | GND (ou −) | GND |
| LDR | Terminal 1 | 3V3 |
| LDR | Terminal 2 | GPIO 34 |
| Resistor 10 kΩ | Um lado no GPIO 34 (junto do LDR) | GPIO 34 |
| Resistor 10 kΩ | Outro lado | GND |
| LED | Já está na placa | GPIO 2 (nada a ligar) |

### Divisor de tensão do LDR

O LDR e o resistor ficam em série. O GPIO 34 lê o ponto entre eles.

```
3V3 ───[ LDR ]───┬───[ 10 kΩ ]─── GND
                 │
              GPIO 34
```

Com **mais luz**, o LDR tem menos resistência, a tensão no GPIO 34 sobe e a luminosidade lida **aumenta**. Com o LDR coberto, o valor **diminui**.

### Nota sobre o DHT11

- **Módulo de 3 pinos** (com uma plaquinha e etiquetas `+`, `OUT`, `−`): ligue direto, como na tabela. O resistor já vem na plaquinha.
- **Sensor solto de 4 pinos:** ligue o pino 1 no 3V3, o pino 2 no GPIO 15 e o pino 4 no GND (o pino 3 não é usado). Coloque um **resistor de 10 kΩ entre o pino 1 (3V3) e o pino 2 (GPIO 15)**. Sem ele, as leituras falham.

> 📷 **Foto do circuito montado:**
> `![Circuito montado](imagens/circuito.jpg)`

---

## 💻 Preparação do computador

Faça uma vez só.

### 1. Instalar a Arduino IDE

Baixe a versão 2.x em <https://www.arduino.cc/en/software> e instale.

### 2. Adicionar o suporte ao ESP32

1. Abra a Arduino IDE e vá em **File → Preferences**.
2. No campo **Additional boards manager URLs**, cole:
   ```
   https://espressif.github.io/arduino-esp32/package_esp32_index.json
   ```
3. Clique em **OK**.
4. Vá em **Tools → Board → Boards Manager**.
5. Busque por `esp32` e instale **esp32 by Espressif Systems**. Demora alguns minutos.

### 3. Instalar as bibliotecas

Vá em **Sketch → Include Library → Manage Libraries** e instale as três:

| Biblioteca | Autor |
|---|---|
| `PubSubClient` | Nick O'Leary |
| `DHT sensor library` | Adafruit |
| `Adafruit Unified Sensor` | Adafruit |

Se a IDE perguntar sobre dependências, clique em **Install all**.

### 4. Baixar o projeto

Baixe ou clone este repositório. O arquivo do código fica em `vinheria_mqtt/vinheria_mqtt.ino`.

> ⚠️ O arquivo `.ino` precisa estar dentro de uma pasta com o **mesmo nome** (`vinheria_mqtt`). Se não estiver, a IDE reclama.

### 5. Conferir se compila (opcional, não precisa da placa)

1. Abra o `vinheria_mqtt.ino` na IDE.
2. Vá em **Tools → Board → esp32 → ESP32 Dev Module**.
3. Clique no botão **✓ Verify**.
4. Deve aparecer `Done compiling` sem erros.

---

## ⚙️ Configuração do código

Abra o `vinheria_mqtt.ino` e ajuste o bloco do início do arquivo.

```cpp
#define NUM     "0XX"            // número do dispositivo do grupo, entre aspas (ex.: "001")

const char* SSID     = "FIAP-IOT";       // nome da rede Wi-Fi (2.4 GHz)
const char* PASSWORD = "SENHA_DA_REDE";  // senha da rede (informada pelo professor)

const char* BROKER_MQTT = "3.135.234.101";  // endereço do servidor MQTT
const int   BROKER_PORT = 1883;             // porta do servidor MQTT
```

| Variável | O que colocar |
|---|---|
| `NUM` | O número do dispositivo do grupo, com três dígitos e entre aspas. Isso ajusta sozinho todos os tópicos, o ID MQTT e o comando do LED. |
| `SSID` | Nome da rede Wi-Fi. Na faculdade: `FIAP-IOT`. |
| `PASSWORD` | Senha do Wi-Fi, informada pelo professor. **Não envie a senha real para o GitHub.** |
| `BROKER_MQTT` | `3.135.234.101` |
| `BROKER_PORT` | `1883` |

> O ESP32 só conecta em redes **2.4 GHz**. Se a rede da sala não funcionar, use o hotspot do celular configurado em 2.4 GHz.

### Valores que não precisam mudar

| Item | Valor no código |
|---|---|
| Pino do DHT11 | `DHTPIN 15` |
| Tipo do sensor | `DHTTYPE DHT11` |
| Pino do LDR | `LDRPIN 34` |
| Pino do LED | `LEDPIN 2` |
| Intervalo de envio | `INTERVALO 5000` (5 segundos) |

---

## 🚀 Gravando o código no ESP32

1. Conecte o ESP32 ao computador com o cabo USB.
2. Na IDE, vá em **Tools → Board → esp32 → ESP32 Dev Module**.
3. Vá em **Tools → Port** e escolha a porta que apareceu (ex.: `COM3`). Se nenhuma aparecer, troque o cabo (precisa ser de dados) ou instale o driver CP210x/CH340.
4. Clique no botão **→ Upload**.
5. Se aparecer `Connecting........` e travar, **segure o botão BOOT** do ESP32 até a gravação começar, e depois solte.
6. Espere aparecer `Hard resetting via RTS pin...`.

### Conferindo no Monitor Serial

1. Vá em **Tools → Serial Monitor**.
2. No canto inferior direito, escolha **115200 baud**.
3. Se não aparecer nada, aperte o botão **EN** (reset) do ESP32.

Saída esperada:

```
Conectando na rede: FIAP-IOT
....
Conectado! IP: 10.x.x.x
* Conectando ao Broker MQTT: 3.135.234.101
Conectado ao broker MQTT!
Temp: 24.0 C | Umid: 60.0 % | Luz: 44 % | LED: off
Temp: 24.0 C | Umid: 60.0 % | Luz: 44 % | LED: off
```

Se aparecer `Conectado ao broker MQTT!` e as linhas de leitura, o ESP32 está funcionando e enviando dados.

> 📷 **Print do Monitor Serial:**
> `![Monitor Serial](imagens/monitor_serial.png)`

---

## 📡 Tópicos MQTT utilizados

Troque `device0XX` pelo número do dispositivo do grupo. Os tópicos **começam com `/`**.

| Tópico | Direção | O que contém | Exemplo de mensagem |
|---|---|---|---|
| `/TEF/device0XX/attrs/t` | ESP32 → Servidor | Temperatura em °C | `24.5` |
| `/TEF/device0XX/attrs/h` | ESP32 → Servidor | Umidade relativa em % | `62.0` |
| `/TEF/device0XX/attrs/p` | ESP32 → Servidor | Luminosidade em % (0 a 100) | `45` |
| `/TEF/device0XX/attrs` | ESP32 → Servidor | Estado do LED | `s\|on` ou `s\|off` |
| `/TEF/device0XX/cmd` | Servidor → ESP32 | Comando para o LED | `device0XX@on\|` ou `device0XX@off\|` |

- Os valores de temperatura, umidade e luminosidade são enviados a cada **5 segundos**.
- O estado do LED também é reenviado a cada 5 segundos e na hora em que um comando é executado.
- O comando é escrito como `device0XX@on|` (liga) e `device0XX@off|` (desliga), **com o `|` no final e sem espaços**.

---

## 📱 Usando o MyMQTT (leitura e escrita)

O app tem três áreas principais: **Settings** (conexão), **Subscribe** (ler) e **Publish** (escrever). As telas podem variar um pouco entre versões, mas os campos são os mesmos.

### 1. Conectar ao servidor

1. Abra o MyMQTT e vá em **Settings**.
2. Preencha:
   - **Broker address:** `3.135.234.101`
   - **Port:** `1883`
   - Usuário e senha: deixe em branco (a menos que o professor informe).
3. Salve e toque em **Connect**. O app deve indicar que está conectado.

> O celular precisa estar em uma rede com acesso ao servidor. Se não conectar na rede da faculdade, tente com o hotspot ou com dados móveis.

### 2. Leitura: assinar os tópicos (Subscribe)

Na aba **Subscribe**, assine cada um dos tópicos abaixo (trocando `0XX`):

- `/TEF/device0XX/attrs/t` → temperatura
- `/TEF/device0XX/attrs/h` → umidade
- `/TEF/device0XX/attrs/p` → luminosidade
- `/TEF/device0XX/attrs` → estado do LED

Espere até **10 segundos**: os valores começam a aparecer a cada 5 segundos.

> **Atalho para ver tudo de uma vez:** assine `/TEF/device0XX/#`. O `#` recebe todos os tópicos que começam com `/TEF/device0XX/`.

> 📷 **Print do MyMQTT recebendo os dados:**
> `![MyMQTT leitura](imagens/mymqtt_leitura.png)`

### 3. Escrita: enviar comando ao LED (Publish)

Na aba **Publish**:

| Campo | Valor |
|---|---|
| Topic | `/TEF/device0XX/cmd` |
| Message (ligar) | `device0XX@on\|` |
| Message (desligar) | `device0XX@off\|` |

Depois de enviar:

1. O LED da placa ESP32 acende (ou apaga).
2. O Monitor Serial mostra a mensagem recebida.
3. Em até 5 segundos, o tópico `/TEF/device0XX/attrs` passa a mostrar `s|on` ou `s|off`.

⚠️ **Cuidado ao digitar o comando:** o teclado do celular pode inserir um **espaço antes do `|`**. O código tolera isso, mas confira o texto antes de enviar.

> 📷 **Print do comando enviado:**
> `![MyMQTT escrita](imagens/mymqtt_escrita.png)`

---

## 🧠 Como o código funciona

O arquivo `vinheria_mqtt/vinheria_mqtt.ino` é organizado assim:

| Parte | O que faz |
|---|---|
| **Configurações do início** | Número do dispositivo, Wi-Fi, servidor e tópicos. Os tópicos são montados automaticamente a partir de `NUM`. |
| `setup()` | Roda uma vez ao ligar: inicia a serial, configura o LED, inicia o DHT11, conecta ao Wi-Fi e configura o servidor MQTT. |
| `loop()` | Roda para sempre: garante que Wi-Fi e MQTT estão conectados, escuta mensagens e, a cada 5 s, envia os sensores. |
| `reconectWiFi()` | Conecta ao Wi-Fi e só sai quando conseguir. |
| `reconnectMQTT()` | Conecta ao servidor, **assina o tópico `/cmd`** e publica o estado inicial do LED. Repete a tentativa a cada 2 s se falhar. |
| `mqtt_callback()` | Chamada automaticamente quando chega uma mensagem. Se contém `@on`, liga o LED. Se contém `@off`, desliga. Depois informa o novo estado. |
| `enviaSensores()` | Lê temperatura, umidade e luminosidade, publica nos tópicos e escreve a leitura no Monitor Serial. |

### Detalhes de funcionamento

- **Luminosidade em %:** o ADC do ESP32 lê de 0 a 4095 (12 bits). O código converte para 0 a 100 com `map()`.
- **Falha do DHT11:** se o sensor não responder, a leitura vem como `NaN`. O código avisa no Serial e **não** publica temperatura e umidade naquele ciclo, para não enviar lixo.
- **ID MQTT único:** o ID é `fiware_` + número do dispositivo. Dois ESP32 com o mesmo ID se derrubam mutuamente no servidor; por isso cada grupo usa o seu número.
- **Reconexão automática:** se o Wi-Fi ou o servidor cair, o ESP32 reconecta sozinho.
- **GPIO 34:** é um pino de entrada analógica do grupo ADC1, que funciona normalmente com o Wi-Fi ligado.

---

## 🧪 Testando sem o hardware (Wokwi)

Não tem o kit em casa? Dá para simular o projeto em <https://wokwi.com> (gratuito). O ESP32 simulado se conecta ao servidor de verdade.

1. Crie um projeto **ESP32** (Arduino).
2. Cole o conteúdo de `vinheria_mqtt.ino` no `sketch.ino`.
3. No arquivo `libraries.txt`, escreva uma biblioteca por linha:
   ```
   PubSubClient
   DHT sensor library
   Adafruit Unified Sensor
   ```
4. Adicione ao circuito um **DHT22** e um **Photoresistor Sensor** e ligue assim:

   | Componente | Pino | ESP32 |
   |---|---|---|
   | DHT22 | VCC | 3V3 |
   | DHT22 | SDA | D15 |
   | DHT22 | GND | GND |
   | Photoresistor | VCC | 3V3 |
   | Photoresistor | AO | D34 |
   | Photoresistor | GND | GND |

5. **Só na simulação**, altere no código:
   ```cpp
   #define DHTTYPE DHT22
   const char* SSID     = "Wokwi-GUEST";
   const char* PASSWORD = "";
   ```
   E use um número de teste no `NUM` (ex.: `"099"`) para não conflitar com outros grupos.
6. Clique em **▶ Start Simulation** e use o MyMQTT normalmente.

> ⚠️ **Antes de gravar no ESP32 real,** volte `DHTTYPE` para `DHT11`, o `SSID` para `FIAP-IOT` e o `NUM` para o número do grupo.

---


---

## 🛠️ Solução de problemas

| Sintoma | Causa provável | O que fazer |
|---|---|---|
| Não aparece nenhuma porta COM | Cabo só de carga ou falta de driver | Trocar o cabo; instalar o driver CP210x ou CH340 |
| Upload trava em `Connecting...` | Placa não entrou em modo de gravação | Segurar o botão **BOOT** durante o upload |
| Monitor Serial vazio ou com símbolos estranhos | Velocidade errada | Selecionar **115200 baud** e apertar **EN** |
| Só aparecem pontinhos `....` | Wi-Fi não conecta | Conferir SSID e senha; usar rede **2.4 GHz**; tentar o hotspot |
| `Falha, rc=-2` | Servidor inacessível | Conferir IP e porta; verificar se a rede libera a porta 1883; tentar o hotspot |
| `Falha, rc=5` | Servidor exige usuário e senha | Perguntar ao professor |
| Conecta e desconecta toda hora | Outro dispositivo com o mesmo `NUM` | Conferir se o número do grupo é único |
| `Falha na leitura do DHT11` | Fio solto, pino errado, falta do resistor (sensor de 4 pinos) | Conferir DATA no GPIO 15, 3V3 e GND |
| Luminosidade sempre 0 ou sempre 100 | LDR ou resistor mal ligado | Conferir o divisor de tensão no GPIO 34 |
| Dados não aparecem no MyMQTT | Número do dispositivo diferente no app | O número no tópico do app tem que ser igual ao `NUM` do código; testar com `/TEF/device0XX/#` |
| Dados demoram a chegar | Intervalo de envio e atraso de rede | Esperar até 10 s |
| LED não responde ao comando | Tópico errado ou comando incorreto | Tópico `/TEF/device0XX/cmd`; mensagem `device0XX@on\|` ou `device0XX@off\|`; olhar o Serial |
| Estado continua `s\|off` | Comando não foi recebido | No Serial deve aparecer `- Topico: ... \| Mensagem: [...]` após o envio |

---


---

## 🔐 Observação sobre a senha do Wi-Fi

Por segurança, a senha real do Wi-Fi **não está no código deste repositório**. O campo `PASSWORD` contém um texto de exemplo. Para executar o projeto, substitua pela senha informada pelo professor, apenas na sua cópia local.

---

*Projeto acadêmico – FIAP – Edge Computing – Check Point 05.*
