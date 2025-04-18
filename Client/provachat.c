#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define MAX_MSG_LEN 256

// Funzione per pulire la linea di input
void clear_input_line() {
    printf("\33[2K\r"); // ANSI: cancella la linea corrente
    fflush(stdout);
}

// Funzione per cancellare la linea precedente
void clear_previous_line() {
    printf("\033[1A\33[2K\r"); // Sposta su di una riga e cancella
    fflush(stdout);
}

// Funzione per visualizzare un messaggio
void mostra_messaggio(const char* mittente, const char* messaggio, int a_destra) {
    // Aggiungi un separatore per ogni messaggio
    if (a_destra) {
        // Messaggio a destra (tu)
        printf("\033[1;32m"); // Colore verde per il tuo messaggio
        printf("[%s]      : %s\n", mittente, messaggio);
    } else {
        // Messaggio a sinistra (altro utente)
        printf("\033[1;34m"); // Colore blu per l'altro utente
        printf("[%s]: %s\n", mittente, messaggio);
    }
    printf("\033[0m");  // Reset dei colori
}

// Funzione per gestire l'input dell'utente
void chat_input() {
    char msg[MAX_MSG_LEN];
    
    while (1) {
        printf("(/exit)> ");  // Prompt per l'utente
        fflush(stdout);

        // Fai attenzione a pulire la riga dell'input prima di riceverlo
        if (fgets(msg, MAX_MSG_LEN, stdin) == NULL) {
            break;
        }

        // Rimuovi il carattere di fine linea
        msg[strcspn(msg, "\n")] = 0;

        // Se l'utente scrive "/esci", chiudiamo la chat
        if (strcmp(msg, "/exit") == 0) {
            break;
        }

        // Pulisci la linea di input
        clear_input_line();

        // Cancella la linea precedente se c'è
        clear_previous_line();

        // Mostra il messaggio
        mostra_messaggio("tu", msg, 1); // Mostra il messaggio che l'utente ha scritto
    }
}


int main() {
    // Intestazione della chat
    printf("==========================================\n");
    printf("            CHAT CONNESSA CON:           \n");
    printf("               alice_93                 \n");
    printf("==========================================\n");

    // Simulazione della ricezione di messaggi
    mostra_messaggio("alice_93", "Ehi, come va?", 0);
    sleep(1);
    mostra_messaggio("tu", "Tutto bene, tu?", 1);
    sleep(1);
    mostra_messaggio("alice_93", "Sto provando questo nuovo client ", 0);
    sleep(1);
    mostra_messaggio("tu", "Sembra figo!", 1);
    sleep(1);

    // Ora l'utente può scrivere un messaggio
    chat_input();

    printf("\nChat terminata.\n");

    return 0;
}
