#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#define TENTATIVI_MAX 7

const char MESSAGE_WELCOME[] = "OK benvenuti nel server GIAVAZZI - PIDELLO\n";
const char MESSAGE_MINOR[] = "NO +\n";
const char MESSAGE_MAJOR[] = "NO -\n";
const char MESSAGE_CORRECT[] = "SI complimenti HAI INDOVINATO !!!\n";
const char MESSAGE_ERROR[] = "ER valore NON ACCETTABILE dal server\n";
const char MESSAGE_ERROR_TENTATIVI[] = "ER numero di tentativi esauriti, RITENTA sarai piu' fortunato\n";

int main(int argc, char *argv[]) {
    int simpleSocket = 0;
    int simplePort = 0;
    int returnStatus = 0;
    struct sockaddr_in simpleServer;
    int random=0;
    int tentativo=0;
    int valore_client=0;/**variabile in cui viene registrato il numero fornito dal cliente come tentativo*/
    int i=0;
    int end=0; /**flag per intrrompere il ciclo di verifica del valore fornito dal client se indovinato o se il valore non e' accettabile*/
    char buffer[32] = "";
    char output[64] = "";

    if (2 != argc) {
      fprintf(stderr, "Usage: %s <port>\n", argv[0]);
      exit(1);
    }

    simpleSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

    if (simpleSocket == -1) {
      fprintf(stderr, "Could not create a socket!\n");
      exit(1);
    }
    else {
	    fprintf(stderr, "Socket created!\n");
    }

    /* retrieve the port number for listening */
    simplePort = atoi(argv[1]);

    /* setup the address structure */
    /* use INADDR_ANY to bind to all local addresses  */
    memset(&simpleServer, '\0', sizeof(simpleServer));
    simpleServer.sin_family = AF_INET;
    simpleServer.sin_addr.s_addr = htonl(INADDR_ANY);
    simpleServer.sin_port = htons(simplePort);

    /*  bind to the address and port with our socket  */
    returnStatus = bind(simpleSocket,(struct sockaddr *)&simpleServer,sizeof(simpleServer));

    if (returnStatus == 0) {
	    fprintf(stderr, "Bind completed!\n");
    }
    else {
      fprintf(stderr, "Could not bind to address!\n");
      close(simpleSocket);
      exit(1);
    }

    /* lets listen on the socket for connections      */
    returnStatus = listen(simpleSocket, 5);

    if (returnStatus == -1) {
      fprintf(stderr, "Cannot listen on socket!\n");
      close(simpleSocket);
      exit(1);
    }

    while (1)
    {
      struct sockaddr_in clientName = { 0 };
      int simpleChildSocket = 0;
      int clientNameLength = sizeof(clientName);

      /* wait here */

      simpleChildSocket = accept(simpleSocket,(struct sockaddr *)&clientName, &clientNameLength);
      if (simpleChildSocket == -1) {
        fprintf(stderr, "Cannot accept connections!\n");
        close(simpleSocket);
        exit(1);
      }
	     /* wait here */


        /* handle the new connection request  */
	/* write out our message to the client */
      write(simpleChildSocket, MESSAGE_WELCOME, strlen(MESSAGE_WELCOME));
      srand(time(NULL)); /**modifico il seme di random*/
      random = rand()%100 + 1; /**genero il numero casuale compreso tra 1 e 100*/
      //printf("Numero da indovinare : %d\n", random);//debug
      while(end==0){
        //printf("Inizio ciclo dei tentativi con tentativo %d\n", tentativo);//debug
        memset(buffer, '\0', sizeof(buffer));
        returnStatus = read(simpleChildSocket, buffer, sizeof(buffer));
        //printf("4\n");//debug
        //printf("returnStatus: %d\n", returnStatus);//debug
        //printf("%s\n", buffer);//debug
        valore_client=atoi(buffer);
        if (returnStatus > 0){

          if(valore_client==random){/** ramo in cui il numero e' corretto*/
            //printf("ramo corretto\n");//debug
            strcpy(output, MESSAGE_CORRECT);
            end=1;
          }
          else if(valore_client> 0 && valore_client<random){/** ramo in cui il numero e' minore di random*/
            //printf("ramo minore\n");//debug
            if(tentativo==TENTATIVI_MAX){
              end = 1;
              //printf("tentativi massimi raggiunti\n");//debug
              strcpy(output, MESSAGE_ERROR_TENTATIVI);
            }
            else strcpy(output, MESSAGE_MINOR);
          }
          else if (valore_client > random && valore_client <= 100){ /** ramo in cui il numero e' maggiore di random */
            //printf("ramo maggiore\n");//debug
            if(tentativo==TENTATIVI_MAX){
              end = 1;
              //printf("tentativi massimi raggiunti\n");//debug
              strcpy(output, MESSAGE_ERROR_TENTATIVI);
            }
            else strcpy(output, MESSAGE_MAJOR);

          }
          else { /** ramo in cui il numero non e' accettabile*/
            strcpy(output, MESSAGE_ERROR);
            //printf("ramo valore non accettabile\n");//debug
            end=1;
          }
          write(simpleChildSocket, output, strlen(output));
        }
        else{
          end=1;
          //printf("connessione interrota con il client o problemi con la read\n");//debug
        }
        tentativo++;
      }
      tentativo=0;
      end=0;
      //printf("numero massimo di tentativi o errore o indovinato\n");//debug
      close(simpleChildSocket);
    }

    close(simpleSocket);
    return 0;
}
