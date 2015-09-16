#include <stdlib.h> /* For malloc in symbol table */
#include <string.h> /* For strcmp in symbol table */
#include <stdio.h>
#include "symtab.h"

symrec *sym_table = (symrec *)0;

symrec *putsym_net ( char *name, int scope ) {
    symrec *ptr;
    /* printf("put %s", name); */
    ptr = (symrec *) malloc (sizeof(symrec));
    ptr->rec.rec_net.name = (char *) malloc (strlen(name)+1);
    strcpy (ptr->rec.rec_net.name, name);
    /* printf(" -> %p\n", ptr); */
    ptr->rec.rec_net.scope = scope;
    ptr->next = (struct symrec *)sym_table;
    sym_table = ptr;
    /* printf("(%p)", ptr->next); */
    /* printf(" sym_table: %p\n", sym_table); */
    return ptr;
}

symrec *getsym_net ( char *name, int scope ) {
    symrec *ptr;
    /* printf("get %s <-? ", name); */
    for (ptr = sym_table; ptr != (symrec *)0; ptr = ptr->next) {
        /* printf("%p / ", ptr); */
        if ((strcmp (ptr->rec.rec_net.name, name) == 0)
            && (ptr->rec.rec_net.scope == scope)) {
            /* printf(" => hit: %p\n", ptr); */
            /* printf(" sym_table: %p\n", sym_table); */
            return ptr;
        }
    }
    /* printf(" => no hit: %p\n", ptr); */
    /* printf(" sym_table: %p\n", sym_table); */
    return 0;
}
