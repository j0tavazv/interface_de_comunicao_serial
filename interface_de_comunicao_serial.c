#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "pico/stdlib.h"
#include "hardware/pio.h"
#include "hardware/clocks.h"
#include "hardware/timer.h"
#include "hardware/i2c.h"
#include "inclusao/ssd1306.h"
#include "inclusao/font.h"
#include "ws2818b.pio.h"

// Definições de constantes para pinos e configurações
#define CONTADOR_LED 25
#define PINO_MATRIZ_LED 7
#define LED_VERDE 11
#define LED_AZUL 12
#define BOTAO_A 5
#define BOTAO_B 6
#define I2C_PORTA i2c1
#define I2C_SDA 14
#define I2C_SCL 15
#define ENDERECO 0x3C
#define INTENS_LED 200

bool cor = true;
bool led_verde_ON = false;
bool led_azul_ON = false;
static volatile uint32_t last_time = 0;
static void interrupcao(uint gpio, uint32_t events);
// Estrutura para representar um pixel (LED) com componentes GRB
struct pixel_t {
    uint8_t G, R, B;
};
typedef struct pixel_t LED_da_matriz;

// Buffer para armazenar o estado de cada LED na matriz
LED_da_matriz leds[CONTADOR_LED];

// Variáveis para controle da máquina de estados PIO
PIO maquina_pio;
uint variavel_maquina_de_estado;

// Definição das coordenadas dos LEDs para cada número (0-9)
const uint8_t quantidade[10] = {12, 8, 11, 10, 9, 11, 12, 8, 13, 12};
const uint8_t coordenadas_numero[10][13] = {
    {1, 2, 3, 6, 8, 11, 13, 16, 18, 21, 22, 23}, // 0
    {1, 2, 3, 7, 12, 16, 17, 22}, // 1
    {1, 2, 3, 6, 11, 12, 13, 18, 21, 22, 23}, // 2
    {1, 2, 3, 8, 11, 12, 18, 21, 22, 23}, // 3
    {1, 8, 11, 12, 13, 16, 18, 21, 23}, // 4
    {1, 2, 3, 8, 11, 12, 13, 16, 21, 22, 23}, // 5
    {1, 2, 3, 6, 8, 11, 12, 13, 16, 21, 22, 23}, // 6
    {1, 8, 11, 16, 18, 21, 22, 23}, // 7
    {1, 2, 3, 6, 8, 11, 12, 13, 16, 18, 21, 22, 23}, // 8
    {1, 2, 3, 8, 11, 12, 13, 16, 18, 21, 22, 23} // 9
};

// Variáveis para controle de estado dos LEDs e botões
static volatile bool estado_led_azul = false, estado_led_verde = false, botao_pressionado = false;
static volatile uint32_t tempo_atual, tempo_passado = 0;

// Estrutura para controle do display SSD1306
ssd1306_t ssd;

// Função para inicializar a máquina PIO e configurar os LEDs
void inicializacao_maquina_pio(uint pino){
    uint programa_pio, i;
    programa_pio = pio_add_program(pio0, &ws2818b_program);
    maquina_pio = pio0;
    variavel_maquina_de_estado = pio_claim_unused_sm(maquina_pio, false);
    if (variavel_maquina_de_estado < 0) {
        maquina_pio = pio1;
        variavel_maquina_de_estado = pio_claim_unused_sm(maquina_pio, true);
    }
    ws2818b_program_init(maquina_pio, variavel_maquina_de_estado, programa_pio, pino, 800000.f);
    for (i = 0; i < CONTADOR_LED; ++i) {
        leds[i].R = 0;
        leds[i].G = 0;
        leds[i].B = 0;
    }
}

// Função para atribuir uma cor a um LED específico
void atribuir_cor_ao_led(const uint indice, const uint8_t r, const uint8_t g, const uint8_t b){
    leds[indice].R = r;
    leds[indice].G = g;
    leds[indice].B = b;
}

// Função para limpar o buffer de LEDs (desligar todos os LEDs)
void limpar_o_buffer(void){
    for (uint i = 0; i < CONTADOR_LED; ++i)
        atribuir_cor_ao_led(i, 0, 0, 0);
}

// Função para enviar os dados do buffer para a matriz de LEDs
void escrever_no_buffer(void){
    for (uint i = 0; i < CONTADOR_LED; ++i){
        pio_sm_put_blocking(maquina_pio, variavel_maquina_de_estado, leds[i].G);
        pio_sm_put_blocking(maquina_pio, variavel_maquina_de_estado, leds[i].R);
        pio_sm_put_blocking(maquina_pio, variavel_maquina_de_estado, leds[i].B);
    }
    sleep_us(100); // Tempo de espera para resetar a matriz de LEDs
}

// Função principal
int main(void){
    char caractere_digitado;

    // Inicializa a máquina PIO e os LEDs
    inicializacao_maquina_pio(PINO_MATRIZ_LED);
    limpar_o_buffer();
    escrever_no_buffer();

    // Inicializa a comunicação serial e os pinos
    stdio_init_all();
    inicializacao_dos_pinos();

    // Configura interrupções para os botões
    gpio_set_irq_enabled_with_callback(BOTAO_A, GPIO_IRQ_EDGE_FALL, true, &interrupcao);
    gpio_set_irq_enabled_with_callback(BOTAO_B, GPIO_IRQ_EDGE_FALL, true, &interrupcao);

    // Inicializa e configura o display SSD1306
    ssd1306_init(&ssd, WIDTH, HEIGHT, false, ENDERECO, I2C_PORTA);
    ssd1306_config(&ssd);
    ssd1306_send_data(&ssd);
    ssd1306_fill(&ssd, false);
    ssd1306_send_data(&ssd);

    // Loop principal
    while(true){
        printf("Caractere: ");
        if(scanf("%c", &caractere_digitado) == 1){
            printf("Caractere digitado: %c\n", caractere_digitado);
            interpretacao_do_caractere(caractere_digitado);
            if(caractere_digitado >= 48 && caractere_digitado <= 57){
                manipulacao_matriz_led(caractere_digitado); // Exibe o número na matriz de LEDs
            }else{
                limpar_o_buffer(); // Apaga a matriz de LEDs se o caractere não for um número
                escrever_no_buffer();
            }
        }
        sleep_ms(100); // Delay para evitar uso excessivo do processador
    }

    return 0;
}

// Função para inicializar os pinos GPIO
void inicializacao_dos_pinos(void){
    gpio_init(LED_AZUL);
    gpio_init(LED_VERDE);
    gpio_set_dir(LED_AZUL, GPIO_OUT);
    gpio_set_dir(LED_VERDE, GPIO_OUT);
    gpio_put(LED_AZUL, false);
    gpio_put(LED_VERDE, false);

    gpio_init(BOTAO_A);
    gpio_init(BOTAO_B);
    gpio_set_dir(BOTAO_A, GPIO_IN);
    gpio_set_dir(BOTAO_B, GPIO_IN);
    gpio_pull_up(BOTAO_A);
    gpio_pull_up(BOTAO_B);

    i2c_init(I2C_PORTA, 400 * 1000);
    gpio_set_function(I2C_SDA, GPIO_FUNC_I2C);
    gpio_set_function(I2C_SCL, GPIO_FUNC_I2C);
    gpio_pull_up(I2C_SDA);
    gpio_pull_up(I2C_SCL);
}

// Função para interpretar o caractere digitado e exibir no display
void interpretacao_do_caractere(char caractere){
    char mensagem[2][20] = {"Caractere ", "Numero "};
    uint registro_de_tipo;
    if(caractere >= 'A' && caractere <= 'Z' || caractere >= 'a' && caractere <= 'z'){
        registro_de_tipo = 0;
        mensagem[0][10] = caractere;
    }else if(caractere >= '0' && caractere <= '9'){
        registro_de_tipo = 1;
        mensagem[1][7] = caractere;
    }
    ssd1306_fill(&ssd, false);
    ssd1306_rect(&ssd, 3, 3, 122, 58, true, false);
    ssd1306_draw_string(&ssd, mensagem[registro_de_tipo], 8, 10);
    ssd1306_send_data(&ssd);
}

// Função para manipular a matriz de LEDs com base no número digitado
void manipulacao_matriz_led(int numero){
    uint num = numero - 48; // Converte o caractere para o valor numérico correspondente
    limpar_o_buffer();
    for(uint i = 0; i < quantidade[num]; i++)
        atribuir_cor_ao_led(coordenadas_numero[num][i], 0, 0, INTENS_LED); // Atribui cor aos LEDs correspondentes
    escrever_no_buffer();
}
void interrupcao(uint gpio, uint32_t events)
{
  uint32_t current_time = to_us_since_boot(get_absolute_time());
  if (current_time - last_time > 500000) // Debouncing de 500ms
  {
    last_time = current_time;

    if (gpio == BOTAO_A) // Botão A pressionado
    {
      led_verde_ON = !led_verde_ON;
      gpio_put(LED_VERDE, led_verde_ON);
      cor = !cor;
      ssd1306_fill(&ssd, !cor);
      ssd1306_rect(&ssd, 3, 3, 122, 58, cor, !cor);
      ssd1306_draw_string(&ssd, "    Botao A", 8, 10);
      ssd1306_draw_string(&ssd, "Pressionado", 20, 30);
      ssd1306_draw_string(&ssd, led_verde_ON ? "LED Verde ON" : "LED Verde OFF", 15, 48);
      ssd1306_send_data(&ssd);
      printf("Botão A pressionado e Led Verde %s\n", led_verde_ON ? "ligado" : "desligado");
    }
    if (gpio == BOTAO_B) // Botão B pressionado
    {
      led_azul_ON = !led_azul_ON;
      gpio_put(LED_AZUL, led_azul_ON);
      cor = !cor;
      ssd1306_fill(&ssd, !cor);
      ssd1306_rect(&ssd, 3, 3, 122, 58, cor, !cor);
      ssd1306_draw_string(&ssd, "    Botao B", 8, 10);
      ssd1306_draw_string(&ssd, "Pressionado", 20, 30);
      ssd1306_draw_string(&ssd, led_azul_ON ? "LED Azul ON" : "LED Azul OFF", 11, 48);
      ssd1306_send_data(&ssd);
      printf("Botão B pressionado e Led Azul %s\n", led_azul_ON ? "ligado" : "desligado");
    }
}
}