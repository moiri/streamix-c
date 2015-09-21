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

typedef struct symrec_port symrec_port;
typedef struct symrec symrec;

// linked list structure for symbol table port records
struct symrec_port
{
    int mode;           // VAL_IN, VAL_OUT
    int pclass;         // VAL_UP, VAL_DOWN, VAL_SIDE
    char* name;         // name of the port
    symrec_port* next;  // link field
};

// linked list structure for symbol table box records
struct symrec
{
    char* name;                 // name of the box
    int type;                   // VAL_WRAP, VAL_BOX
    symrec_port *port_table;    // pointer to the port linked list
    symrec *next;               // link field
};

/*
 * Get a net identifier from the symbol table.
 *
 * @param char*     name of the net
 * @return symrec*:
 *      a pointer to the location where the data was stored
 *      a null pointer if the element was not found
 * */
symrec* getsym_net ( char* );

/*
 * Add a net identifier to the symbol table.
 *
 * @param char*:    name of the net
 * @param int:      type of the net
 * @return symrec*:
 *      a pointer to the location where the data was stored
 * */
symrec* putsym_net ( char*, int );

/*
 * Get a port identifier from the symbol table of the net
 * that is currently declared.
 *
 * @param char*:    name of the port
 * @param int:      calss of the port (up, down, side)
 * @param int:      mode of the port (in, out)
 * @return symrec_port*:
 *      a pointer to the location where the data was stored
 *      a null pointer if the element was not found
 * */
symrec_port* getsym_port ( char*, int, int );

/*
 * Get a port identifier from the symbol table.
 *
 * @param char*:    name of the port
 * @return symrec_port*:
 *      a pointer to the location where the data was stored
 *      a null pointer if the element was not found
 * */
symrec_port* getsym_port_all ( char* );

/*
 * Add a port identifier to the symbol table.
 *
 * @param char*:    name of the port
 * @param int:      calss of the port (up, down, side)
 * @param int:      mode of the port (in, out)
 * @return symrec_port*:
 *      a pointer to the location where the data was stored
 * */
symrec_port* putsym_port ( char*, int, int );

#endif /* SYMTAB_H */
