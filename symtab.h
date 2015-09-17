/* 
 * A simple symbol table plugin
 *
 * @file    symtab.h
 * @author  Anthony A. Aaby
 * @author  Simon Maurer
 * @see     http://foja.dcs.fmph.uniba.sk/kompilatory/docs/compiler.pdf
 *
 * */


#ifndef SYMTAB_H
#define SYMTAB_H

struct symrec_port
{
    int mode;
    int pclass;
    char *name;
    struct symrec_port *next;
};
typedef struct symrec_port symrec_port;
struct symrec
{
    char* name;
    int type;
    struct symrec_port *port_table;
    struct symrec *next; /* link field */
};
typedef struct symrec symrec;
extern symrec *sym_table;
extern symrec_port *sym_port_table;

symrec *putsym_net ( char*, int );
symrec *getsym_net ( char* );
symrec_port *putsym_port ( char*, int, int );
symrec_port *getsym_port ( char*, int, int );
symrec_port *getsym_port_all ( char* );

#endif /* SYMTAB_H */
