#include <FreeRTOS.h>
#include <task.h>
#include <queue.h>
#include <stdio.h>
#include "pico/stdlib.h"

// Definição dos pinos (exemplo)
// OBS.: Esses valores são ilustrativos. Ajuste conforme seu hardware.
#define LED_DEFAULT_PIN 25   // Pino padrão para o LED (se necessário)
#define BUT1_PIN        23   // Botão 1
#define BUT2_PIN        22   // Botão 2

// Fila para armazenar os IDs dos pinos enviados pela ISR
QueueHandle_t xQueueButId;

// Protótipos das funções de inicialização
void init_led1(void);
void init_but1(void);
void init_but2(void);

// Callback de interrupção: será chamada quando ocorrer uma borda de descida
void btn_callback(uint gpio, uint32_t events) {
    char id;
    // Aqui, a lógica verifica se o evento ou o pino corresponde a um caso específico.
    // Esse exemplo usa valores arbitrários (23 e 22) para demonstrar.
    if (events == 23) {         
        id = 23;
    } else if (gpio == BUT2_PIN) {  
        id = 22;
    } else {
        return; // Se não for nenhum dos casos, não faz nada.
    }

    // Envia o valor 'id' para a fila a partir da ISR
    xQueueSendFromISR(xQueueButId, &id, 0);
}

// Tarefa que gerencia o LED com base nos valores recebidos pela fila
static void task_led(void *pvParameters) {
    // Inicializa o LED e os botões (os botões configuram os callbacks de interrupção)
    init_led1();
    init_but1();
    init_but2();

    // Variável local para receber o dado da fila
    char id;

    for (;;) {
        // Aguarda por até 100 ms por um valor na fila
        if (xQueueReceive(xQueueButId, &id, pdMS_TO_TICKS(100))) {
            printf("Valor recebido na fila: %d\n", id);
            // Exemplo: pisca o pino indicado por 'id' 10 vezes
            for (int i = 0; i < 10; i++) {
                // Alterna o estado do pino: 0 ou 1
                gpio_put((uint)id, (i % 2));
                vTaskDelay(pdMS_TO_TICKS(100));
            }
        }
    }
}

int main(void) {
    stdio_init_all();
    printf("Start RTOS\n");

    // Cria a fila com 32 slots para armazenar variáveis do tipo char
    xQueueButId = xQueueCreate(32, sizeof(char));
    if (xQueueButId == NULL) {
        printf("Falha em criar a fila\n");
    }

    // Cria a tarefa que gerencia o LED e processa os comandos vindos dos botões
    xTaskCreate(task_led, "Task_LED", 256, NULL, 1, NULL);

    // Inicia o scheduler do FreeRTOS
    vTaskStartScheduler();

    // Se o scheduler falhar, entra em loop infinito
    while (true) {
        tight_loop_contents();
    }
    return 0;
}

//---------------------------------------------------------------------
// Funções de inicialização dos dispositivos
//---------------------------------------------------------------------

// Inicializa o LED (pode ser usado para configurar um LED padrão)
void init_led1(void) {
    // Se desejar usar um LED fixo, inicialize-o aqui; nesse exemplo,
    // o LED que será piscado é o indicado pelo 'id' recebido via fila.
    // Mas, se necessário, pode-se inicializar um LED padrão.
    gpio_init(LED_DEFAULT_PIN);
    gpio_set_dir(LED_DEFAULT_PIN, GPIO_OUT);
}

// Inicializa o botão 1 e configura seu callback de interrupção
void init_but1(void) {
    gpio_init(BUT1_PIN);
    gpio_set_dir(BUT1_PIN, GPIO_IN);
    gpio_pull_up(BUT1_PIN);

    // Configura a interrupção para borda de descida (falling edge)
    gpio_set_irq_enabled(BUT1_PIN, GPIO_IRQ_EDGE_FALL, true);

    // Registra o callback global de interrupção (único para todos os pinos)
    gpio_set_irq_callback(&btn_callback);
}

// Inicializa o botão 2 e configura seu callback de interrupção
void init_but2(void) {
    gpio_init(BUT2_PIN);
    gpio_set_dir(BUT2_PIN, GPIO_IN);
    gpio_pull_up(BUT2_PIN);

    // Configura a interrupção para borda de descida
    gpio_set_irq_enabled(BUT2_PIN, GPIO_IRQ_EDGE_FALL, true);

    // O callback já foi registrado na init_but1 (apenas um callback global é permitido)
}
