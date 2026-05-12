/*  
    Data_structure_midterm_finale.c
    Problem Description:
        Design a program to maintain and query relationships between elements that belong to exactly two disjoint
        equivalence classes. The program should support the following operations:
        1. Add Relationship(D element1 element2): Given two elements, and declare that they belong to different equivalence classes.
        2. Query Relationship(Q element1 element2): Given two elements, determine if they belong to the same equivalence class or
            different equivalence classes, or if the relationship is unknown.

    Approach: array-based dsu (disjoint set union)
*/

#include <stdio.h>
#define MAX_ELEMENTS 100

typedef struct node
{
    int parent; // -1: unsorted, 0: root
    int flag;   // 0/1: use to add value to path
}
Node;

void init(Node *nodes, int size)
{
    for (int i = 0; i < size; i++)
    {
        nodes[i].parent = -1;
        nodes[i].flag = 0;
    }
}

// find the root of the element, point it to root(path compression), and change flag value
// return root
int findroot(Node *nodes, int i)
{
    if (nodes[i].parent == -1)
    {
        return -1; // unsorted
    }
    if (nodes[i].parent == 0)
    {
        return i; // root
    }
    int j;
    int pathlen = nodes[i].flag; // use to update flag value
    for (j = nodes[i].parent; nodes[j].parent != 0; j = nodes[j].parent)
    {
        pathlen++;
        pathlen += nodes[j].flag; // update pathlen    
    }
    nodes[i].parent = j; // path compression
    nodes[i].flag = pathlen % 2; // update flag value
    return j;
}

// D element1 element2 case
void Dtype(Node *nodes, int i, int j)
{
    if (nodes[i].parent == -1 && nodes[j].parent == -1)
    {
        nodes[i].parent = 0;
        nodes[i].flag = 1; // set flag value to 1 for root
        nodes[j].parent = i;
        // no need to set flag, for it will contrary to path compression
    }
    else if (nodes[i].parent == -1)
    {
        nodes[i].parent = j;
    }
    else if (nodes[j].parent == -1)
    {
        nodes[j].parent = i;
    }
    else
    {
        int rooti = findroot(nodes, i);
        int rootj = findroot(nodes, j);
        if (rooti == rootj)
        {
            if (nodes[i].flag == nodes[j].flag)
            {
                printf("Inconsistent\n");
            }
        } 
        else
        {
            nodes[rooti].parent = rootj;
            nodes[rooti].flag = (nodes[i].flag + nodes[j].flag) % 2; // update flag value
        }
    }   
}

// Q element1 element2 case
void Qtype(Node *nodes, int i, int j)
{
    int rooti = findroot(nodes, i);
    int rootj = findroot(nodes, j);
    if (rooti == -1 || rootj == -1)
    {
        printf("Unknown\n");
    }
    else if (rooti == rootj)
    {
        if (nodes[i].flag == nodes[j].flag)
        {
            printf("Same\n");
        }
        else
        {
            printf("Different\n");
        }
    }
    else
    {        
        printf("Unknown\n");
    }
}

void print(Node *nodes, int left, int right)
{
    if (left < 0 || right > MAX_ELEMENTS + 1 || left >= right)
    {
        printf("Invalid input\n");
        return;
    }
    printf("index");
    for (int i = left; i < right; i++)
    {
        printf("\t%d", i);
    }
    printf("\nparent");
    for (int i = left; i < right; i++)
    {
        printf("\t%d", nodes[i].parent);
    }
    printf("\nflag");
    for (int i = left; i < right; i++)
    {
        printf("\t%d", nodes[i].flag);
    }
    printf("\n");
}

int main()
{
    Node nodes[MAX_ELEMENTS + 1];
    init(nodes, MAX_ELEMENTS + 1);
    char op;
    int element1, element2;
    while (scanf("%c %d %d", &op, &element1, &element2) != EOF)
    {        
        if (op == 'D')
        {            
            Dtype(nodes, element1, element2);
        }
        else if (op == 'Q')
        {            
            Qtype(nodes, element1, element2);
        }
        else if (op == 'P')
        {
            print(nodes, element1, element2);
        }
    }
    return 0;
}
