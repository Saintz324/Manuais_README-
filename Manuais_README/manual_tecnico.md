# Manual Técnico - 4 em Linha

## Introdução
Este documento descreve a implementação do jogo **4 em Linha** em linguagem C, abordando as decisões técnicas, estrutura do código, algoritmos utilizados e justificativas para cada escolha.

## Estrutura do Código
O código está organizado da seguinte forma:

- Definição de constantes e estrutura de dados
- Funções para manipulação da leaderboard
- Funções para manipulação do tabuleiro de jogo
- Algoritmos de jogabilidade e verificação de vitória
- Histórico de jogos e exibição de estatísticas
- Interface do utilizador e menu principal

## Definições e Estruturas
```c
#define ROWS 6
#define COLS 7
#define MAX_LEADERBOARD 10
#define FILENAME "leaderboard.txt"
#define HISTORY_DIR "game_history"
```

As constantes definem o tamanho do tabuleiro e os limites do sistema. A leaderboard é projetada para armazenar informações de jogadores e suas vitórias.

A estrutura utilizada para armazenar os vencedores foi expandida para incluir diferentes tipos de vitórias:

```c
typedef struct {
    char winner[50];
    int wins_vs_bot;
    int wins_vs_player;
} Match;
```

A estrutura `Match` armazena o nome do jogador vencedor e o número de vitórias, diferenciando entre vitórias contra o bot e contra outros jogadores.

## Algoritmos e Justificativas

### 1. Gestão de Diretórios e Arquivos

#### Criação do Diretório de Histórico
```c
void createHistoryDir() {
    if (CreateDirectory(HISTORY_DIR, NULL) == 0) {
        if (GetLastError() != ERROR_ALREADY_EXISTS) {
            printf("Erro ao criar o historico do diretorio! codigo de erro: %lu\n", GetLastError());
        }
    }
}
```

Esta função utiliza a API do Windows para criar o diretório de histórico de jogos, com tratamento adequado de erros. A verificação de `ERROR_ALREADY_EXISTS` evita mensagens de erro desnecessárias quando o diretório já existe.

### 2. Manipulação da Leaderboard

As funções para manipulação da leaderboard incluem:

- `saveLeaderboard()`: Salva a leaderboard em um arquivo.
- `loadLeaderboard()`: Carrega a leaderboard de um arquivo.
- `sortLeaderboard()`: Ordena a leaderboard em ordem decrescente de vitórias totais (Bubble Sort).
- `addLeaderboard()`: Adiciona um novo vencedor à leaderboard ou atualiza um existente.

A implementação do Bubble Sort para ordenação:

```c
void sortLeaderboard() {
    for (int i = 0; i < leaderboard_count - 1; i++) {
        for (int j = 0; j < leaderboard_count - i - 1; j++) {
            int totalWinsJ = leaderboard[j].wins_vs_bot + leaderboard[j].wins_vs_player;
            int totalWinsJ1 = leaderboard[j + 1].wins_vs_bot + leaderboard[j + 1].wins_vs_player;

            if (totalWinsJ < totalWinsJ1) {
                // troca a partida
                Match temp = leaderboard[j];
                leaderboard[j] = leaderboard[j + 1];
                leaderboard[j + 1] = temp;
            }
        }
    }
}
```

O Bubble Sort foi escolhido por ser fácil de implementar e eficiente para listas pequenas como a leaderboard (que exibe apenas os 10 melhores jogadores).

### 3. Inicialização e Impressão do Tabuleiro

```c
void initBoard(char board[ROWS][COLS]) {
    for (int i = 0; i < ROWS; i++)
        for (int j = 0; j < COLS; j++)
            board[i][j] = ' ';
}

void printBoard(char board[ROWS][COLS]) {
    printf(" 1 2 3 4 5 6 7\n");
    for (int i = 0; i < ROWS; i++) {
        printf("|");
        for (int j = 0; j < COLS; j++)
            printf("%c|", board[i][j]);
        printf("\n");
    }
    printf("---------------\n");
}
```

A função `initBoard()` inicializa o tabuleiro com espaços vazios, e `printBoard()` imprime o estado atual do tabuleiro de forma visualmente organizada, incluindo números de coluna para facilitar a jogada do utilizador.

### 4. Jogadas e Verificação de Vitória

#### Inserção de Peça
```c
int dropPiece(char board[ROWS][COLS], int col, char piece) {
    if (col < 1 || col > COLS || board[0][col - 1] != ' ')
        return 0;
    for (int i = ROWS - 1; i >= 0; i--) {
        if (board[i][col - 1] == ' ') {
            board[i][col - 1] = piece;
            return 1;
        }
    }
    return 0;
}
```

O jogador escolhe uma coluna, e a peça é posicionada na linha mais baixa disponível. A função inclui validações para garantir que a coluna escolhida está dentro dos limites e não está cheia.

#### Algoritmo de Verificação de Vitória Otimizado

```c
int checkWin(char board[ROWS][COLS], char piece) {
    // Verifica horizontal
    for (int r = 0; r < ROWS; r++)
        for (int c = 0; c < COLS - 3; c++)
            if (board[r][c] == piece && board[r][c + 1] == piece && board[r][c + 2] == piece && board[r][c + 3] == piece)
                return 1;

    // Verifica vertical
    for (int r = 0; r < ROWS - 3; r++)
        for (int c = 0; c < COLS; c++)
            if (board[r][c] == piece && board[r + 1][c] == piece && board[r + 2][c] == piece && board[r + 3][c] == piece)
                return 1;

    // Verifica diagonal ↘
    for (int r = 0; r < ROWS - 3; r++)
        for (int c = 0; c < COLS - 3; c++)
            if (board[r][c] == piece && board[r + 1][c + 1] == piece && board[r + 2][c + 2] == piece && board[r + 3][c + 3] == piece)
                return 1;

    // Verifica diagonal ↗
    for (int r = 3; r < ROWS; r++)
        for (int c = 0; c < COLS - 3; c++)
            if (board[r][c] == piece && board[r - 1][c + 1] == piece && board[r - 2][c + 2] == piece && board[r - 3][c + 3] == piece)
                return 1;

    return 0;
}
```

A função verifica se um jogador conseguiu alinhar quatro peças consecutivas na horizontal, vertical ou diagonal. A implementação divide as verificações por direção para melhorar a legibilidade e eficiência.

### 5. Histórico de Jogos

O sistema implementa um gerir sofisticado do histórico de jogos:

```c
void saveGameHistory(const char *player1, const char *player2, char board[ROWS][COLS], const char *winner) {
    createHistoryDir();  // verifica se o diretorio do historico do jogo existe

    // Lista todos os .txt no diretorio game_history
    DIR *d;
    struct dirent *dir;
    d = opendir(HISTORY_DIR);

    // Código para listar e gerir arquivos

    // se tem mais de 10 ficheiros, remove o ultimo
    if (fileCount >= 10) {
        // organiza os nomes dos ficheiros por data de criacao (antigos primeiro)
        struct _stat stat1, stat2;
        // Código de ordenação e remoção
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
    
    // [Código para escrever detalhes do jogo]
}
```

Características principais:
1. **Nomenclatura Padronizada**: Os arquivos seguem o formato `Player1_vs_Player2_YYYY-MM-DD_HH-MM-SS.txt`
2. **Limitação de Arquivos**: Mantém um máximo de 10 arquivos, removendo o mais antigo quando necessário
3. **Ordenação por Data**: Usa `_stat()` para comparar datas de modificação

### 6. Visualização de Histórico de Jogos

```c
void viewGameHistory() {
    DIR *d;
    struct dirent *dir;
    d = opendir(HISTORY_DIR);

    // Código para listar arquivos

    // pergunta ao user para selecionar
    int choice;
    printf("\nEscolha o numero do arquivo que deseja visualizar: ");
    while (scanf("%d", &choice) != 1 || choice < 1 || choice >= index) {
    }

    // todo o caminho do ficheiro e abre
    char filepath[300];
    snprintf(filepath, sizeof(filepath), "%s/%s", HISTORY_DIR, files[choice - 1]);

    FILE *file = fopen(filepath, "r");
    
    // Código para exibir conteúdo do arquivo
}
```

Esta função permite ao utilizador selecionar e visualizar um jogo específico do histórico.

### 7. Implementação do Bot

```c
if (mode == 2 && turn == 1) {
    col = rand() % 7 + 1; // Bot faz uma jogada
    printf("%d\n", col);
}
```

O bot utiliza um algoritmo simples de geração de números aleatórios para escolher uma coluna. Esta abordagem foi escolhida por sua simplicidade de implementação, proporcionando uma experiência de jogo básica contra o computador.

### 8. Interface e Menu

```c
void menu() {
    int choice;
    srand(time(NULL));
    loadLeaderboard();

    while (1) {
        printf("\n1. Jogar\n2. Leaderboard\n3. Ver Historico de Jogos\n4. Sair\nEscolha: ");

        // Loop ate uma opcao valida e escolhida
        while (scanf("%d", &choice) != 1 || choice < 1 || choice > 4) {
            printf("Opcao invalida! Escolha uma opcao valida (1, 2, 3 ou 4): ");
            while (getchar() != '\n'); //limpa input invalido
        }

        // Switch case para opções do menu
    }
}
```

O menu interativo implementa validação robusta de entrada, evitando falhas causadas por entradas inesperadas. O uso de `getchar()` para limpar o buffer de entrada é uma técnica eficaz para evitar loops infinitos durante a validação.

## Gestão de Entrada e Tratamento de Erros

O código implementa uma técnica robusta de validação de entrada em vários pontos:

```c
while (scanf("%d", &choice) != 1 || choice < 1 || choice > 4) {
    printf("Opcao invalida! Escolha uma opcao valida (1, 2, 3 ou 4): ");
    while (getchar() != '\n'); // limpa input invalido
}
```

Essa abordagem verifica:
1. Se a leitura foi bem-sucedida (`scanf("%d", &choice) != 1`)
2. Se o valor está dentro do intervalo esperado (`choice < 1 || choice > 4`)
3. Limpa o buffer de entrada para evitar problemas em leituras futuras

## Conclusão

O código foi desenvolvido para ser **eficiente, modular e intuitivo**. As principais decisões técnicas incluem:

1. **Uso de arquivos para persistência de dados**:
   - Leaderboard armazenada em arquivo texto
   - Histórico de jogos em arquivos individuais com nomenclatura padronizada

2. **Gerenciamento de recursos**:
   - Limitação a 10 arquivos de histórico
   - Remoção automática dos arquivos mais antigos

3. **Validação robusta de entradas**:
   - Todas as entradas do utilizador são validadas
   - Buffer de entrada é limpo após cada leitura inválida

4. **Estruturação modular**:
   - Funções com responsabilidades bem definidas
   - Separação entre lógica de jogo e interface do utilizador

5. **Múltiplos modos de jogo**:
   - Player vs Player
   - Player vs Bot (algoritmo de jogada aleatória)

