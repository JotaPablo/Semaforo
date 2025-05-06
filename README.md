# Semáforo Inteligente com Modo Noturno - Utilizando FreeRTOS

Um sistema de semáforo embarcado utilizando a plataforma RP2040 (Raspberry Pi Pico) com FreeRTOS, que alterna automaticamente entre os estados Verde, Amarelo e Vermelho, e conta com um modo noturno de sinalização(Pisca o Amarelo). Além de LEDs RGB discretos, matriz de LEDs e display OLED, o projeto inclui **feedback sonoro via buzzer** para acessibilidade de pessoas com baixa visão. O modo noturno é ativado pelo **botão A**, e o **botão joystick** serve para entrar em modo BOOTSEL.

## Demonstração em Vídeo
[![Assista ao vídeo no YouTube](https://img.youtube.com/vi/HY4chSQSjx0/hqdefault.jpg)](https://youtu.be/HY4chSQSjx0)

## Funcionalidades Principais

- **Modo Normal**  
  - Ciclo de cores: Verde → Amarelo → Vermelho  
  - **Feedback sonoro**:
    - Verde: 1 beep curto por 1 s (“pode atravessar”)
    - Amarelo: beeps rápidos intermitentes (“atenção”)
    - Vermelho: tom contínuo curto (500 ms ligado, 1.5 s desligado) (“pare”)
- **Modo Noturno**  
  - Pisca sinal amarelo (LED RGB e matriz WS2812B)  
  - Beep lento a cada 2 s  
  - Mensagem “MODO NOTURNO” no display OLED  
- Controle de tarefas com **FreeRTOS** (sem filas, semáforos ou mutexes)  
- Exibição gráfica dos estados no **Display OLED SSD1306**  
- Sinalização visual:
  - LEDs RGB discretos (Verde, Amarelo, Vermelho)
  - Matriz de LEDs RGB WS2812B (5×5)  
- Alternância de modo via **Botão A**  
- Entrar em modo BOOTSEL via **Botão Joystick**  

## Componentes Utilizados

| Componente                 | GPIO/Pino     | Função                                                      |
|----------------------------|---------------|-------------------------------------------------------------|
| Display OLED SSD1306       | 14 (SDA), 15 (SCL) | Exibição de texto e ícones dos estados do semáforo         |
| LED RGB Verde              | 11            | Indicação do estado “Verde”                                 |
| LED RGB Vermelho           | 13            | Indicação do estado “Vermelho”                              |
| LED RGB Azul               | 12            | Combinado para sinal amarelo (Verde + Vermelho)             |
| Matriz de LEDs WS2812B     | (configurável)¹ | Representação animada dos estados                          |
| Buzzer                     | 21            | Feedback sonoro para acessibilidade (feedback de semáforo)  |
| Botão A                    | 5             | Alterna entre modo Normal e Modo Noturno                    |
| Botão Joystick             | 22            | Entra em modo BOOTSEL (DFU)      

## Organização do Código

- **`main.c`**  
  – Inicialização de hardware e criação de tarefas FreeRTOS  
- **Tarefas principais**  
  - `vBotaoATask` - altera entre Modo Noturno e Modo Normal
  - `vEstadoSemaforoTask()` – alterna ciclos Verde → Amarelo → Vermelho (modo normal) ou apenas pisca Amarelo (modo noturno)  
  - `vLedRGBTask()` – acende LEDs RGB discretos conforme estado ou pisca Amarelo no modo noturno  
  - `vMatrizLedTask()` – anima a matriz WS2812B de acordo com o estado  
  - `vDisplayOledTask()` – atualiza o SSD1306 com textos e desenho do semáforo  
  - `vBuzzerTask()` – gera os padrões de beep para cada estado e modo  
- **Funções auxiliares**  
  - `esperaModo(bool modoEsperado, int ms)` – delay sensível à troca de modo  
  - Handlers de interrupção de botões (`gpio_button_joystick_handler`)  

---


## ⚙️ Instalação e Uso

1. **Pré-requisitos**
   - Clonar o repositório:
     ```bash
     git clone https://github.com/JotaPablo/semaforo-inteligente.git
     cd semaforo-inteligente
     ```
   - Instalar o **Visual Studio Code** com as extensões:
     - **C/C++**
     - **Pico SDK Helper** ou extensão da Raspberry Pi Pico
     - **Compilador ARM GCC**
     - **CMake Tools**

2. **Ajuste do caminho do FreeRTOS**
   - Abra o arquivo `CMakeLists.txt` na raiz do projeto e ajuste a variável `FREERTOS_KERNEL_PATH` para o diretório onde você instalou o FreeRTOS Kernel.  
     Exemplo:
     ```cmake
     set(FREERTOS_KERNEL_PATH "C:/FreeRTOS-Kernel")
     ```
     → Substitua `"C:/FreeRTOS-Kernel"` pelo caminho correto na sua máquina.

3. **Compilação**
   - Compile o projeto manualmente via terminal:
     ```bash
     mkdir build
     cd build
     cmake ..
     make
     ```
   - Ou utilize a opção **Build** da extensão da Raspberry Pi Pico no VS Code.

4. **Execução**
   - Conecte o Raspberry Pi Pico ao computador mantendo o botão **BOOTSEL** pressionado.
   - Copie o arquivo `.uf2` gerado na pasta `build` para o dispositivo `RPI-RP2`.