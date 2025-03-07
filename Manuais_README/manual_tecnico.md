# Manual Técnico

## Introdução
Este documento descreve a implementação do jogo **4 em Linha** em linguagem C, abordando as decisões técnicas, estrutura do código, algoritmos utilizados e justificativas para cada escolha.

## Estrutura do Código
O código está organizado da seguinte forma:

- Definição de constantes e estrutura de dados
- Funções para manipulação da leaderboard
- Funções para manipulação do tabuleiro de jogo
- Algoritmos de jogabilidade e verificação de vitória
- Histórico de jogos e exibição de estatísticas
- Interface do usuário e menu principal

## Definições e Estruturas
```c
#define ROWS 6
#define COLS 7
#define MAX_LEADERBOARD 10
#define FILENAME "leaderboard.txt"
```
As constantes **ROWS** e **COLS** definem o tamanho do tabuleiro. A leaderboard armazena os **10 melhores jogadores** e seus respectivos números de vitórias.

A estrutura utilizada para armazenar os vencedores:
```c
typedef struct {
    char winner[50];
    int wins;
} Match;
```
A estrutura `Match` armazena o nome do jogador vencedor e o número de vitórias.

## Algoritmos e Justificativas
### 1. Manipulação da Leaderboard
As funções para manipulação da leaderboard são:

- `saveLeaderboard()`: Salva a leaderboard em um arquivo.
- `loadLeaderboard()`: Carrega a leaderboard de um arquivo.
- `sortLeaderboard()`: Ordena a leaderboard em ordem decrescente de vitórias (Bubble Sort).
- `addLeaderboard()`: Adiciona um novo vencedor à leaderboard.

O **Bubble Sort** foi escolhido por ser fácil de implementar, já que a lista da leaderboard é pequena (máximo de 10 entradas).

### 2. Inicialização e Impressão do Tabuleiro
```c
void initBoard(char board[ROWS][COLS]) {
    for (int i = 0; i < ROWS; i++)
        for (int j = 0; j < COLS; j++)
            board[i][j] = ' ';
}
```
A função `initBoard()` inicializa o tabuleiro com espaços vazios, e `printBoard()` imprime o estado atual do tabuleiro de forma visualmente organizada.

### 3. Jogadas e Verificação de Vitória
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
O jogador escolhe uma coluna e a peça é posicionada na linha mais baixa disponível. 

#### Algoritmo de Verificação de Vitória
A função `checkWin()` verifica se um jogador conseguiu alinhar quatro peças consecutivas na **horizontal, vertical ou diagonal**. A escolha de varredura sequencial garante a verificação eficiente em O(n).

```c
int checkWin(char board[ROWS][COLS], char piece) {
    for (int i = 0; i < ROWS; i++) {
        for (int j = 0; j < COLS; j++) {
            if (j + 3 < COLS && board[i][j] == piece && board[i][j + 1] == piece && board[i][j + 2] == piece && board[i][j + 3] == piece)
                return 1;
            if (i + 3 < ROWS && board[i][j] == piece && board[i + 1][j] == piece && board[i + 2][j] == piece && board[i + 3][j] == piece)
                return 1;
            if (i + 3 < ROWS && j + 3 < COLS && board[i][j] == piece && board[i + 1][j + 1] == piece && board[i + 2][j + 2] == piece && board[i + 3][j + 3] == piece)
                return 1;
            if (i + 3 < ROWS && j - 3 >= 0 && board[i][j] == piece && board[i + 1][j - 1] == piece && board[i + 2][j - 2] == piece && board[i + 3][j - 3] == piece)
                return 1;
        }
    }
    return 0;
}
```

### 4. Histórico de Jogos
Cada partida é salva em um arquivo `.txt` com informações do jogo, incluindo os jogadores, data e tabuleiro final. Isso é feito por `saveGameHistory()`, que gera nomes de arquivos baseados na data/hora do jogo.

### 5. Interface e Menu
O menu interativo permite ao usuário escolher entre **Jogar**, **Exibir Leaderboard** ou **Ver Histórico de Jogos**. O uso de `scanf()` inclui validações para evitar entradas inválidas.

```c
while (scanf("%d", &choice) != 1 || choice < 1 || choice > 4) {
    printf("Opção inválida! Escolha 1, 2, 3 ou 4: ");
    while (getchar() != '\n'); // Limpar buffer
}
```
Isso evita que o programa falhe devido a entradas inesperadas.

## Conclusão
O código foi desenvolvido para ser **eficiente, modular e intuitivo**. As principais decisões técnicas incluem:
- **Uso de arquivos para persistência de dados** (leaderboard e histórico de jogos).
- **Algoritmo de ordenação simples** para manter a leaderboard atualizada.
- **Validações de entrada** para garantir jogabilidade estável.
- **Modo Player vs Bot** com jogadas aleatórias.
