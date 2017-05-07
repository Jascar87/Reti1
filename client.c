#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>

int split_message(char* source, char* dest_keyword, char* dest_message, size_t n, char split, char terminazione, int pointer_read){
  int split_counter=pointer_read;
  int pointer_keyword=0;
  int pointer_message=0;
  int splitted=0;
  if (source == NULL) return -1;
  if (dest_keyword==NULL) return -2;
  if (dest_message==NULL) return -3;
  if (n<=0) return -4;
  if (split<0 || split>127) return -5;
  if (pointer_read<0 ) return -6;
  n+=pointer_read;
  while(pointer_read < n){
    if(source[pointer_read]!= terminazione){
      if(splitted==0 && source[pointer_read]!=split){ /**ramo in cui non si ha ancora splittato o non ho travato il carratere di split*/
        dest_keyword[pointer_keyword++]=source[pointer_read];
        split_counter++;
      }
      else if (splitted == 0){/**ramo in cui ho trovato il carratteri di split per la prima volta*/
         splitted=1;
         split_counter++;
         dest_keyword[pointer_keyword]='\0';
       }
      else dest_message[pointer_message++]=source[pointer_read]; /**ramo in cui e' gia' stato splittato il messaggio*/
    }
    else dest_message[pointer_message]='\0'; /**ramo in cui ho incontrato il carattere di terminazione*/
    pointer_read++;
  }
  return split_counter;
}

int main(int argc, char *argv[]) {
    int pointer_read=0;
    int simpleSocket = 0;
    int simplePort = 0;
    int returnStatus = 0;
    char buffer[256] = "";
    char buffer_in[32] ="";
    char keyword[4]= "";
    char message[253] = "";
    struct sockaddr_in simpleServer;
    const char MESSAGE_OK[] = "OK";
    const char MESSAGE_NO[] = "NO";
    const char MESSAGE_SI[] = "SI";
    const char MESSAGE_ER[] = "ER";

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
    returnStatus = read(simpleSocket, buffer, sizeof(buffer));/** per leggere il messaggio di benvenuto*/
    if ( returnStatus > 0 ) {
      pointer_read = split_message(buffer, keyword, message, (strlen(buffer)-pointer_read), ' ', '\n', pointer_read);
      if(pointer_read<0) {
        close(simpleSocket);
        fprintf(stderr, "ERRORE FUNZION DI SPLIT = %d \n", pointer_read);
        return -1;
      }

      if(strcmp(keyword, MESSAGE_OK)==0){/**ramo in cui il server ha risposto con il messaggio di benvenuto in modo corretto*/
        printf("%d: %s\n", returnStatus, message);
        while(end==0){
          printf("Inserire un numero da 1 a 100\n");
          printf("tutti gli altri valori termineranno il gioco\n");
          fflush(stdin);
          fscanf(stdin, "%s", buffer_in);
          write(simpleSocket, buffer_in, strlen(buffer_in)); /**inoltro il messaggio conetenuto in buffer al servver*/
          memset(buffer, '\0', sizeof(buffer));
          returnStatus = read(simpleSocket, buffer, sizeof(buffer));
          if ( returnStatus < 0 ) fprintf(stderr, "Return Status = %d \n", returnStatus);
            /**valutare il messaggio ricevuto dal server*/
          memset(keyword, '\0', sizeof(keyword));
          memset(message, '\0', sizeof(message));
          pointer_read=0;
          pointer_read = split_message(buffer, keyword, message, (strlen(buffer)-pointer_read), ' ', '\n', pointer_read);
          if(pointer_read<0) {
            close(simpleSocket);
            fprintf(stderr, "ERRORE FUNZION DI SPLIT = %d \n", pointer_read);
            return -1;
          }
          if(strcmp(keyword, MESSAGE_SI)==0){/**ramo in cui il numero è stato indovinato*/
            end=1;
            printf("%s\n", message);
            printf("Sono il client migliore che ci sia, ti ho fatto indovinare\n");
          }

          if(strcmp(keyword, MESSAGE_ER)==0){/**ramo in cui ci sono stati errori o si sono superati i tentativi massimi*/
            end=1;
            printf("%s\n", message);
            printf("Se il valore inserito era errato, il server se ne e' accorto\n");
            printf("Se invece non hai indovinato, non e' colpa mia perche' ho svolto il mio compito corretamente\n");
          }

          if(strcmp(keyword, MESSAGE_NO)==0){/**ramo in cui il numero non e' stato indovinato*/
          memset(keyword, '\0', sizeof(keyword));
          memset(message, '\0', sizeof(message));
          pointer_read = split_message(buffer, keyword, message, (strlen(buffer)-pointer_read), ' ', '\n', pointer_read);
          if(pointer_read<0) {
            close(simpleSocket);
            fprintf(stderr, "ERRORE FUNZION DI SPLIT = %d \n", pointer_read);
            return -1;
          }
            if(strcmp(keyword, "-")==0) printf("Il numero inserito e' MAGGIORE del numero da indovinare \n");/**il numero inserito e' maggiore del numero da indovinare*/
            else if(strcmp(keyword, "+")==0) printf("Il numero inserito e' MINORE del numero da indovinare \n");/**il numero inserito e' minore del numero da indovinare*/
            else {/**la parola chiave inviata non e' codificata correttamente*/
              fprintf(stderr, "Parola chiave trasmessa : %s \n", keyword);
              end=1;
            }
          }

          else{/**casi restanti*/
            fprintf(stderr, "Parola chiave trasmessa : %s \n", keyword);
            end=1;
          }
          memset(buffer_in, '\0', sizeof(buffer_in));
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
