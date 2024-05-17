#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <sys/wait.h>
#include <unistd.h>
#include <string.h>

#define MAX_PLAYERS 10
#define BOARD_SIZE 10
#define NUM_SHIPS 5
#define RESET   "\x1B[0m"
#define RED     "\x1B[31m"
#define GREEN   "\x1B[32m"
#define YELLOW  "\x1B[33m"
#define BLUE    "\x1B[34m"
#define MAGENTA "\x1B[35m"
#define CYAN    "\x1B[36m"
#define WHITE   "\x1B[37m"

typedef struct {
    int row;
    int col;
} Coord;

typedef struct {
    int len;
    Coord coords[4]; // Tamaño máximo de un barco es 4
} Ship;

typedef struct {
    pid_t idplayer;
    Ship ships[NUM_SHIPS];
    int board[BOARD_SIZE][BOARD_SIZE];
} Player;

typedef struct {
    Player *attacker;
    int x;
    int y;
} Rocket;

Player *players[MAX_PLAYERS];

pid_t readyPlayers[MAX_PLAYERS] = {0}; // Initialize to 0

int NUM_PLAYERS; // Número de jugadores

// Función para inicializar el tablero de un jugador con los barcos
Player* initializeBoard(Player *player) {
    // Inicializa el tablero con ceros
    for (int i = 0; i < BOARD_SIZE; i++) {
        for (int j = 0; j < BOARD_SIZE; j++) {
            player->board[i][j] = 0;
        }
    }

    // Semilla para la generación de números aleatorios
    srand(time(NULL));

    // Coloca los barcos en posiciones aleatorias
    int ship_sizes[NUM_SHIPS] = {2, 2, 3, 3, 4}; // Tamaños de los barcos
    for (int ship_index = 0; ship_index < NUM_SHIPS; ship_index++) {
        int ship_size = ship_sizes[ship_index];
        Ship new_ship;
        new_ship.len = ship_size;

        // Determina si el barco estará en posición horizontal o vertical
        int is_horizontal = rand() % 2; // 0 para vertical, 1 para horizontal

        // Variables para almacenar la fila y columna inicial del barco
        int start_row, start_col;

        // Coloca el barco en una posición válida
        do {
            // Posición aleatoria dentro del tablero
            start_row = rand() % BOARD_SIZE;
            start_col = rand() % BOARD_SIZE;

            // Verifica si hay suficiente espacio para colocar el barco
            if (is_horizontal) {
                if (start_col + ship_size <= BOARD_SIZE) {
                    // Verifica si la posición está ocupada
                    int overlap = 0;
                    for (int i = 0; i < ship_size; i++) {
                        if (player->board[start_row][start_col + i] != 0) {
                            overlap = 1;
                            break;
                        }
                    }
                    if (!overlap) break;
                }
            } else {
                if (start_row + ship_size <= BOARD_SIZE) {
                    // Verifica si la posición está ocupada
                    int overlap = 0;
                    for (int i = 0; i < ship_size; i++) {
                        if (player->board[start_row + i][start_col] != 0) {
                            overlap = 1;
                            break;
                        }
                    }
                    if (!overlap) break;
                }
            }
        } while (1);

        // Coloca el barco en el tablero
        for (int i = 0; i < ship_size; i++) {
            if (is_horizontal) {
                player->board[start_row][start_col + i] = ship_size; // Se asigna el tamaño del barco para representar su presencia
                new_ship.coords[i].row = start_row;
                new_ship.coords[i].col = start_col + i;
            } else {
                player->board[start_row + i][start_col] = ship_size; // Se asigna el tamaño del barco para representar su presencia
                new_ship.coords[i].row = start_row + i;
                new_ship.coords[i].col = start_col;
            }
        }

        // Guarda el barco en la estructura del jugador
        player->ships[ship_index] = new_ship;
    }

    return player;
}

// Función para imprimir el tablero de un jugador
void printBoard(Player *player) {
    if (player->idplayer != 0) { // Verifica si el ID del jugador es distinto de 0
        printf("Tablero del jugador %d:\n", player->idplayer);
        for (int i = 0; i < BOARD_SIZE; ++i) {
            for (int j = 0; j < BOARD_SIZE; ++j) {
                printf("%d ", player->board[i][j]);
            }
            printf("\n");
        }
    }
}

int allReady() {
    for (int i = 0; i < NUM_PLAYERS; i++) {
        if (readyPlayers[i] == 0) {
            return 0; // Si hay algún jugador que no está listo, retorna 0
        }
    }
    return 1; // Todos los jugadores están listos
}

int isHit(Player *opponent, int row, int col) {
    // Verifica si hay un barco en la posición de ataque
    if (opponent->board[row][col] > 0) {
        return 1; // Ataque exitoso
    } else {
        return 0; // Ataque fallido
    }
}

Player* playerAttack(Rocket *rocket, Player *opponent) {
    // Genera coordenadas aleatorias para el ataque
    int attack_row = rand() % BOARD_SIZE;
    int attack_col = rand() % BOARD_SIZE;

    if(rocket->attacker->idplayer == opponent->idplayer){
        return opponent;
    }
    printf("Jugador %d ataca a la posición (%d, %d) del Jugador %d...\n", rocket->attacker->idplayer, attack_row, attack_col, opponent->idplayer);
    
    // Verifica si el ataque fue exitoso
    if (isHit(opponent, attack_row, attack_col)) {
        printf(GREEN "¡Ataque exitoso al Jugador %d!\n" RESET, opponent->idplayer);
        opponent->board[attack_row][attack_col] = -1; // Marca el ataque exitoso en el tablero del oponente
        printBoard(opponent);
        return opponent;
        
    } else {
        printf(RED "Ataque fallido al Jugador %d :(\n" RESET, opponent->idplayer);
        return opponent;
    }
}

int hasRemainingShips(Player *player) {
    for (int i = 0; i < NUM_SHIPS; i++) {
        for (int j = 0; j < player->ships[i].len; j++) {
            if (player->board[player->ships[i].coords[j].row][player->ships[i].coords[j].col] > 0) {
                return 1; // Todavía quedan barcos
            }
        }
    }
    return 0; // No quedan barcos
}

int main() {
    // Solicitar al usuario el número de jugadores
    printf("Ingrese el número de jugadores (entre 2 y %d): ", MAX_PLAYERS);
    scanf("%d", &NUM_PLAYERS);

    if (NUM_PLAYERS < 2 || NUM_PLAYERS > MAX_PLAYERS) {
        printf("Número de jugadores inválido. Debe estar entre 2 y %d.\n", MAX_PLAYERS);
        return 1; // Salir del programa con un código de error
    }

    // Crear pipes para la comunicación entre el padre y los hijos
    int pipes[NUM_PLAYERS][2];
    for (int i = 0; i < NUM_PLAYERS; i++) {
        if (pipe(pipes[i]) == -1) {
            perror("Error al crear el pipe");
            exit(EXIT_FAILURE);
        }
    }

    // Crear procesos hijos (jugadores)
    for (int i = 0; i < NUM_PLAYERS; i++) {
        pid_t pid = fork();
        if (pid < 0) {
            perror("Error al crear el proceso hijo");
            exit(EXIT_FAILURE);
        } 
        else if (pid == 0) { // Proceso hijo (jugador)
            Player actualPlayer; // Declara una variable local para el jugador
            close(pipes[i][0]); // Cerrar el extremo de lectura del pipe en el proceso hijo
            actualPlayer.idplayer = getpid(); // Asigna el ID del jugador
            initializeBoard(&actualPlayer); // Inicializar el jugador
            sleep(1); //IMPORTANTE, NO QUITAR ESTE SLEEP (PORFAVOR)
            ssize_t bytes_written = write(pipes[i][1], &actualPlayer, sizeof(Player)); // Escribe los datos del jugador en el pipe
            if (bytes_written != sizeof(Player)) {
                perror("Error al escribir los datos del jugador en el pipe");
                exit(EXIT_FAILURE);
            }
            close(pipes[i][1]); // Cerrar el extremo de escritura del pipe en el proceso hijo después de escribir los datos
            exit(EXIT_SUCCESS);
        } 
        else if(pid > 0){ // Proceso padre
            wait(NULL);
            close(pipes[i][1]); // Cerrar el extremo de escritura del pipe en el proceso padre
            Player actPlayerPipe; // Declara una variable local para almacenar los datos del jugador recibidos desde el pipe
            ssize_t bytes_read = read(pipes[i][0], &actPlayerPipe, sizeof(Player)); // Lee los datos desde el pipe
            if (bytes_read != sizeof(Player)) {
                perror("Error al leer los datos del jugador desde el pipe");
                exit(EXIT_FAILURE);
            }
            players[i] = malloc(sizeof(Player)); // Asigna memoria dinámica para el jugador en el array de jugadores
            if (players[i] == NULL) {
                perror("Error al asignar memoria para el jugador en el array");
                exit(EXIT_FAILURE);
            }
            memcpy(players[i], &actPlayerPipe, sizeof(Player)); // Copia los datos del jugador al array de jugadores
            readyPlayers[i] = players[i]->idplayer; // Guarda el ID del jugador en readyPlayers
            printBoard(players[i]); // Imprime el tablero del jugador recibido
            close(pipes[i][0]);
        }
    }
    sleep(2); // ESTE SLEEP SE PUEDE QUITAR :)
    // cuando todos los jugadores están listos, comienza el juego
    if (allReady()) {
        printf(CYAN"¡Todos los jugadores están listos! Comenzando el juego...\n" RESET);
        pid_t gamepid;
        // AQUI EMPIEZA LO BUENO JÓVENES
        while(1){ // LA GUERRA DEL PACÍFICO 
            int remaining_players = NUM_PLAYERS;
            for (int i = 0; i < NUM_PLAYERS; i++) {
                if (!hasRemainingShips(players[i])) {
                    remaining_players--;
                }
            }
            // stop el juego si queda un solo jugador con barcos
            if (remaining_players <= 1) {
                // WINNER
                int winner_id = 0;
                for (int i = 0; i < NUM_PLAYERS; i++) {
                    if (hasRemainingShips(players[i])) {
                        winner_id = players[i]->idplayer;
                        break;
                    }
                }
                printf(YELLOW "Tablero del jugador ganador (ID: %d):\n" YELLOW, winner_id);
                for (int i = 0; i < NUM_PLAYERS; i++) {
                    if (players[i]->idplayer == winner_id) {
                        printBoard(players[i]);
                        break;
                    }
                }
                printf(YELLOW "¡El jugador %d ha ganado!\n" RESET, winner_id);
                break;
            }

            for (int i = 0; i < NUM_PLAYERS; i++) {//abriendo los pipes denuevo
                if (pipe(pipes[i]) == -1) {
                    perror("Error al crear el pipe");
                    exit(EXIT_FAILURE);
                }
            }
            for(int i = 0; i < NUM_PLAYERS; i++){
                gamepid = fork();
                if(gamepid == 0){// Proceso hijo (jugador)
                    wait(NULL);
                    srand(time(NULL));
                    close(pipes[i][0]); // Cerrar el extremo de lectura del pipe
                    Rocket *rocket = malloc(sizeof(Rocket));
                    if (rocket == NULL) {
                        perror("Error al asignar memoria para el cohete");
                        exit(EXIT_FAILURE);
                    }
                    rocket->attacker = players[i];
                    // Enviar resultados del ataque al proceso padre
                    ssize_t bytes_written = write(pipes[i][1], rocket, sizeof(Rocket)); // Escribe los datos del jugador en el pipe
                    if (bytes_written != sizeof(Rocket)) {
                        perror("Error al escribir los datos del cohete en el pipe");
                        exit(EXIT_FAILURE);
                    }
                    free(rocket); // Liberar memoria después de usarla
                    close(pipes[i][1]); // Cerrar el extremo de escritura del pipe en el proceso hijo
                    exit(EXIT_SUCCESS);
                }
                else if(gamepid > 0){
                    wait(NULL);
                    close(pipes[i][1]); // Cerrar el extremo de escritura del pipe en el proceso padre
                    Rocket *rocket = malloc(sizeof(Rocket));
                    if (rocket == NULL) {
                        perror("Error al asignar memoria para el cohete");
                        exit(EXIT_FAILURE);
                    }
                    ssize_t bytes_read = read(pipes[i][0], rocket, sizeof(Rocket)); // Lee los datos desde el pipe
                    if (bytes_read != sizeof(Rocket)) {
                        perror("Error al leer los datos del Cohete desde el pipe");
                        exit(EXIT_FAILURE);
                    }
                    for (int j  = 0; j < NUM_PLAYERS; j++){
                        playerAttack(rocket, players[j]);
                    }
                    free(rocket); // Liberar memoria después de usarla
                    close(pipes[i][0]);
                }
                wait(NULL);
            }
        }
    }
    else {
        for (int i = 0; i < NUM_PLAYERS; i++) {
            printf("xd: %d\n",readyPlayers[i]);
        }
        printf("¡Algunos jugadores no están listos! No se puede comenzar el juego.\n");
    }

    return 0;
}
