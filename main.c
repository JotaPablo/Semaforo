#include <stdio.h>
#include "pico/bootrom.h"
#include "pico/stdlib.h"

#include "lib/ssd1306.h"
#include "neopixel.h"
#include "buzzer.h"

#include "FreeRTOS.h"
#include "FreeRTOSConfig.h"
#include "task.h"

// Pinos
#define RED_PIN       13
#define GREEN_PIN     11
#define BLUE_PIN      12
#define BUTTON_A      5
#define BUTTON_JOYSTICK 22
#define BUZZER_PIN    21


static ssd1306_t ssd; // Estrutura do display OLED
volatile bool modoNoturno = false;
volatile bool atualizaDisplay = false; // Flag para 
volatile int estadoSemaforo = 2; // Estado atual do semáforo (0:verde, 1:amarelo, 2:vermelh

//Função para haver o delay porém sensível à mudança de modo
void esperaModo(bool modoEsperado, int tempoTotalMs) {
    const int passo = 10; // em ms
    for (int i = 0; i < tempoTotalMs; i += passo) {
        if (modoNoturno != modoEsperado) break;
        vTaskDelay(pdMS_TO_TICKS(passo));
    }
}

//Altera habilita e desabilita o Modo Noturno
void vBotaoATask(){

    absolute_time_t last_time = 0;
    absolute_time_t current_time;

    while(true){

        current_time = to_ms_since_boot(get_absolute_time());

        // Verifica pressionamento com debounce de 200ms
        if(!gpio_get(BUTTON_A) && current_time - last_time >= 200){

            printf("Botão A pressionado!\n\n");
            
            last_time = current_time;
            modoNoturno = !modoNoturno; // Altera o modo
            atualizaDisplay = true;

            if(modoNoturno)
                printf("Modo Noturno Habilitado\n\n");
            else{
                printf("Modo Noturno Desabilitado\n\n");
                estadoSemaforo = 2;
            }
        }
        vTaskDelay(pdMS_TO_TICKS(50));
    }
}

//Alterna os estados do semaforo
void vEstadoSemaforo(){
    while(true){
        if(!modoNoturno){
            estadoSemaforo = (estadoSemaforo + 1)%3;
            atualizaDisplay = true;
            esperaModo(false, 2000);
        }
        else esperaModo(true, 50);
    }
}

//Controla o LED RGB segundo estado do semáforo ou modo noturno
void vLedRGBTask(){

    while(true){

        if(modoNoturno){
            //Pisca Amarelo(1 segundo ligado e 1 segundo desligado)
            gpio_put(GREEN_PIN, true);
            gpio_put(RED_PIN, true);
            gpio_put(BLUE_PIN, false);

            esperaModo(true, 1000);

            gpio_put(GREEN_PIN, false);
            gpio_put(RED_PIN, false);
            gpio_put(BLUE_PIN, false);

            esperaModo(true, 1000);
        }
        else{
            // Acende cor conforme estado do semáforo
            if(estadoSemaforo == 0) {  // Verde
                gpio_put(GREEN_PIN, true);
                gpio_put(RED_PIN, false);
                gpio_put(BLUE_PIN, false);
            }
            else if(estadoSemaforo == 1) {  // Amarelo
                gpio_put(GREEN_PIN, true);
                gpio_put(RED_PIN, true);   
                gpio_put(BLUE_PIN, false);
            }
            else if(estadoSemaforo == 2) {  // Vermelho
                gpio_put(GREEN_PIN, false);
                gpio_put(RED_PIN, true);
                gpio_put(BLUE_PIN, false);
            }

          vTaskDelay(pdMS_TO_TICKS(20)); // Espera de cada 50ms
        }
    }
}

//Controla a matriz 5x5 de LEDs
void vMatrizLedTask(){

    while(true){
        if(modoNoturno){
             // Amarelo piscando
            npSetLED(npGetIndex(2,3), 0, 1, 0);
            npSetLED(npGetIndex(2,2), 20, 20, 0);
            npSetLED(npGetIndex(2,1), 1, 0, 0);
            npWrite();

            esperaModo(true, 1000);

            npSetLED(npGetIndex(2,3), 0, 1, 0);
            npSetLED(npGetIndex(2,2), 1, 1, 0);
            npSetLED(npGetIndex(2,1), 1, 0, 0);
            npWrite();

            esperaModo(true, 1000);

        }
        else{
            if(estadoSemaforo == 0) {  // Verde
                npSetLED(npGetIndex(2,3), 0, 50, 0);
                npSetLED(npGetIndex(2,2), 1, 1, 0);
                npSetLED(npGetIndex(2,1), 1, 0, 0);
                npWrite();

            }
            else if(estadoSemaforo == 1) {  // Amarelo
                npSetLED(npGetIndex(2,3), 0, 1, 0);
                npSetLED(npGetIndex(2,2), 50, 50, 0);
                npSetLED(npGetIndex(2,1), 1, 0, 0);
                npWrite();

            }
            else if(estadoSemaforo == 2) {  // Vermelho
                npSetLED(npGetIndex(2,3), 0, 1, 0);
                npSetLED(npGetIndex(2,2), 1, 1, 0);
                npSetLED(npGetIndex(2,1), 50, 0, 0);
                npWrite();
            }

            esperaModo(false,50);
        }
    }
}

//Atualização do display OLED
void vDisplayOledTask(){

    ssd1306_fill(&ssd, false);
    atualizaDisplay = true; 

    while(true){

        if(atualizaDisplay){

            if(modoNoturno){
                ssd1306_fill(&ssd, false); //Apaga

                //Desenha um semaforo a esquerda com o circulo do meio preenchido
                ssd1306_rect(&ssd, 5, 2, 20, 54, true, false);
                ssd1306_draw_string(&ssd, "MODO NOTURNO", 27, 32);
                ssd1306_circle(&ssd, 12, 19, 5, true, false);
                ssd1306_circle(&ssd, 12, 33, 5, true, true);
                ssd1306_circle(&ssd, 12, 47, 5, true, false);
                ssd1306_send_data(&ssd);
                
                esperaModo(true, 1000);
                
                //Desenha um semaforo a esquerda com o circulo do meio não preenchido
                ssd1306_fill(&ssd, false);
                ssd1306_rect(&ssd, 5, 2, 20, 54, true, false);
                ssd1306_draw_string(&ssd, "MODO NOTURNO", 27, 32);
                ssd1306_circle(&ssd, 12, 19, 5, true, false);
                ssd1306_circle(&ssd, 12, 33, 5, true, false);
                ssd1306_circle(&ssd, 12, 47, 5, true, false);
                ssd1306_send_data(&ssd);
                esperaModo(true, 1000);
                
            }
            else{
                atualizaDisplay = false;
                ssd1306_fill(&ssd, false);

                //Desenha o controno do semaforo
                ssd1306_rect(&ssd, 5, 2, 20, 54, true, false);
            
                if(estadoSemaforo == 0) {  // Verde (Circulo de cima preenchido)
                    ssd1306_draw_string(&ssd, "PODE", 59, 22);
                    ssd1306_draw_string(&ssd, "AVANCAR", 47, 34);
                    ssd1306_circle(&ssd, 12, 19, 5, true, true);
                    ssd1306_circle(&ssd, 12, 33, 5, true, false);
                    ssd1306_circle(&ssd, 12, 47, 5, true, false);
                }
                else if(estadoSemaforo == 1) {  // Amarelo (Circulo do meio preenchido)

                    ssd1306_draw_string(&ssd, "ATENCAO", 47, 32);
                    ssd1306_circle(&ssd, 12, 19, 5, true, false);
                    ssd1306_circle(&ssd, 12, 33, 5, true, true);
                    ssd1306_circle(&ssd, 12, 47, 5, true, false);
                }
                else if(estadoSemaforo == 2) {  // Vermelho (Circulo de baixo preenchido)
                    ssd1306_draw_string(&ssd, "PARE", 59, 32);
                    ssd1306_circle(&ssd, 12, 19, 5, true, false);
                    ssd1306_circle(&ssd, 12, 33, 5, true, false);
                    ssd1306_circle(&ssd, 12, 47, 5, true, true);
                }
                ssd1306_send_data(&ssd);
            }
        }
        vTaskDelay(pdMS_TO_TICKS(20));
    }


}

//Controle sonoro do buzzer
void vBuzzerTask() {
    const uint FREQ_VERDE = 2000;
    const uint FREQ_AMARELO = 3000;
    const uint FREQ_VERMELHO = 1000;
    const uint FREQ_NOTURNO = 500;

    while (true) {
        if (modoNoturno) {
            buzzer_turn_on(FREQ_NOTURNO);
            esperaModo(true, 200);
            buzzer_stop();  // para garantir que o beep pare
            esperaModo(true, 1800);
        } else {
            switch (estadoSemaforo) {
                case 0:  // Verde
                //1 beep curto por um segundo “pode atravessar”
                    buzzer_turn_on(FREQ_VERDE);
                    esperaModo(false,1000);
                    buzzer_stop();
                    esperaModo(false,1000);
                    break;

                case 1:  // Amarelo
                //beep rápido intermitente “atenção”
                    for (int i = 0; i < 2; i++) {
                        if (modoNoturno) break;
                        buzzer_turn_on(FREQ_AMARELO);
                        esperaModo(false,200);
                        buzzer_stop();
                        esperaModo(false,200);
                    }
                    break;

                case 2:  // Vermelho
                // tom contínuo curto (500ms ligado e 1.5s desligado) “pare”
                    buzzer_turn_on(FREQ_VERMELHO);
                    esperaModo(false,500);
                    buzzer_stop();
                    esperaModo(false,1500);
                    break;
            }
        }
        buzzer_stop();
    }
}

//Callback para colocar em modo BOOTSEL
static void gpio_button_joystick_handler(uint gpio, uint32_t events){
    printf("\nHABILITANDO O MODO GRAVAÇÃO\n");

    // Adicionar feedback no display OLED
    ssd1306_fill(&ssd, false);
    ssd1306_draw_string(&ssd, "  HABILITANDO", 5, 25);
    ssd1306_draw_string(&ssd, " MODO GRAVACAO", 5, 38);
    ssd1306_send_data(&ssd);

    reset_usb_boot(0,0); // Reinicia no modo DFU
}

//Configuração inicial de hardware
void setup(){

    stdio_init_all();

    npInit(LED_PIN);

    display_init(&ssd);

    buzzer_init(BUZZER_PIN);

    // Configuração dos LEDs
    gpio_init(RED_PIN);
    gpio_set_dir(RED_PIN, GPIO_OUT);
    gpio_init(GREEN_PIN);
    gpio_set_dir(GREEN_PIN, GPIO_OUT);
    gpio_init(BLUE_PIN);
    gpio_set_dir(BLUE_PIN, GPIO_OUT);

    //Configuração dos Botões
    gpio_init(BUTTON_A);
    gpio_set_dir(BUTTON_A, GPIO_IN);
    gpio_pull_up(BUTTON_A);

    gpio_init(BUTTON_JOYSTICK);
    gpio_set_dir(BUTTON_JOYSTICK, GPIO_IN);
    gpio_pull_up(BUTTON_JOYSTICK);

    gpio_set_irq_enabled_with_callback(BUTTON_JOYSTICK, GPIO_IRQ_EDGE_FALL, true, &gpio_button_joystick_handler);

}

int main()
{
    setup();
    
    xTaskCreate(vBotaoATask, "Botao A Task", configMINIMAL_STACK_SIZE,
        NULL, tskIDLE_PRIORITY + 1, NULL);
    xTaskCreate(vEstadoSemaforo, "Estado Semaforo Task", configMINIMAL_STACK_SIZE,
        NULL, tskIDLE_PRIORITY, NULL);
    xTaskCreate(vLedRGBTask, "Led RGB Task", configMINIMAL_STACK_SIZE,
        NULL, tskIDLE_PRIORITY, NULL);
    xTaskCreate(vMatrizLedTask, "Matriz Led Task", configMINIMAL_STACK_SIZE,
        NULL, tskIDLE_PRIORITY, NULL);
    xTaskCreate(vDisplayOledTask, "Display OLED Task", configMINIMAL_STACK_SIZE,
            NULL, tskIDLE_PRIORITY, NULL);
    xTaskCreate(vBuzzerTask, "Buzzer Task", configMINIMAL_STACK_SIZE,
        NULL, tskIDLE_PRIORITY, NULL);

    vTaskStartScheduler();
    panic_unsupported();
}

