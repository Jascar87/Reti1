#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>

int main(int argc, char *argv[]) {

    int simpleSocket = 0;
    int simplePort = 0;
    int returnStatus = 0;
    char buffer[256] = "";
    char keyword[4]= "";
    struct sockaddr_in simpleServer;
    const char MESSAGE_OK[] = "OK ";
    const char MESSAGE_NO[] = "NO ";
    const char MESSAGE_SI[] = "SI ";
    const char MESSAGE_ER[] = "ER ";

    int end=0; /** flag per terminare il ciclo while*/

    if (3 != argc) {
      fprintf(stderr, "Usage: %s <server> <port>\n", argv[0]);
      exit(1);
    }

    /* create a streaming socket      */
    simpleSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

    if (simpleSocket == -1) {
      fprintf(stderr, "Could not create a socket!\n");
      exit(1);
    }
    else {
	    fprintf(stderr, "Socket created!\n");
    }

    /* retrieve the port number for connecting */
    simplePort = atoi(argv[2]);

    /* setup the address structure */
    /* use the IP address sent as an argument for the server address  */
    //bzero(&simpleServer, sizeof(simpleServer));
    memset(&simpleServer, '\0', sizeof(simpleServer));
    simpleServer.sin_family = AF_INET;
    //inet_addr(argv[2], &simpleServer.sin_addr.s_addr);
    simpleServer.sin_addr.s_addr=inet_addr(argv[1]);
    simpleServer.sin_port = htons(simplePort);

    /*  connect to the address and port with our socket  */
    returnStatus = connect(simpleSocket, (struct sockaddr *)&simpleServer, sizeof(simpleServer));

    if (returnStatus == 0) {
	    fprintf(stderr, "Connect successful!\n");
    }
    else {
      fprintf(stderr, "Could not connect to address!\n");
      close(simpleSocket);
      exit(1);
    }

    /* get the message from the server   */
    returnStatus = read(simpleSocket, buffer, sizeof(buffer));/** per leggere il messa di benvenuto*/
    if ( returnStatus > 0 ) {
      read(buffer, keyword, sizeof(keyword));
      if(strcmp(keyword, MESSAGE_OK)==0){/**ramo in cui il server ha risposto con il messaggio di benvenuto in modo corretto*/
        printf("%d: %s", returnStatus, buffer);
        while(end==0){
          printf("Inserire un numero da 1 a 100\n");
          printf("tutti gli altri valori termineranno il gioco\n", );
          fflush(stdin);
          fscanf(stdin, "%s", buffer);
          write(simplePort, buffer, strlen(buffer)); /**inoltro il messaggio conetenuto in buffer al servver*/

          do{ /**continuo il ciclo fino a quando riesco ad effettuare una read andata a buon fine*/
            returnStatus = read(simpleSocket, buffer, sizeof(buffer));
            if ( returnStatus < 0 ) fprintf(stderr, "Return Status = %d \n", returnStatus);
          }while(returnStatus>0);

            /**valutare il messaggio ricevuto dal server*/
          read(buffer, keyword, sizeof(keyword));
          if(strcmp(keyword, MESSAGE_SI)==0){/**ramo in cui il numero è stato indovinato*/
            end=1;
            printf("%s", buffer);
            printf("Sono il client migliore che ci sia, ti ho fatto indovinare\n");
          }

          if(strcmp(keyword, MESSAGE_ER)==0){/**ramo in cui ci sono stati errori o si sono superati i tentativi massimi*/
            end=1;
            printf("%s", buffer);
            printf("Se il valore inserito era errato, il server se ne e' accorto\n");
            printf("Se invece non hai indovinato, non e' colpa mia, io ho svolto il mio compito corretamente\n");
          }

          if(strcmp(keyword, MESSAGE_NO)==0){/**ramo in cui il numero non e' stato indovinato*/
            read(buffer, keyword, 1);
            if(strcmp(keyword, "-")==0) printf("il numero inserito e' MAGGIORE del numero da indovinare \n");/**il numero inserito e' maggiore del numero da indovinare*/
            if(strcmp(keyword, "+")==0) printf("il numero inserito e' MINORE del numero da indovinare \n");/**il numero inserito e' minore del numero da indovinare*/
            else {/**la parola chiave inviata non e' codificata correttamente*/
              fprintf(stderr, "Parola chiave trasmessa : %s \n", keyword);
              end=1;
            }
          }

          else{/**casi restanti*/
            else fprintf(stderr, "Parola chiave trasmessa : %s \n", keyword);
            end=1;
          }

        }

      }
      else fprintf(stderr, "Parola chiave trasmessa : %s \n", keyword); /**ramo in cui la parola chiave non risulta corretta, viene mandata in output*/
    }
    else {/**ramo in cui ci sono stati problemi con la read*/
        fprintf(stderr, "Return Status = %d \n", returnStatus);
    }

    close(simpleSocket);
    return 0;
}
