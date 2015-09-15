#ifndef SYMTAB_H
#define SYMTAB_H

struct symrec
{
    char *name; /* name of symbol */
    struct symrec *next; /* link field */
};
typedef struct symrec symrec;
extern symrec *sym_table;

symrec *putsym ( char* );
symrec *getsym ( char* );

#endif /* SYMTAB_H */
