#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

#define BoardType __uint64_t
#define ScaleType unsigned int
#define mask(x) (((BoardType)1) << (x))
#define i_j2pos_32(i, j) ((int)(((i) << 3) + (j)))
#define BoardSize 8
#define Display_Delay 100000

BoardType board;

// mask array for next step finder
BoardType next_step_array[64];

typedef struct
{
    ScaleType pos;      // i << 3 + j
    BoardType unvisited;  // initialized as 0
}
StackType;              // 128 bits

StackType Stack[64];    // 1 KB

int top;

#define initialStack() (top = 0)
#define isEmptyStack() (top == 0)
#define pushStack(x, y) (Stack[top++] = (StackType){(x), (y)})
// #define peekStack(x, y) ((x) = Stack[top - 1].pos, (y) = Stack[top - 1].unvisited)
#define popStack(x, y) (top--, (x) = Stack[top].pos, (y) = Stack[top].unvisited)
#define isFullStack() (top == 64)

int steps_taken;

// initialise mask array
void mask_generate()
{
    int direction_i[] = {-2, -1, 1, 2, 2, 1, -1, -2};
    int direction_j[] = {1, 2, 2, 1, -1, -2, -2, -1};
    for (int i = 0; i < BoardSize; i ++)
    {
        for (int j = 0; j < BoardSize; j ++)
        {
            int pos = i_j2pos_32(i, j);
            BoardType cur_mask = (BoardType)0;
            for (int k = 0; k < BoardSize; k ++)
            {
                int next_i = i + direction_i[k];
                int next_j = j + direction_j[k];
                if (next_i >= 0 && next_i < BoardSize && next_j >= 0 && next_j < BoardSize)
                {
                    cur_mask |= mask(i_j2pos_32(next_i, next_j));
                }
            }
            next_step_array[pos] = cur_mask;
        }
    }
}

void print_board()
{
    if (system("clear"))
    {
        printf ("clear screen failed!");
    }
    int path[8][8] = {0};
    int last_row = -1;
    int last_col = -1;

    if (top > 0)
    {
        int last_pos = Stack[top - 1].pos;
        last_row = last_pos >> 3;
        last_col = last_pos & 7;
    }

    for (int k = 0; k < top; k++)
    {
        int pos = Stack[k].pos;
        path[pos >> 3][pos & 7] = k + 1;
    }

    printf("     A    B    C    D    E    F    G    H\n");
    printf("  ┌────┬────┬────┬────┬────┬────┬────┬────┐\n");

    for (int i = 0; i < 8; i++)
    {
        printf("%d │", i + 1); 

        for (int j = 0; j < 8; j++)
        {
            if (path[i][j] == 0)
            {
                printf("    │"); 
            }
            else if (i == last_row && j == last_col)
            {
                printf("\033[1;32m[%2d]\033[0m│", path[i][j]);
            }
            else
            {
                printf(" %2d │", path[i][j]);
            }
        }
        printf("\n");

        if (i < 7)
        {
            printf("  ├────┼────┼────┼────┼────┼────┼────┼────┤\n");
        }
        else
        {
            printf("  └────┴────┴────┴────┴────┴────┴────┴────┘\n");
        }
    }
    printf("\ncurrent step: \033[1;32m%d\033[0m / 64\n\n", steps_taken);
    usleep(Display_Delay);
}

// traverse kernel (DFS)
void traverse_kernel(int i, int j, int print_board_open)
{
    mask_generate();
    ScaleType cur_pos = (ScaleType)i_j2pos_32(i, j);
    board = mask(cur_pos);                              // initial board
    BoardType cur_unvisited = next_step_array[cur_pos];   //~board & next_step_array[cur_pos];
    initialStack();
    pushStack(cur_pos, cur_unvisited);
    steps_taken = 1;
    
    if(print_board_open) print_board();
    
    while (!isEmptyStack() && !isFullStack())
    {
        steps_taken ++;
        popStack(cur_pos, cur_unvisited);
        if (cur_unvisited != (BoardType)0)
        {
            board |= mask(cur_pos);

            ScaleType next_pos;
            BoardType selecter = cur_unvisited;
            int min_choice = 9;             // find the pos where have least choices
            while (selecter != 0)           // Warnsdorff
            {
                ScaleType test_pos = (ScaleType)__builtin_ctzll(selecter);
                selecter &= selecter - 1;

                int test_choice = __builtin_popcountll(~board & next_step_array[test_pos]);
                if (test_choice < min_choice)
                {
                    min_choice = test_choice;
                    next_pos = test_pos;
                }
            }

            cur_unvisited &= ~mask(next_pos);
            pushStack(cur_pos, cur_unvisited);

            BoardType next_unvisited = ~board & next_step_array[next_pos];
            pushStack(next_pos, next_unvisited);
            
            if(print_board_open) print_board();
        }
        else
        {
            board &= ~mask(cur_pos);
            if(print_board_open) print_board();
        }
    }

    // simple display, need to expand later
    if (isFullStack())
    {
        board |= mask(Stack[top - 1].pos);
        if(print_board_open) print_board();

        printf ("\npass\n\n");
    }
}

int main()
{
    char charbuf[100];
    printf ("Please enter in a single line: [start_row(0-7)] [start_col(0-7)] [1: Render Board / 0: Time Count]\n");
    while(fgets(charbuf, 100, stdin) != NULL)
    {
        int i, j, print_open;
        if (sscanf(charbuf, "%d %d %d", &i, &j, &print_open) == 3)
        {
            if (i >= 0 && i < 8 && j >= 0 && j < 8)
            {
                if (print_open)
                {
                    traverse_kernel(i, j, 1);
                }
                else
                {
                    clock_t start = clock();
                    traverse_kernel(i, j, 0);
                    clock_t end = clock();
                    double time_taken = (double)(end - start) / CLOCKS_PER_SEC;
                    printf ("Time taken: %fs\n", time_taken);
                }
                break;
            }
        }
        printf ("Wrong format! Please try again.\n");
    }
    return 0;
}