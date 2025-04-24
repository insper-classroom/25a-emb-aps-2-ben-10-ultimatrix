/*
 * LED blink with FreeRTOS
 */
#include <FreeRTOS.h>
#include <task.h>
#include <semphr.h>
#include <queue.h>

#include "ssd1306.h"
#include "gfx.h"

#include "pico/stdlib.h"
#include <stdio.h>


#include "hardware/gpio.h"
#include "hardware/adc.h"

const uint BTN_START = 17;
const uint BTN_SAVE = 18;
const uint BTN_LOAD = 19;
const uint BTN_HEROI = 20;

QueueHandle_t xQueueControle;
SemaphoreHandle_t xSemaphore_start;
SemaphoreHandle_t xSemaphore_save;
SemaphoreHandle_t xSemaphore_load;
SemaphoreHandle_t xSemaphore_heroi;

int16_t modulo(int16_t x) {
    return (x < 0) ? -x : x;
}

void btn_callback(uint gpio, uint32_t events) {
    if(events == 0x4){
        if(gpio == BTN_START){
            xSemaphoreGiveFromISR(xSemaphore_start, 0);
        } else if(gpio == BTN_SAVE){
            xSemaphoreGiveFromISR(xSemaphore_save, 0);
        } else if(gpio == BTN_LOAD){
            xSemaphoreGiveFromISR(xSemaphore_load, 0);
        } else if(gpio == BTN_HEROI){   
            xSemaphoreGiveFromISR(xSemaphore_heroi, 0);
        }
    }
}

void btn_task(void *p) {
    int botao;
    gpio_init(BTN_START);
    gpio_set_dir(BTN_START, GPIO_IN);
    gpio_pull_up(BTN_START);
    gpio_set_irq_enabled_with_callback(BTN_START, GPIO_IRQ_EDGE_FALL, true, &btn_callback);

    gpio_init(BTN_SAVE);
    gpio_set_dir(BTN_SAVE, GPIO_IN);
    gpio_pull_up(BTN_SAVE);
    gpio_set_irq_enabled_with_callback(BTN_SAVE, GPIO_IRQ_EDGE_FALL, true, &btn_callback);

    gpio_init(BTN_LOAD);
    gpio_set_dir(BTN_LOAD, GPIO_IN);
    gpio_pull_up(BTN_LOAD);
    gpio_set_irq_enabled_with_callback(BTN_LOAD, GPIO_IRQ_EDGE_FALL, true, &btn_callback);

    gpio_init(BTN_HEROI);
    gpio_set_dir(BTN_HEROI, GPIO_IN);
    gpio_pull_up(BTN_HEROI);
    gpio_set_irq_enabled_with_callback(BTN_HEROI, GPIO_IRQ_EDGE_FALL, true, &btn_callback);

    while (1) {
        if(xSemaphoreTake(xSemaphore_start, pdMS_TO_TICKS(500)) == pdTRUE) {
            botao = 1;
            xQueueSend(xQueueControle, &botao, 0); // Envia p fila se clicou no start
        }else if(xSemaphoreTake(xSemaphore_save, pdMS_TO_TICKS(500)) == pdTRUE) {
            botao = 2;
            xQueueSend(xQueueControle, &botao, 0); // Envia p fila se clicou no save
        } else if(xSemaphoreTake(xSemaphore_load, pdMS_TO_TICKS(500)) == pdTRUE) {
            botao = 3;
            xQueueSend(xQueueControle, &botao, 0); // Envia p fila se clicou no load
        } else if(xSemaphoreTake(xSemaphore_heroi, pdMS_TO_TICKS(500)) == pdTRUE) {
            botao = 4;
            xQueueSend(xQueueControle, &botao, 0); // Envia p fila se clicou no heroi
        }
        vTaskDelay(pdMS_TO_TICKS(50)); // Debounce e delay de leitura
    }

}

void ptc_task(void *p) {
    adc_init();
    adc_gpio_init(27); 

    
    uint16_t result;
    int envio;
    uint16_t last_result = adc_read(); // Última leitura do ADC


    // 270 graus é o máximo do potenciômetro
    // O adc faz leituras de 0 a 4095
    // 15 graus é o quanto de giro é desejado para printar se foi p sentido horario ou antihorario
    // int((15/270) * 4095) = 228


    const uint16_t limiar = 228; // Limiar para ~15° (5,56% de 4095)

    while (1) {
        adc_select_input(1); 
        result = adc_read(); 

        int16_t delta = result - last_result;
        if (modulo(delta) > limiar) {
            if (delta > 0) {
                // Giro horário 
                envio = 5;
                xQueueSend(xQueueControle, &envio, 0); // Envia p fila se girou horario
            } else {
                // Giro anti-horário
                envio = 6;
                xQueueSend(xQueueControle, &envio, 0); // Envia p fila se girou anti-horario
            }
            last_result = result;
        }
        vTaskDelay(pdMS_TO_TICKS(200)); // Delay de 200ms
    }
}

void pressure_task(void *p) {
    adc_init();
    adc_gpio_init(28); // SoftPot ligado ao GPIO27 (ADC1)

    uint16_t leitura;
    uint16_t limiar_toque = 60; // Se estiver abaixo, consideramos "sem toque"
    bool pressionado = false;

    uint8_t index = 0;
    uint32_t sum = 0;
    uint16_t anterior;
    int resultado;

    while (1) {
        adc_select_input(2); // Seleciona o canal ADC2
        leitura = adc_read(); // Valor entre 0 e 4095
        //printf("leitura: %d\n", leitura);

        if (index != 0){
            sum += modulo(leitura - anterior);

        }     
        //printf("sum: %d\n", sum);
        if(index == 2) {
            if (modulo(sum) < limiar_toque){
                resultado = 7;
                xQueueSend(xQueueControle, &resultado, 0); // Envia p fila se clicou no macro
            }
            sum = 0;
        }
        index = (index + 1) % 3;
    
        vTaskDelay(pdMS_TO_TICKS(50)); // Debounce e delay de leitura
        anterior = leitura;
    }
}

void serial_task(void *p){
    int received_data;
    uint8_t valor = 0;
    uint8_t tipo = 0;
    uint8_t eop = (uint8_t)0xFF;
    while(1){
        if(xQueueReceive(xQueueControle, &received_data, 0)){
            if(received_data <= 4){
                tipo = (uint8_t)3;
                valor = (uint8_t)received_data;
            }else if (received_data == 7){
                tipo = (uint8_t)2;
                valor = (uint8_t)0;
            }else if (received_data == 5 || received_data == 6){
                tipo = (uint8_t)(1 & 0xFF);
                valor = (uint8_t)(received_data-4 & 0xFF);
            }
            uart_putc_raw(uart0, (uint8_t)0xFF);
            uart_putc_raw(uart0, tipo);
            uart_putc_raw(uart0, valor);
            uart_putc_raw(uart0, eop);
            
        }
        
    }
}

int main() {
u    stdio_init_all();
    adc_init();

    uart_init(uart0, 115200);
    gpio_set_function(0, GPIO_FUNC_UART);   
    gpio_set_function(1, GPIO_FUNC_UART);

    xQueueControle = xQueueCreate(10, sizeof(int));

    xSemaphore_start = xSemaphoreCreateBinary();
    xSemaphore_save = xSemaphoreCreateBinary();
    xSemaphore_load = xSemaphoreCreateBinary();
    xSemaphore_heroi = xSemaphoreCreateBinary();

    xTaskCreate(btn_task, "Btn", 4095, NULL, 1, NULL);
    xTaskCreate(ptc_task, "Potenciometro", 4095, NULL, 1, NULL);
    // xTaskCreate(pressure_task, "Pressão", 4095, NULL, 1, NULL);
    xTaskCreate(serial_task, "Serial", 4095, NULL, 1, NULL);

    vTaskStartScheduler();

    while (true)
        ;
}
