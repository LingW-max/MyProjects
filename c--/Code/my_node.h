#ifndef MY_NODE_H
#define MY_NODE_H
#define CHILD_SIZE 10
typedef enum
{
        stnc = 1,
        wrds = 2
} node_type;

struct Node
{
        char *name;
        int line_num;
        node_type type; // words or sentence;
        struct Node *children[CHILD_SIZE];
        union
        {
                int node_int;
                float node_float;
                char *node_str;
        };
        int child_num;
};
struct Node *root;
struct Node *createNode(char *name, int lineno, node_type type);

#endif
