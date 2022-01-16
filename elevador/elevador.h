// arquivo com macros

#define BUFFER_IN                            5
#define BUFFER_OUT                           10
#define TAMANHO_DA_MAIOR_MENSAGEM            12
#define DEF_TRUE                             1
#define DEF_FALSE                            0

// ID das mensagens
#define ID_MENSAGEM_SEM_MENSAGEM             0
#define ID_MENSAGEM_INICIALIZA               1
#define ID_MENSAGEM_ABRE                     10
#define ID_MENSAGEM_FECHA                    11
#define ID_MENSAGEM_SOBE                     12
#define ID_MENSAGEM_DESCE                    13
#define ID_MENSAGEM_PARA                     14
#define ID_MENSAGEM_CONSULTA                 15
#define ID_MENSAGEM_LIGA_BOTAO               16
#define ID_MENSAGEM_DESLIGA_BOTAO            17


// conteudo das mensagens
#define MENSAGEM_ELEVADOR_ESQUERDO           "e"
#define MENSAGEM_ELEVADOR_CENTRO             "c"
#define MENSAGEM_ELEVADOR_DIREITO            "d"
#define MENSAGEM_INICIALIZA                  "r"
#define MENSAGEM_ABRE                        "a"
#define MENSAGEM_FECHA                       "f"
#define MENSAGEM_SOBE                        "s"
#define MENSAGEM_DESCE                       "d"
#define MENSAGEM_PARA                        "p"
#define MENSAGEM_CONSULTA                    "x"
#define MENSAGEM_LIGAR_BOTAO                 "L"
#define MENSAGEM_DESLIGAR_BOTAO              "D"

// flags
#define FLAG_ENVIAR_MENSAGEM_GERAL           0x00000001
#define FLAG_ENVIAR_MENSAGEM_ESQUERDO        0x00000002
#define FLAG_ENVIAR_MENSAGEM_CENTRAL         0x00000004
#define FLAG_ENVIAR_MENSAGEM_DIREITO         0x00000008


// estrturas
typedef struct
{
   char conteudo[11];    // comando com letras e nums
   unsigned char nulo;   // esta vazio
}Mensagem; 

typedef struct
{
   osThreadId_t ID;     // id do elevador
   char status_mov;     // p = parado
                        // s = subindo
                        // d = descendo
  
   int andar_atual;     // registro do andar
   
   int fila_req[16];    // fila das requisicoes (pior caso = todos andares)
   int fila_indice;     // qual é o prox andar a ser tratado
  
   char status_porta;   // a = aberta
                        // f = fechada
   int tam_fila;        // tamanho da fila
   void* buffer_in;
   int index;
}Elevador;              // struct Elevador  

// funções
void app_elevador();
void app_uart_receive(void *arg);
void app_uart_transmit(void *arg);
void addFila(int valor, int* vetor, int tam, char sent, int andar_atual);
void removePrimeiroFila(int* vetor, int tam);
int botaoInternoNum(char value);
int decodAndar(char* msg);
void controle_mensagens(Elevador* elevador, void* buffer_in,int* index);
