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

const uint BTN_BLUE = 17;
const uint BTN_GREEN = 16;
const uint BTN_YELLOW = 19;
const uint BTN_RED = 20;
const uint BTN_HEROI = 21;
const uint LED = 15;

QueueHandle_t xQueueControle;
SemaphoreHandle_t xSemaphore_blue;
SemaphoreHandle_t xSemaphore_green;
SemaphoreHandle_t xSemaphore_yellow;
SemaphoreHandle_t xSemaphore_red;
SemaphoreHandle_t xSemaphore_hero;


int filtro(int value) {
    
    value = (value - 2047)/8;

    if (value >= -30 && value <= 30) {
        return 0;
    }

    if (value < -255) {
        value = -255;
    } else if (value > 255) {
        value = 255;
    }

    
    return value;
}

int16_t modulo(int16_t x) {
    return (x < 0) ? -x : x;
}

void x_task(void *p) {
    adc_init();
    adc_gpio_init(27);
    
    int vetor[5] = {0}; 
    int indice = 0;     

    
    while (1) {
        adc_select_input(1);
        int value = adc_read();
        
        vetor[indice] = value;

        int sum = 0;
        for (int i = 0; i < 5; i++) {
            sum += vetor[i];
        }
        value = sum / 5;
        
        indice++;
        if (indice >= 5) {
            indice = 0;
        }
        
        value = filtro(value);
 
        int envio;
        if (value < 0) {
            envio = 8;
        } else {
            envio = 9;
        }


        if (value != 0) {
            xQueueSend(xQueueControle, &envio, pdMS_TO_TICKS(50));
        }
        vTaskDelay(pdMS_TO_TICKS(50));
    }
}

void y_task(void *p) {
    adc_init();
    adc_gpio_init(26);
    
    int vetor[5] = {0};     
    int indice = 0;     
    
    while (1) {
        adc_select_input(0);
        int value = adc_read();
        
        vetor[indice] = value;
        
        int sum = 0;
        for (int i = 0; i < 5; i++) {
            sum += vetor[i];
        }
        value = sum / 5;
        
        indice++;
        if (indice >= 5) {
            indice = 0;
        }
        
        value = filtro(value);

        int envio;
        if (value < 0) {
            envio = 10;
        } else {
            envio = 11;
        }

        if (value != 0) {
            xQueueSend(xQueueControle, &envio, pdMS_TO_TICKS(50));
        }
        vTaskDelay(pdMS_TO_TICKS(50));
    }
}


void btn_callback(uint gpio, uint32_t events) {
    if(events == 0x4){
        if(gpio == BTN_BLUE){
            xSemaphoreGiveFromISR(xSemaphore_blue, 0);
        } else if(gpio == BTN_GREEN){
            xSemaphoreGiveFromISR(xSemaphore_green, 0);
        } else if(gpio == BTN_YELLOW){
            xSemaphoreGiveFromISR(xSemaphore_yellow, 0);
        } else if(gpio == BTN_RED){   
            xSemaphoreGiveFromISR(xSemaphore_red, 0);
        } else if(gpio == BTN_HEROI){
            xSemaphoreGiveFromISR(xSemaphore_hero, 0);
        }
    }
}

void btn_task(void *p) {
    int botao;

    gpio_init(BTN_BLUE);
    gpio_set_dir(BTN_BLUE, GPIO_IN);
    gpio_pull_up(BTN_BLUE);
    gpio_set_irq_enabled_with_callback(BTN_BLUE, GPIO_IRQ_EDGE_FALL, true, &btn_callback);

    gpio_init(BTN_GREEN);
    gpio_set_dir(BTN_GREEN, GPIO_IN);
    gpio_pull_up(BTN_GREEN);
    gpio_set_irq_enabled_with_callback(BTN_GREEN, GPIO_IRQ_EDGE_FALL, true, &btn_callback);

    gpio_init(BTN_YELLOW);
    gpio_set_dir(BTN_YELLOW, GPIO_IN);
    gpio_pull_up(BTN_YELLOW);
    gpio_set_irq_enabled_with_callback(BTN_YELLOW, GPIO_IRQ_EDGE_FALL, true, &btn_callback);

    gpio_init(BTN_RED);
    gpio_set_dir(BTN_RED, GPIO_IN);
    gpio_pull_up(BTN_RED);
    gpio_set_irq_enabled_with_callback(BTN_RED, GPIO_IRQ_EDGE_FALL, true, &btn_callback);

    gpio_init(BTN_HEROI);
    gpio_set_dir(BTN_HEROI, GPIO_IN);
    gpio_pull_up(BTN_HEROI);
    gpio_set_irq_enabled_with_callback(BTN_HEROI, GPIO_IRQ_EDGE_FALL, true, &btn_callback);
    
    while (1) {
        if(xSemaphoreTake(xSemaphore_blue, pdMS_TO_TICKS(500)) == pdTRUE) {
            botao = 1;
            xQueueSend(xQueueControle, &botao, 0); // Envia p fila se clicou no start
        }else if(xSemaphoreTake(xSemaphore_green, pdMS_TO_TICKS(500)) == pdTRUE) {
            botao = 2;
            xQueueSend(xQueueControle, &botao, 0); // Envia p fila se clicou no save
        } else if(xSemaphoreTake(xSemaphore_yellow, pdMS_TO_TICKS(500)) == pdTRUE) {
            botao = 3;
            xQueueSend(xQueueControle, &botao, 0); // Envia p fila se clicou no load
        } else if(xSemaphoreTake(xSemaphore_red, pdMS_TO_TICKS(500)) == pdTRUE) {
            botao = 4;
            xQueueSend(xQueueControle, &botao, 0); // Envia p fila se clicou no heroi
        } else if (xSemaphoreTake(xSemaphore_hero, pdMS_TO_TICKS(500)) == pdTRUE) {
            botao = 5;
            xQueueSend(xQueueControle, &botao, 0); // Envia p fila se clicou no heroi
        }
        vTaskDelay(pdMS_TO_TICKS(50)); // Debounce e delay de leitura
    }

}

void ptc_task(void *p) {
    adc_init();
    adc_gpio_init(28); 

    
    uint16_t result;
    int envio;
    uint16_t last_result = adc_read(); // Última leitura do ADC


    // 270 graus é o máximo do potenciômetro
    // O adc faz leituras de 0 a 4095
    // 15 graus é o quanto de giro é desejado para printar se foi p sentido horario ou antihorario
    // int((15/270) * 4095) = 228


    const uint16_t limiar = 228; // Limiar para ~15° (5,56% de 4095)

    while (1) {
        adc_select_input(2); 
        result = adc_read(); 

        int16_t delta = result - last_result;
        if (modulo(delta) > limiar) {
            if (delta > 0) {
                // Giro horário 
                envio = 6;
                xQueueSend(xQueueControle, &envio, 0); // Envia p fila se girou horario
            } else {
                // Giro anti-horário
                envio = 7;
                xQueueSend(xQueueControle, &envio, 0); // Envia p fila se girou anti-horario
            }
            last_result = result;
        }
        vTaskDelay(pdMS_TO_TICKS(200)); // Delay de 200ms
    }
}

void uart_task(void *p){
    int received_data;
    uint8_t valor = 0;
    uint8_t tipo = 0;
    uint8_t eop = (uint8_t)0xFF;
    //inicializar o led
    gpio_init(LED);
    gpio_set_dir(LED, GPIO_OUT);
    gpio_put(LED, 0);

    while(1){
        if(uart_is_readable(uart0)){
            uint8_t byte = uart_getc(uart0);
            if(byte == 0x21){
                gpio_put(LED, 1);
            }
        }
        if(xQueueReceive(xQueueControle, &received_data, 0)){
            if(received_data <= 5){
                tipo = (uint8_t)3;
                valor = (uint8_t)received_data;
            }else if (received_data > 7){
                tipo = (uint8_t)2;
                valor = (uint8_t)received_data-7;
            }else if (received_data == 6 || received_data == 7){
                tipo = (uint8_t)(1 & 0xFF);
                valor = (uint8_t)(received_data-5 & 0xFF);
            }
            uart_putc_raw(uart0, (uint8_t)0xFF);
            uart_putc_raw(uart0, tipo);
            uart_putc_raw(uart0, valor);
            uart_putc_raw(uart0, eop);
            
        }
        
    }
}

int main() {
    stdio_init_all();
    adc_init();

    uart_init(uart0, 115200);
    gpio_set_function(0, GPIO_FUNC_UART);   
    gpio_set_function(1, GPIO_FUNC_UART);

    xQueueControle = xQueueCreate(10, sizeof(int));

    xSemaphore_blue = xSemaphoreCreateBinary();
    xSemaphore_green = xSemaphoreCreateBinary();
    xSemaphore_yellow= xSemaphoreCreateBinary();
    xSemaphore_red = xSemaphoreCreateBinary();
    xSemaphore_hero = xSemaphoreCreateBinary();

    xTaskCreate(btn_task, "Btn", 4095, NULL, 1, NULL);
    xTaskCreate(ptc_task, "Potenciometro", 4095, NULL, 1, NULL);
    xTaskCreate(x_task, "X_task", 4095, NULL, 1, NULL);
    xTaskCreate(y_task, "Y_task", 4095, NULL, 1, NULL);
    xTaskCreate(uart_task, "Uart", 4095, NULL, 1, NULL);

    vTaskStartScheduler();

    while (true)
        ;
}
