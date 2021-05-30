typedef struct node node;
typedef struct list list;

list* create_list();

_Noreturn void delete_list(list* list);
void print_list(list* list);
void insert_value(list* list, int value);
void remove_value(list* list, int value);
void count_list(list* list, int (*predicate)(int));
