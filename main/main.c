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

const uint BTN_1_OLED = 28;
const uint BTN_2_OLED = 26;
const uint BTN_3_OLED = 27;

const uint LED_1_OLED = 20;
const uint LED_2_OLED = 21;
const uint LED_3_OLED = 22;


QueueHandle_t xQueueADC;
int16_t modulo(int16_t x) {
    return (x < 0) ? -x : x;
}



// void adc_task(void *p) {
//     adc_init();
//     adc_gpio_init(27); 

    
//     uint16_t result;
//     uint16_t envio;
//     uint16_t last_result = adc_read(); // Última leitura do ADC


//     // 270 graus é o máximo do potenciômetro
//     // O adc faz leituras de 0 a 4095
//     // 15 graus é o quanto de giro é desejado para printar se foi p sentido horario ou antihorario
//     // int((15/270) * 4095) = 228


//     const uint16_t limiar = 228; // Limiar para ~15° (5,56% de 4095)

//     while (1) {
//         adc_select_input(1); 
//         result = adc_read(); 

//         int16_t delta = result - last_result;
//         if (modulo(delta) > limiar) {
//             if (delta > 0) {
//                 // Giro horário 
//                 printf("horario\n");
//                 envio = 1;
//                 //xQueueSend(xQueueADC, &envio, 0); // Envia p fila se girou horario
//             } else {
//                 // Giro anti-horário
//                 printf("anti-horario\n");
//                 envio = 2;
//                 //xQueueSend(xQueueADC, &envio, 0); // Envia p fila se girou anti-horario
//             }
//             last_result = result;
//         }

        
        

//         vTaskDelay(pdMS_TO_TICKS(200)); // Delay de 200ms
//     }
// }

void adc_task(void *p) {
    adc_init();
    adc_gpio_init(27); // SoftPot ligado ao GPIO27 (ADC1)

    uint16_t leitura;
    uint16_t limiar_toque = 200; // Se estiver abaixo, consideramos "sem toque"
    bool pressionado = false;

    while (1) {
        adc_select_input(1); // GPIO27 → ADC1
        leitura = adc_read(); // Valor entre 0 e 4095

        if (!pressionado && leitura > limiar_toque) {
            printf("clicou\n");
            pressionado = true;
        }

        // Espera o toque ser liberado para detectar de novo
        if (pressionado && leitura <= limiar_toque) {
            pressionado = false;
        }

        vTaskDelay(pdMS_TO_TICKS(50)); // Debounce e delay de leitura
    }
}




int main() {
    stdio_init_all();

    xTaskCreate(adc_task, "Potenciometro", 4095, NULL, 1, NULL);

    //xQueueADC = xQueueCreate(10, sizeof(adc_t));

    vTaskStartScheduler();

    while (true)
        ;
}
