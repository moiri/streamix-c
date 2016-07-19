/**
 * A simple symbol table plugin
 *
 * @file    symtab.h
 * @author  Anthony A. Aaby
 * @author  Simon Maurer
 * @see     http://foja.dcs.fmph.uniba.sk/kompilatory/docs/compiler.pdf
 *
 */

#ifndef SYMTAB_H
#define SYMTAB_H

// TYPEDEFS -------------------------------------------------------------------
typedef struct symrec_s symrec_t;
typedef struct symrec_list_s symrec_list_t;
typedef struct attr_box_s attr_box_t;
typedef struct attr_net_s attr_net_t;
typedef struct attr_port_s attr_port_t;
typedef struct attr_prot_s attr_prot_t;
typedef struct attr_wrap_s attr_wrap_t;
typedef enum symrec_type_e symrec_type_t;

// INCLUDES -------------------------------------------------------------------
#include <stdbool.h>
#include "uthash.h"
#include "utarray.h"
#include "vnet.h"

// ENUMS ----------------------------------------------------------------------
/**
 * @brief   Type of a symbol table record
 */
enum symrec_type_e
{
    SYMREC_BOX,
    SYMREC_NET,
    SYMREC_NET_PROTO,
    SYMREC_PORT,
    SYMREC_WRAP
};

// STRUCTURES -----------------------------------------------------------------
/**
 * @brief   Definition of a record in a hashtable (uthash)
 */
struct symrec_s
{
    char*           key;
    char*           name;   /**< name of the symbol */
    int             scope;  /**< scope of the record */
    symrec_type_t   type;   /**< #symrec_type_e */
    int             line;   /**< line position in the source file */
    union {
        attr_box_t*     attr_box;   /**< SYMREC_BOX */
        attr_net_t*     attr_net;   /**< SYMREC_NET */
        attr_port_t*    attr_port;  /**< SYMREC_PORT */
        attr_prot_t*    attr_proto; /**< SYMREC_NET_PROTO */
        attr_wrap_t*    attr_wrap;  /**< SYMREC)WRAP */
    };
    symrec_t*       next;   /**< pointer to the next element (collisions) */
    UT_hash_handle  hh;     /**< makes this structure hashable */
};

/**
 * @brief   Linked list to associate ports to nets
 */
struct symrec_list_s
{
    symrec_t*       rec;    /**< pointer to port in symbol table */
    symrec_list_t*  next;   /**< next element in the list */
};

/**
 * @breif   Attributes of a box
 */
struct attr_box_s
{
    bool            attr_pure;  /**< a box can be pure (functional) */
    char*           impl_name;  /**< implementation name */
    symrec_list_t*  ports;      /**< pointer to the port list of the net */
};

/**
 * @brief   Attributes of a net
 */
struct attr_net_s
{
    virt_net_t* v_net;  /**< pointer to a virtual net */
};

/**
 * @brief   Attributes of ports (all kind of ports: box (sync) or net)
 */
struct attr_port_s
{
    char*   int_name;       /**< internal name or NULL */
    int     mode;           /**< input or output */
    int     collection;     /**< VAL_UP, VAL_DOWN, VAL_SIDE, VAL_NONE */
    bool    decoupled;      /**< port is non-triggering **/
    int     sync_id;        /**< number to group sync ports together */
};

/**
 * @brief   Attributes of a net prototype
 */
struct attr_prot_s
{
    symrec_list_t*  ports;  /**< pointer to a linked list of ports */
};

/**
 * @breif   Attributes of a wrapper
 */
struct attr_wrap_s
{
    bool            attr_static;/**< wrapper does no proliferation */
    symrec_list_t*  ports;      /**< pointer to the port list of the net */
};

// FUNCTIONS ------------------------------------------------------------------
/**
 * @brief   checks whether two symtab entries are identical
 *
 * @param rec1  pointer to a symbol table record
 * @param rec2  pointer to a symbol table record
 * @param type  type of the records to compare
 *
 * @return      true if records are identical, false if not
 */
bool is_symrec_identical( symrec_t*, symrec_t*, symrec_type_t );

/**
 * @brief   Create a box attribute structure
 *
 * @param attr_pure flag indicating whether a box is pure or not
 * @param impl_name name of the box implementation
 * @param ports     pointer to a port list
 * @return          pointer to the new structure
 */
attr_box_t* symrec_attr_create_box( bool, char*, symrec_list_t* );

/**
 * @brief   Create a net attribute structure
 *
 * @param v_net pointer to a virtual net
 * @return          pointer to the new structure
 */
attr_net_t* symrec_attr_create_net( virt_net_t* );

/**
 * @brief   Create a port attribute structure
 *
 * @param int_name      internal name of the port
 * @param mode          direction of the port
 * @param collection    class of the port
 * @param decoupled     flag indicating whether a port is decoupled
 * @param sync_id       number to group synchronized ports together
 * @return              pointer to the new structure
 */
attr_port_t* symrec_attr_create_port( char*, int, int, bool, int );

/**
 * @brief   Create a net prototype attribute structure
 *
 * @param ports     pointer to a port list
 * @return          pointer to the new structure
 */
attr_prot_t* symrec_attr_create_proto( symrec_list_t* );

/**
 * @brief   Create a wrap attribute structure
 *
 * @param attr_static   flag incdicating whether a wrapper is static
 * @param ports         pointer to a port list
 * @return              pointer to the new structure
 */
attr_wrap_t* symrec_attr_create_wrap( bool, symrec_list_t* );

/**
 * @brief   Destroy attributes of a symbol table record
 *
 * @param rec   pointer to the symbol table record
 */
void symrec_attr_destroy_box( symrec_t* );
void symrec_attr_destroy_net( symrec_t* );
void symrec_attr_destroy_port( symrec_t* );
void symrec_attr_destroy_proto( attr_prot_t* );
void symrec_attr_destroy_wrap( symrec_t* );

/**
 * @brief   Create a symbol table record.
 *
 * Create a symbol table record where the attribute field is kept non
 * initialised.
 * @attention   the attribute field has to be initialised after the creation
 *
 * @param name      name of the record
 * @param scope     scope of the record
 * @param line      position (line number) of the identifier
 * @return          a pointer to the new record structure
 */
symrec_t* symrec_create( char*, int, symrec_type_t, int );


/**
 * @brief   Create a specific symbol table record.
 *
 * @param name      name of the record
 * @param scope     scope of the record
 * @param line      position (line number) of the identifier
 * @param attr      pointer to the specific attribute structure
 * @return          a pointer to the new record structure
 */
symrec_t* symrec_create_box( char*, int, int, attr_box_t* );
symrec_t* symrec_create_net( char*, int, int, attr_net_t* );
symrec_t* symrec_create_port( char*, int, int, attr_port_t* );
symrec_t* symrec_create_proto( char*, int, int, attr_prot_t* );
symrec_t* symrec_create_wrap( char*, int, int, attr_wrap_t* );

/**
 * @brief    Remove a record from the symbol table and free the allocated space
 *
 * @param symtab pointer to the hashtable
 * @param rec    pointer to the record to be removed
 */
void symrec_del( symrec_t**, symrec_t* );

/**
 * @brief   Remove all records from the symbol table
 *
 * @param recs  pointer to the hashtable
 */
void symrec_del_all( symrec_t** );

/**
 * @brief   free the memory of a symrec, excluding attributes
 *
 * @param rec   pointer to the symbol table record
 */
void symrec_destroy( symrec_t* );

/**
 * @brief   Remove all elements of a linked list
 *
 * @param list  pointer to the first element of a linked list
 */
void symrec_list_del( symrec_list_t* );

/**
 * @brief   Get a symbol from the symbol table
 *
 * Get an identifier from the symbol table and produce an error if it was not
 * found
 *
 * @param symtab        pointer to the hashtable
 * @param scope_stack   pointer to the scope stack
 * @param name          name of the identifier
 * @param line          position (line number) of the identifier
 * @return              a pointer to the location where the data is stored
 *                      a null pointer if the element was not found
 */
symrec_t* symrec_get( symrec_t**, UT_array*, char*, int );

/**
 * @brief   Add a record to the symbol table
 *
 * Add are cord to the symbol table and check for collisions. If there is a
 * key-collision, the new element is appended to the existing element as a
 * linked list. If the new element is identical to the existing one an error
 * message is printed
 *
 * @param symtab    pointer to the hashtable
 * @param new_item  pointer to the new record to be added
 * @return          a pointer to the location where the data was stored
 *                  a null pointer if the element was not stored
 */
symrec_t* symrec_put( symrec_t**, symrec_t* );

/**
 * Search an identifier in the symbol table and rturn it if found
 *
 * @param symrec**:     pointer to the hashtable
 * @param UT_array**:   pointer to the scope stack
 * @param char*:        name of the identifier
 * @return symrec*:
 *      a pointer to the location where the data is stored
 *      a null pointer if the element was not found
 * */
symrec_t* symrec_search( symrec_t**, UT_array*, char* );

#endif /* SYMTAB_H */
