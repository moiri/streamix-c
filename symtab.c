#include <stdlib.h> /* For malloc in symbol table */
#include <string.h> /* For strcmp in symbol table */
#include <stdio.h>
#include "symtab.h"

symrec *sym_table = (symrec *)0;
symrec_port *sym_port_table = (symrec_port *)0;
symrec_port *sym_port_tmp = (symrec_port *)0;

/******************************************************************************/
symrec* getsym_net ( char *name ) {
    symrec *ptr;
    for (ptr = sym_table; ptr != (symrec *)0; ptr = ptr->next) {
        if (strcmp (ptr->name, name) == 0) {
            return ptr; // element found
        }
    }
    return 0;
}

/******************************************************************************/
symrec* putsym_net ( char *name, int type ) {
    symrec *ptr;
    // put name
    ptr = (symrec *) malloc(sizeof(symrec));
    ptr->name = (char *) malloc(strlen(name)+1);
    strcpy (ptr->name, name);
    // put type
    ptr->type = type;
    // update ptr
    ptr->next = sym_table;
    sym_port_table = sym_port_tmp;
    sym_port_tmp = (symrec_port *)0;
    ptr->port_table = sym_port_table;
    sym_table = ptr;
    return ptr;
}

/******************************************************************************/
symrec_port* getsym_port ( char *name, int pclass, int mode) {
    symrec_port *ptr;
    for (ptr = sym_port_tmp; ptr != (symrec_port *)0; ptr = ptr->next) {
        if ((strcmp (ptr->name, name) == 0)
            && ((mode == -1) || (ptr->mode == mode))
            && ((pclass == -1) || (ptr->pclass == pclass))) {
            return ptr; // element found
        }
    }
    return 0;
}

/******************************************************************************/
symrec_port* getsym_port_all ( char *name ) {
    symrec_port *ptr;
    symrec *ptr_net;
    for (ptr_net = sym_table; ptr_net != (symrec *)0; ptr_net = ptr_net->next) {
        /* printf("net name: %s\n", ptr_net->name); */
        for (ptr = ptr_net->port_table; ptr != (symrec_port *)0; ptr = ptr->next) {
            /* printf("port name: %s\n", ptr->name); */
            if (strcmp (ptr->name, name) == 0) {
                return ptr; // element found
            }
        }
    }
    return 0;
}

/******************************************************************************/
symrec_port* putsym_port ( char *name, int pclass, int mode ) {
    symrec_port *ptr;
    // put name
    ptr = (symrec_port *) malloc(sizeof(symrec_port));
    ptr->name = (char *) malloc(strlen(name)+1);
    strcpy (ptr->name, name);
    // put mode and class
    ptr->mode = mode;
    ptr->pclass = pclass;
    // update ptr
    ptr->next = sym_port_tmp;
    if (sym_port_tmp == 0) {
        // first port in this scope
        sym_port_tmp = sym_port_table;
    }
    sym_port_tmp = ptr;
    return ptr;
}
