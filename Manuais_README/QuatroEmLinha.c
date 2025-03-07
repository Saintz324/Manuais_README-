#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include <dirent.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <io.h>
#include <windows.h>

#define ROWS 6
#define COLS 7
#define MAX_LEADERBOARD 10
#define FILENAME "leaderboard.txt"
#define HISTORY_DIR "game_history" // diretorio para salvar o hist do jogo

typedef struct
{
    char winner[50];
    int wins_vs_bot;
    int wins_vs_player;
} Match;

Match leaderboard[MAX_LEADERBOARD * 10]; // mais de 10 players
int leaderboard_count = 0;

void createHistoryDir()
{
    // tenta criar diretorio
    if (CreateDirectory(HISTORY_DIR, NULL) == 0)
    {
        if (GetLastError() != ERROR_ALREADY_EXISTS)
        {

            printf("Erro ao criar o historico do diretorio! codigo de erro: %lu\n", GetLastError());
        }
    }
    else
    {
        printf("Diretorio '%s' criado com sucesso.\n", HISTORY_DIR);
    }
}

void saveLeaderboard()
{
    FILE *file = fopen(FILENAME, "w");
    if (!file)
        return;
    for (int i = 0; i < leaderboard_count; i++)
    {
        fprintf(file, "%s %d %d\n", leaderboard[i].winner, leaderboard[i].wins_vs_bot, leaderboard[i].wins_vs_player);
    }
    fclose(file);
}

void loadLeaderboard()
{
    FILE *file = fopen(FILENAME, "r");
    if (!file)
        return;
    leaderboard_count = 0;
    while (fscanf(file, "%s %d %d", leaderboard[leaderboard_count].winner,
                  &leaderboard[leaderboard_count].wins_vs_bot,
                  &leaderboard[leaderboard_count].wins_vs_player) == 3)
    {
        leaderboard_count++;
    }
    fclose(file);
}

void sortLeaderboard()
{
    for (int i = 0; i < leaderboard_count - 1; i++)
    {
        for (int j = 0; j < leaderboard_count - i - 1; j++)
        {
            int totalWinsJ = leaderboard[j].wins_vs_bot + leaderboard[j].wins_vs_player;
            int totalWinsJ1 = leaderboard[j + 1].wins_vs_bot + leaderboard[j + 1].wins_vs_player;

            if (totalWinsJ < totalWinsJ1)
            {
                // troca a partida
                Match temp = leaderboard[j];
                leaderboard[j] = leaderboard[j + 1];
                leaderboard[j + 1] = temp;
            }
        }
    }
}

void addLeaderboard(const char *winner, const char *loser)
{
    int is_vs_bot = (strcmp(loser, "Bot") == 0);

    // verifica se o player ja existe
    for (int i = 0; i < leaderboard_count; i++)
    {
        if (strcmp(leaderboard[i].winner, winner) == 0)
        {
            if (is_vs_bot)
                leaderboard[i].wins_vs_bot++;
            else
                leaderboard[i].wins_vs_player++;

            sortLeaderboard();
            saveLeaderboard();
            return;
        }
    }

    // Adiciona o player se nao existir ainda
    strcpy(leaderboard[leaderboard_count].winner, winner);
    leaderboard[leaderboard_count].wins_vs_bot = is_vs_bot ? 1 : 0;
    leaderboard[leaderboard_count].wins_vs_player = is_vs_bot ? 0 : 1;
    leaderboard_count++;

    sortLeaderboard();
    saveLeaderboard();
}

void showLeaderboard()
{
    sortLeaderboard(); // ordena os players com base nas vitorias

    printf("Leaderboard (Top 10 Jogadores):\n");
    for (int i = 0; i < (leaderboard_count < MAX_LEADERBOARD ? leaderboard_count : MAX_LEADERBOARD); i++)
    {
        printf("%d. %s - Vs Bot: %d wins | Vs Player: %d wins\n",
               i + 1, leaderboard[i].winner,
               leaderboard[i].wins_vs_bot,
               leaderboard[i].wins_vs_player);
    }
}

void initBoard(char board[ROWS][COLS])
{
    for (int i = 0; i < ROWS; i++)
        for (int j = 0; j < COLS; j++)
            board[i][j] = ' ';
}

void printBoard(char board[ROWS][COLS])
{
    printf(" 1 2 3 4 5 6 7\n");
    for (int i = 0; i < ROWS; i++)
    {
        printf("|");
        for (int j = 0; j < COLS; j++)
            printf("%c|", board[i][j]);
        printf("\n");
    }
    printf("---------------\n");
}

int dropPiece(char board[ROWS][COLS], int col, char piece)
{
    if (col < 1 || col > COLS || board[0][col - 1] != ' ')
        return 0;
    for (int i = ROWS - 1; i >= 0; i--)
    {
        if (board[i][col - 1] == ' ')
        {
            board[i][col - 1] = piece;
            return 1;
        }
    }
    return 0;
}

int checkWin(char board[ROWS][COLS], char piece)
{
    for (int r = 0; r < ROWS; r++)
        for (int c = 0; c < COLS - 3; c++)
            if (board[r][c] == piece && board[r][c + 1] == piece && board[r][c + 2] == piece && board[r][c + 3] == piece)
                return 1;

    for (int r = 0; r < ROWS - 3; r++)
        for (int c = 0; c < COLS; c++)
            if (board[r][c] == piece && board[r + 1][c] == piece && board[r + 2][c] == piece && board[r + 3][c] == piece)
                return 1;

    for (int r = 0; r < ROWS - 3; r++)
        for (int c = 0; c < COLS - 3; c++)
            if (board[r][c] == piece && board[r + 1][c + 1] == piece && board[r + 2][c + 2] == piece && board[r + 3][c + 3] == piece)
                return 1;

    for (int r = 3; r < ROWS; r++)
        for (int c = 0; c < COLS - 3; c++)
            if (board[r][c] == piece && board[r - 1][c + 1] == piece && board[r - 2][c + 2] == piece && board[r - 3][c + 3] == piece)
                return 1;

    return 0;
}

void saveGameHistory(const char *player1, const char *player2, char board[ROWS][COLS], const char *winner)
{
    createHistoryDir();  // verifica se o diretorio do historico do jogo existe

    //Lista todos os .txt no diretorio game_history
    DIR *d;
    struct dirent *dir;
    d = opendir(HISTORY_DIR);

    if (!d)
    {
        printf("Erro ao abrir diretorio!\n");
        return;
    }

    // armazena todos os nomes dos ficheiros num array para organizacao
    char filenames[100][256];
    int fileCount = 0;
    while ((dir = readdir(d)) != NULL)
    {
        if (strstr(dir->d_name, ".txt"))
        { // Lista apenas ficheiros txt
            strcpy(filenames[fileCount], dir->d_name);
            fileCount++;
        }
    }
    closedir(d);

    // se tem mais de 10 ficheiros, remove o ultimo
    if (fileCount >= 10)
    {
        // organiza os nomes dos ficheiros por data de criacao (antigos primeiro)
        struct _stat stat1, stat2;
        for (int i = 0; i < fileCount - 1; i++)
        {
            for (int j = i + 1; j < fileCount; j++)
            {
                char path1[300], path2[300];
                snprintf(path1, sizeof(path1), "%s/%s", HISTORY_DIR, filenames[i]);
                snprintf(path2, sizeof(path2), "%s/%s", HISTORY_DIR, filenames[j]);

                _stat(path1, &stat1);
                _stat(path2, &stat2);

                if (stat1.st_mtime > stat2.st_mtime)  //compara data de modificacao
                {
                    // troca os ficheiros se o primeiro e o mais novo
                    char temp[256];
                    strcpy(temp, filenames[i]);
                    strcpy(filenames[i], filenames[j]);
                    strcpy(filenames[j], temp);
                }
            }
        }

        // Remove o ficheiro mais antigo (the first one in the sorted list)
        char oldestFilePath[300];
        snprintf(oldestFilePath, sizeof(oldestFilePath), "%s/%s", HISTORY_DIR, filenames[0]);
        if (remove(oldestFilePath) == 0)
        {
            printf("Arquivo mais antigo removido: %s\n", filenames[0]);
        }
        else
        {
            printf("Erro ao remover o arquivo mais antigo: %s\n", filenames[0]);
        }
    }

    // gera um nome pro ficheiro unico para o novo jogo
    char filename[100];
    time_t t = time(NULL);
    struct tm tm = *localtime(&t);

    // Cria um nome pro ficheiro: Player1_vs_Player2_YYYY-MM-DD_HH-MM-SS.txt
    sprintf(filename, "%s/%s_vs_%s_%04d-%02d-%02d_%02d-%02d-%02d.txt",
            HISTORY_DIR, player1, player2,
            tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday,
            tm.tm_hour, tm.tm_min, tm.tm_sec);

    // Salva os detalhes do jogo em outro ficheiro
    FILE *file = fopen(filename, "w");
    if (!file)
    {
        printf("Erro ao salvar o historico do jogo.\n");
        return;
    }

    // escreve os detalhes do jogo no ficheiro
    fprintf(file, "Jogo entre: %s vs %s\n", player1, player2);
    fprintf(file, "Data: %04d-%02d-%02d %02d:%02d:%02d\n\n",
            tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday,
            tm.tm_hour, tm.tm_min, tm.tm_sec);

    fprintf(file, "Tabuleiro final:\n 1 2 3 4 5 6 7\n");
    for (int i = 0; i < ROWS; i++)
    {
        fprintf(file, "|");
        for (int j = 0; j < COLS; j++)
            fprintf(file, "%c|", board[i][j]);
        fprintf(file, "\n");
    }
    fprintf(file, "---------------\n\n");

    fprintf(file, "Vencedor: %s\n", winner);
    fclose(file);

    printf("Jogo salvo com sucesso!\n");
}

void playGame(int mode)
{
    char board[ROWS][COLS];
    char players[2] = {'X', 'O'};
    char player1[50], player2[50];
    int turn = 0, col;

    printf("Nome do Jogador 1: ");
    scanf("%s", player1);
    if (mode == 2)
    {
        strcpy(player2, "Bot");
    }
    else
    {
        printf("Nome do Jogador 2: ");
        scanf("%s", player2);
    }

    initBoard(board);
    while (1)
    {
        printBoard(board);
        printf("%s (%c), escolha uma coluna (1-7): ", turn == 0 ? player1 : player2, players[turn]);

        if (mode == 2 && turn == 1)
        {
            col = rand() % 7 + 1; // Bot faz uma jogada
            printf("%d\n", col);
        }
        else
        {
            // Input para validar loop
            while (1)
            {
                if (scanf("%d", &col) != 1 || col < 1 || col > 7)
                {
                    printf("Entrada invalida! Escolha um numero entre 1 e 7: ");
                    while (getchar() != '\n')
                        ; // limpa input invalido
                }
                else
                {
                    break;
                }
            }
        }

        if (dropPiece(board, col, players[turn]))
        {
            if (checkWin(board, players[turn]))
            {
                printBoard(board);
                printf("%s venceu!\n", turn == 0 ? player1 : player2);

                // Salvar jogo
                saveGameHistory(player1, player2, board, turn == 0 ? player1 : player2);
                // so adiciona ao leaderboard se ganhar contra o bot
                addLeaderboard(turn == 0 ? player1 : player2, turn == 1 ? player1 : player2);

                return;
            }
            turn = 1 - turn;
        }
        else
        {
            printf("Jogada invalida, tente novamente.\n");
        }
    }
}

void viewGameHistory()
{
    DIR *d;
    struct dirent *dir;
    d = opendir(HISTORY_DIR);

    if (!d)
    {
        printf("Erro ao abrir diretorio!\n");
        return;
    }

    printf("\nHistorico de jogos disponiveis:\n");
    int index = 1;
    char files[100][256]; // armazena os nomes dos ficheiros

    // lista todos os ficheiros e armazena
    while ((dir = readdir(d)) != NULL)
    {
        if (strstr(dir->d_name, ".txt"))
        { // lista apenas txt
            printf("%d. %s\n", index, dir->d_name);
            strcpy(files[index - 1], dir->d_name); // armazena os nomes dos ficheiros
            index++;
        }
    }
    closedir(d);

    if (index == 1) // sem ficheiros disp
    {
        printf("Nenhum historico de jogo disponivel.\n");
        return;
    }

    // pergunta ao user para selecionar
    int choice;
    printf("\nEscolha o numero do arquivo que deseja visualizar: ");
    while (scanf("%d", &choice) != 1 || choice < 1 || choice >= index)
    {
        printf("Opcao invalida! Escolha um numero valido entre 1 e %d: ", index - 1);
        while (getchar() != '\n')
            ; //limpa input invalido
    }

    // todo o caminho do ficheiro e abre
    char filepath[300];
    snprintf(filepath, sizeof(filepath), "%s/%s", HISTORY_DIR, files[choice - 1]);

    FILE *file = fopen(filepath, "r");
    if (!file)
    {
        printf("Erro ao abrir o arquivo: %s\n", filepath);
        return;
    }

    printf("\n--- Historico do Jogo: %s ---\n", files[choice - 1]);
    char line[256];
    while (fgets(line, sizeof(line), file))
    {
        printf("%s", line);
    }
    printf("\n--------------------------\n");

    fclose(file);
}

void listGameHistory()
{
    DIR *d;
    struct dirent *dir;
    d = opendir(HISTORY_DIR); // abre o HISTORY_DIR corretamente

    if (!d)
    {
        printf("Erro ao abrir diretorio!\n");
        return;
    }

    printf("\nHistorico de jogos disponiveis:\n");
    while ((dir = readdir(d)) != NULL)
    {
        if (strstr(dir->d_name, ".txt"))
        { // lista apenas txt
            printf("- %s\n", dir->d_name);
        }
    }
    closedir(d);
}

void menu()
{
    int choice;
    srand(time(NULL));
    loadLeaderboard();

    while (1)
    {
        printf("\n1. Jogar\n2. Leaderboard\n3. Ver Historico de Jogos\n4. Sair\nEscolha: ");

        // Loop ate uma opcao valida e escolhida
        while (scanf("%d", &choice) != 1 || choice < 1 || choice > 4)
        {
            printf("Opcao invalida! Escolha uma opcao valida (1, 2, 3 ou 4): ");
            while (getchar() != '\n')
                ; //limpa input invalido
        }

        switch (choice)
        {
        case 1:
            printf("1. Player vs Player\n2. Player vs Bot\nEscolha: ");

            // valida a escolha do modo de jogo (1 ou 2)
            while (scanf("%d", &choice) != 1 || (choice != 1 && choice != 2))
            {
                printf("Modo invalido! Escolha 1 ou 2: ");
                while (getchar() != '\n')
                    ; // limpa o imput invalido
            }

            playGame(choice);
            break;

        case 2:
            showLeaderboard();
            break;
        case 3:
            viewGameHistory(); // permite o user a escolher o ficheiro do jogo
            break;
        case 4:
            return; // sai do programa
        }
    }
}

int main()
{
    createHistoryDir();
    menu();
    return 0;
}
