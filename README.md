Projeto de Controle de LEDs e Display com Raspberry Pi Pico
Este projeto utiliza um Raspberry Pi Pico para controlar uma matriz de LEDs NeoPixel, dois LEDs individuais (verde e azul), dois botões e um display OLED SSD1306. O sistema permite a interação do usuário por meio de botões e entrada de caracteres via terminal serial, exibindo informações no display e na matriz de LEDs.

Funcionalidades
Matriz de LEDs NeoPixel:

Exibe números de 0 a 9 na matriz de LEDs com base na entrada do usuário.

Utiliza a biblioteca PIO para controle dos LEDs.

LEDs Individuais:

Dois LEDs (verde e azul) podem ser ligados/desligados por meio dos botões A e B.

O estado dos LEDs é exibido no display OLED.

Botões:

Dois botões (A e B) controlam os LEDs verde e azul, respectivamente.

Implementa debouncing para evitar leituras falsas.

Display OLED SSD1306:

Exibe mensagens sobre o estado dos LEDs e os botões pressionados.

Mostra o caractere digitado pelo usuário (letra ou número).

Entrada Serial:

O usuário pode digitar caracteres no terminal serial.

Se o caractere for um número (0-9), ele é exibido na matriz de LEDs.

Se for uma letra, uma mensagem é exibida no display.

Hardware Necessário
Raspberry Pi Pico

Matriz de LEDs NeoPixel (25 LEDs)

Dois LEDs (verde e azul)

Dois botões

Display OLED SSD1306 (128x64 pixels)

Resistores e jumpers para conexões

Conexões
Componente	Pino no Raspberry Pi Pico
Matriz de LEDs	GPIO 7
LED Verde	GPIO 11
LED Azul	GPIO 12
Botão A	GPIO 5
Botão B	GPIO 6
Display SDA	GPIO 14
Display SCL	GPIO 15
Bibliotecas Utilizadas
pico/stdlib.h: Biblioteca padrão do Raspberry Pi Pico.

hardware/pio.h: Para controle da matriz de LEDs NeoPixel via PIO.

hardware/i2c.h: Para comunicação I2C com o display OLED.

inclusao/ssd1306.h: Biblioteca para controle do display SSD1306.

inclusao/font.h: Biblioteca de fontes para o display.

Como Usar
Conecte o hardware conforme a tabela de conexões.

Compile e carregue o código no Raspberry Pi Pico.

Abra o terminal serial (por exemplo, usando minicom ou screen).

Digite um caractere:

Se for um número (0-9), ele será exibido na matriz de LEDs.

Se for uma letra, uma mensagem será exibida no display.

Pressione os botões A e B para alternar o estado dos LEDs verde e azul, respectivamente. O estado dos LEDs será exibido no display.

Exemplo de Funcionamento
Digite 5 no terminal serial:

O número 5 será exibido na matriz de LEDs.

O display mostrará "Numero 5".

Pressione o botão A:

O LED verde será ligado/desligado.

O display mostrará "Botão A pressionado" e o estado do LED verde.

Pressione o botão B:

O LED azul será ligado/desligado.

O display mostrará "Botão B pressionado" e o estado do LED azul.

Estrutura do Código
inicializacao_maquina_pio: Configura a máquina PIO para controle da matriz de LEDs.

atribuir_cor_ao_led: Define a cor de um LED específico.

limpar_o_buffer: Desliga todos os LEDs da matriz.

escrever_no_buffer: Envia os dados do buffer para a matriz de LEDs.

interrupcao: Trata as interrupções dos botões, alternando os LEDs e atualizando o display.

interpretacao_do_caractere: Exibe o caractere digitado no display.

manipulacao_matriz_led: Exibe o número correspondente na matriz de LEDs.


Como Compilar e Executar
Instale o Raspberry Pi Pico SDK.

Clone este repositório ou copie o código para o seu ambiente de desenvolvimento.

Compile o código usando o SDK:

bash
Copy
mkdir build
cd build
cmake ..
make
Conecte o Raspberry Pi Pico no modo de bootloader e copie o arquivo .uf2 gerado para o dispositivo.
