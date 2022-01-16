#include "system_tm4c1294.h"           // CMSIS-Core
#include "driver_uart.h"               // device drivers
#include "cmsis_os2.h"                 // CMSIS-RTOS
#include <string.h>
#include "elevador.h"

osThreadId_t pol_uart_id;
osThreadId_t send_uart_id;
osThreadId_t elevador_esq_id;
osThreadId_t elevador_cen_id;
osThreadId_t elevador_dir_id;
osThreadId_t controle_esq_id;
osThreadId_t controle_cen_id;
osThreadId_t controle_dir_id;

osTimerId_t timer2_id;

// variáveis globais
Elevador elevador_esq;
Elevador elevador_cen;
Elevador elevador_dir;

static Mensagem buffer_in_E[BUFFER_IN];
static Mensagem buffer_in_C[BUFFER_IN];
static Mensagem buffer_in_D[BUFFER_IN];

// variável que guarda a próxima mensagem a enviar
static unsigned char gucMensagemAEnviar = 0;  

int decodAndar(char* msg)
{
   char aux[3];
   aux[0] = msg[1];
   aux[1] = msg[2];
   aux[2] = '\0';

   if(strcmp(aux, "0\r") == 0)
      return 0;
   else if(strcmp(aux, "1\r") == 0)
      return 1;
   else if(strcmp(aux, "2\r") == 0)
      return 2;
   else if(strcmp(aux, "3\r") == 0)
      return 3;
   else if(strcmp(aux, "4\r") == 0)
      return 4;
   else if(strcmp(aux, "5\r") == 0)
      return 5;
   else if(strcmp(aux, "6\r") == 0)
      return 6;
   else if(strcmp(aux, "7\r") == 0)
      return 7;
   else if(strcmp(aux, "8\r") == 0)
      return 8;
   else if(strcmp(aux, "9\r") == 0)
      return 9;
   else if(strcmp(aux, "10") == 0)
      return 10;
   else if(strcmp(aux, "11") == 0)
      return 11;
   else if(strcmp(aux, "12") == 0)
      return 12;
   else if(strcmp(aux, "13") == 0)
      return 13;
   else if(strcmp(aux, "14") == 0)
      return 14;
   else if(strcmp(aux, "15") == 0)
      return 15;

   return -1;
}

int botaoInternoNum(char value)
{
   switch(value)
   {
      case 'a':
         return 0;
      case 'b':
         return 1;
      case 'c':
         return 2;
      case 'd':
         return 3;
      case 'e':
         return 4;
      case 'f':
         return 5;
      case 'g':
         return 6;
      case 'h':
         return 7;
      case 'i':
         return 8;
      case 'j':
         return 9;
      case 'k':
         return 10;
      case 'l':
         return 11;
      case 'm':
         return 12;
      case 'n':
         return 13;
      case 'o':
         return 14;
      case 'p':
         return 15;
    }
   return -1;
}

void app_controle_mensagens(void *arg)
{
   Elevador* elevador = arg;
   int aux;
   int* index = &(elevador->index);
   
   Mensagem* buffer = (Mensagem*)elevador->buffer_in;

   while(DEF_TRUE)
   {
      if(!buffer[*index].nulo) 
      {
         if(buffer[*index].conteudo[1] == 'I')   // botao interno pressionado
         {
            aux = botaoInternoNum(buffer[*index].conteudo[2]);
            
            if(aux != -1) // caso o valor seja valido
            {
              addFila(aux, elevador->fila_req, elevador->tam_fila, elevador->status_mov, elevador->andar_atual);
            }
         }
         else if((buffer[*index].conteudo[1] - '0') < 10)    // andar atual
         {
            //atualiza o elevador atual
            elevador->andar_atual = decodAndar(buffer[*index].conteudo);

            // seta a flag
            osThreadFlagsSet(elevador->ID, 0x0001);
         }
         else if(buffer[*index].conteudo[1] == 'E')    // andar atual
         {
            int dezena = (int)(buffer[*index].conteudo[2] - '0');
            int unidade = (int)(buffer[*index].conteudo[3] - '0');
            int num = (dezena*10) + unidade;
            
            if(buffer[*index].conteudo[4] == 's' && elevador->status_mov == 's')
               addFila(num, elevador->fila_req, elevador->tam_fila, 's', elevador->andar_atual);

            else
               addFila(num, elevador->fila_req, elevador->tam_fila, 'd', elevador->andar_atual);

            // seta a flag
            osThreadFlagsSet(elevador->ID, 0x0001);
         }
         else if(buffer[*index].conteudo[1] == 'A')  // porta ABERTA
         {
            // seta a flag
            osThreadFlagsSet(elevador->ID, 0x0001);
         }
         else if(buffer[*index].conteudo[1] == 'F')  // porta FECHADA
         {
            // seta a flag
            osThreadFlagsSet(elevador->ID, 0x0001);
         }
         buffer[*index].nulo = 1;
         (*index)++;
         if(*index == BUFFER_IN)
          *index = 0;
      }
      else
      {
         osThreadFlagsWait(0x0001, osFlagsWaitAny, osWaitForever);
      }
   }
}        

/*
    O vetor de entrada deve ter -1 em posições não preenchidas
*/
void addFila(int valor, int* vetor, int tam, char sent, int andar_atual)
{
   int aux;
   int finalFila=0;
   if(sent == 's' || sent == 'p') // se estiver subindo ou parado
   {
      if(vetor[0] > valor && andar_atual > valor)
         finalFila = 1;

      for(int i=0; i < tam; i++)
      {
         if(vetor[i] == -1)
         {
            vetor[i] = valor;
            break;
         }

         if(!finalFila && vetor[i] > valor)
         {
            aux = vetor[i];
            vetor[i] = valor;
            valor = aux;
         }
         else if(vetor[i] == valor)
            break;
      }
   }
   else if(sent == 'd')    // se estiver descendo
   {
      if(vetor[0] < valor && andar_atual < valor)
         finalFila = 1;

      for(int i=0; i < tam; i++)
      {
         if(vetor[i] == -1)
         {
            vetor[i] = valor;
            break;
         }

         if(!finalFila && vetor[i] < valor)
         {
            aux = vetor[i];
            vetor[i] = valor;
            valor = aux;
         }
         else if(vetor[i] == valor)
            break;
      }
   }
}

void removePrimeiroFila(int* vetor, int tam)
{
   for(int i = 0; i < tam; i++)
   {
      if(vetor[i] == -1 && vetor[i+1] == -1)
         break;

      vetor[i] = vetor[i+1];
   }
}

void app_elevador(void *arg)
{
   int estado = 3;
   Elevador* elevador = arg;

   // init elevador
   elevador->status_mov = 'p';
   elevador->status_porta = 'f';
   elevador->andar_atual = 0;
   elevador->fila_indice = 0;
   elevador->tam_fila = 16;
   elevador->index = 0;

   uint32_t flag = 0x00000000;
   for(int i = 0; i < 16; i++)
   {
      elevador->fila_req[i] = -1;  // -1 sera o indicador de vazio
   }

   // define qual a sua flag
   if(elevador->ID == elevador_esq_id)
   {
      flag = FLAG_ENVIAR_MENSAGEM_ESQUERDO;
      elevador->buffer_in = &buffer_in_E;
   }
   else if(elevador->ID == elevador_cen_id)
   {
      flag = FLAG_ENVIAR_MENSAGEM_CENTRAL;
      elevador->buffer_in = &buffer_in_C;
   }
   else if(elevador->ID == elevador_dir_id)
   {
      flag = FLAG_ENVIAR_MENSAGEM_DIREITO;
      elevador->buffer_in = &buffer_in_D;
   }

   while(DEF_TRUE)
   {
      if(estado == 0) // PARADO
      {
         if(elevador->fila_req[0] > -1)
         {
            if(elevador->andar_atual > elevador->fila_req[0])
            {
               estado = 2;
               elevador->status_mov = 'd';
            }
            else if(elevador->andar_atual < elevador->fila_req[0])
            {
               estado = 1;
               elevador->status_mov = 's';
            }
            else
            {
               removePrimeiroFila(elevador->fila_req, elevador->tam_fila);
               estado = 3;
            }
         }
      }
      else if(estado == 1)    // SUBIR
      {
         // comando para subir ao andar elevador.fila_req[0]
         // sendUart(COMANDO_PARA_SUBIR);
         gucMensagemAEnviar = ID_MENSAGEM_SOBE;
         osThreadFlagsSet(send_uart_id, flag);

         while(1)
         {
            osThreadFlagsWait(0x0001, osFlagsWaitAny, osWaitForever);
            if(elevador->fila_req[0] == elevador->andar_atual) 
            {
               break;
            }
         }
         gucMensagemAEnviar = ID_MENSAGEM_PARA;
         osThreadFlagsSet(send_uart_id, flag);
         // se chegou
         estado = 3;
         removePrimeiroFila(elevador->fila_req, elevador->tam_fila);
         osDelay(500);
      }
      else if(estado == 2)    // DESCER
      {
         // comando para descer ao andar elevador.fila_req[0]
         // sendUart(COMANDO_PARA_DESCER);
         gucMensagemAEnviar = ID_MENSAGEM_DESCE;
         osThreadFlagsSet(send_uart_id, flag);

         while(1)
         {
            osThreadFlagsWait(0x0001, osFlagsWaitAny, osWaitForever);
            if(elevador->fila_req[0] == elevador->andar_atual) 
            {
               break;
            }
         }
         gucMensagemAEnviar = ID_MENSAGEM_PARA;
         osThreadFlagsSet(send_uart_id, flag);
         // se chegou
         estado = 3;
         removePrimeiroFila(elevador->fila_req, elevador->tam_fila);
         osDelay(500);
      }
      else if(estado == 3)    // ABRIR PORTA
      {
         // comando para iniciar a abertura
         // sendUart(COMANDO_ABRIR);
         gucMensagemAEnviar = ID_MENSAGEM_ABRE;
         osThreadFlagsSet(send_uart_id, flag);

         // enquanto nao abrir espera flag
         osThreadFlagsWait(0x0001, osFlagsWaitAny, osWaitForever);

         // espera um tempo
         osDelay(3000);
         osThreadFlagsClear(0x0001);

         estado = 4;
         elevador->status_porta = 'a';
      }
      else if(estado == 4)    // FECHAR PORTA
      {
         // comando para fechar
         // sendUart(COMANDO_FECHAR_PORTA);
         gucMensagemAEnviar = ID_MENSAGEM_FECHA;
         osThreadFlagsSet(send_uart_id, flag);

         // enquanto nao fechar espera flag
         osThreadFlagsWait(0x0001, osFlagsWaitAny, osWaitForever);

         estado = 0;
         elevador->status_porta = 'f';
      }
   }   
}

static char received[TAMANHO_DA_MAIOR_MENSAGEM];
int* buffer_in_id;
Mensagem *mensagem_a_distribuir;

static int buffer_inE_id = 0;
static int buffer_inC_id = 0;
static int buffer_inD_id = 0;

void app_uart_receive(void *arg)
{
   // inicializa os buffers de entrada
   for(int i = 0; i < BUFFER_IN; i++)
   {
      buffer_in_E[i].nulo = 1;
      buffer_in_C[i].nulo = 1;
      buffer_in_D[i].nulo = 1;
   }
  
   while(1)
   {
      // recebe toda a mensagem
      for(int i = 0; i < TAMANHO_DA_MAIOR_MENSAGEM; i++)         // itera ate pegar toda msg
      {
         received[i] = receiveUart();
         if(received[i] == 0xD)           // para com o end file definido
         break;
      }

      // distribui as mensagens para as respectivas tarefas
      // caso de receber "initialized"
      if(received[0] == 'i')
      {
         gucMensagemAEnviar = ID_MENSAGEM_INICIALIZA;
         osThreadFlagsSet(send_uart_id, FLAG_ENVIAR_MENSAGEM_GERAL);
         continue;
      }
      
      // decide para qual fila a mensagem ira
      if(received[0] == 'e')
      {
         buffer_in_id = &buffer_inE_id;
         mensagem_a_distribuir = &buffer_in_E[*buffer_in_id];
      }
      else if(received[0] == 'c')
      {
         buffer_in_id = &buffer_inC_id;
         mensagem_a_distribuir = &buffer_in_C[*buffer_in_id];
      }
      else if(received[0] == 'd')
      {
         buffer_in_id = &buffer_inD_id;
         mensagem_a_distribuir = &buffer_in_D[*buffer_in_id];
      }
      
      // copia a string recem chegada para o buffer
      strcpy(mensagem_a_distribuir->conteudo, received);
      mensagem_a_distribuir->nulo = 0;                         // sinaliza que ha algo
      (*buffer_in_id)++;

      if(*buffer_in_id == BUFFER_IN)             
      {
         *buffer_in_id = 0;                          
      }

      // decide qual controle acordar
      if(received[0] == 'e')
      {
         osThreadFlagsSet(controle_esq_id, 0x0001);
      }
      else if(received[0] == 'c')
      {
         osThreadFlagsSet(controle_cen_id, 0x0001);
      }
      else if(received[0] == 'd')
      {
         osThreadFlagsSet(controle_dir_id, 0x0001);
      }
   }
}

void app_uart_transmit(void *arg)
{
  uint32_t flag_recebida = 0x00000000;
  char* elevador_a_comandar = "z";
  
   while(1)
   {
      flag_recebida = osThreadFlagsWait(0x0000000F, osFlagsWaitAny, osWaitForever);

      if(flag_recebida == FLAG_ENVIAR_MENSAGEM_GERAL)
      {
         sendUart(MENSAGEM_ELEVADOR_ESQUERDO);  
         sendUart(MENSAGEM_INICIALIZA);                     // conteudo da msg
         sendUart("\r");                                    // sinaliza o termino da msg
         sendUart(MENSAGEM_ELEVADOR_CENTRO);  
         sendUart(MENSAGEM_INICIALIZA);                     // conteudo da msg
         sendUart("\r");                                    // sinaliza o termino da msg
         sendUart(MENSAGEM_ELEVADOR_DIREITO);  
         sendUart(MENSAGEM_INICIALIZA);                     // conteudo da msg
         sendUart("\r");                                    // sinaliza o termino da msg
         gucMensagemAEnviar = ID_MENSAGEM_SEM_MENSAGEM;
      }
      else if(flag_recebida == FLAG_ENVIAR_MENSAGEM_ESQUERDO)
      {
         elevador_a_comandar = MENSAGEM_ELEVADOR_ESQUERDO;
      }
      else if(flag_recebida == FLAG_ENVIAR_MENSAGEM_CENTRAL)
      {
         elevador_a_comandar = MENSAGEM_ELEVADOR_CENTRO;
      }
      else if(flag_recebida == FLAG_ENVIAR_MENSAGEM_DIREITO)
      {
         elevador_a_comandar = MENSAGEM_ELEVADOR_DIREITO;
      }
       
      switch(gucMensagemAEnviar)
      {
         case ID_MENSAGEM_ABRE:
         {
            sendUart(elevador_a_comandar);  
            sendUart(MENSAGEM_ABRE);                           // conteudo da msg
            sendUart("\r");                                    // sinaliza o termino da msg
            gucMensagemAEnviar = ID_MENSAGEM_SEM_MENSAGEM;
         }
         break;

         case ID_MENSAGEM_FECHA:
         {
            sendUart(elevador_a_comandar);  
            sendUart(MENSAGEM_FECHA);                           // conteudo da msg
            sendUart("\r");                                     // sinaliza o termino da msg
            gucMensagemAEnviar = ID_MENSAGEM_SEM_MENSAGEM;
         }
         break;

         case ID_MENSAGEM_SOBE:
         {
            sendUart(elevador_a_comandar);  
            sendUart(MENSAGEM_SOBE);                           // conteudo da msg
            sendUart("\r");                                    // sinaliza o termino da msg
            gucMensagemAEnviar = ID_MENSAGEM_SEM_MENSAGEM;
         }
         break;

         case ID_MENSAGEM_DESCE:
         {
            sendUart(elevador_a_comandar);  
            sendUart(MENSAGEM_DESCE);                          // conteudo da msg
            sendUart("\r");                                    // sinaliza o termino da msg
            gucMensagemAEnviar = ID_MENSAGEM_SEM_MENSAGEM;
         }
         break;

         case ID_MENSAGEM_CONSULTA:
         {
            sendUart(elevador_a_comandar);  
            sendUart(MENSAGEM_CONSULTA);                       // conteudo da msg
            sendUart("\r");                                    // sinaliza o termino da msg
            gucMensagemAEnviar = ID_MENSAGEM_SEM_MENSAGEM;
         }
         break;

         case ID_MENSAGEM_PARA:
         {
            sendUart(elevador_a_comandar);  
            sendUart(MENSAGEM_PARA);                           // conteudo da msg
            sendUart("\r");                                    // sinaliza o termino da msg
            gucMensagemAEnviar = ID_MENSAGEM_SEM_MENSAGEM;
         }
         break;            
        
         default:
         break;
      }                 
   }
}

void main(void)
{
   SystemInit();
   UARTInit();
  
   osKernelInitialize();
  
   //timer2_id = osTimerNew(callback_uart, osTimerPeriodic, NULL, NULL);
  
   pol_uart_id = osThreadNew(app_uart_receive, NULL, NULL);
   send_uart_id = osThreadNew(app_uart_transmit, NULL, NULL);
   elevador_esq_id = osThreadNew(app_elevador, (void*)&elevador_esq, NULL);
   elevador_cen_id = osThreadNew(app_elevador, (void*)&elevador_cen, NULL);
   elevador_dir_id = osThreadNew(app_elevador, (void*)&elevador_dir, NULL);
   controle_esq_id = osThreadNew(app_controle_mensagens, (void*)&elevador_esq, NULL);
   controle_cen_id = osThreadNew(app_controle_mensagens, (void*)&elevador_cen, NULL);
   controle_dir_id = osThreadNew(app_controle_mensagens, (void*)&elevador_dir, NULL);

   elevador_esq.ID = elevador_esq_id;
   elevador_cen.ID = elevador_cen_id;
   elevador_dir.ID = elevador_dir_id;

   if(osKernelGetState() == osKernelReady)
      osKernelStart();

   osDelay(osWaitForever);
} // main
