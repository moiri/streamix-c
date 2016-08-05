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

#include "defines.h"

// TYPEDEFS -------------------------------------------------------------------
typedef struct symrec_s symrec_t;
typedef struct symrec_list_s symrec_list_t;
typedef struct attr_box_s attr_box_t;
typedef struct attr_net_s attr_net_t;
typedef struct attr_port_s attr_port_t;
typedef struct attr_prot_s attr_prot_t;
typedef struct attr_wrap_s attr_wrap_t;
typedef enum symrec_type_e symrec_type_t;
typedef enum port_mode_e port_mode_t;
typedef enum port_class_e port_class_t;

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
    igraph_t    g;
};

/**
 * @brief   Attributes of ports (all kind of ports: box (sync) or net)
 */
struct attr_port_s
{
    symrec_list_t*  ports_int;      /**< internal name or NULL */
    port_mode_t     mode;           /**< input or output */
    port_class_t    collection;     /**< VAL_UP, VAL_DOWN, VAL_SIDE, VAL_NONE */
    bool            decoupled;      /**< port is non-triggering **/
    int             sync_id;        /**< number to group sync ports together */
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
    virt_net_t*     v_net;      /**< pointer to a virtual net */
    igraph_t        g;
};

// FUNCTIONS ------------------------------------------------------------------
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
 * @param g     pointer to a graph object
 * @return      pointer to the new structure
 */
attr_net_t* symrec_attr_create_net( virt_net_t*, igraph_t* );

/**
 * @brief   Create a port attribute structure
 *
 * @param port_int      pointer to record list of port names
 * @param mode          direction of the port
 * @param collection    class of the port
 * @param decoupled     flag indicating whether a port is decoupled
 * @param sync_id       number to group synchronized ports together
 * @return              pointer to the new structure
 */
attr_port_t* symrec_attr_create_port( symrec_list_t*, port_mode_t, port_class_t,
        bool, int );

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
 * @param v_net         pointer to a virtual net
 * @return              pointer to the new structure
 */
attr_wrap_t* symrec_attr_create_wrap( bool, virt_net_t*, igraph_t* );

/**
 * @brief   Destroy attributes of a symbol table record
 *
 * @param attr  pointer to the attribute
 */
void symrec_attr_destroy_box( attr_box_t* );
void symrec_attr_destroy_net( attr_net_t* );
void symrec_attr_destroy_port( attr_port_t* );
void symrec_attr_destroy_proto( attr_prot_t* );
void symrec_attr_destroy_wrap( attr_wrap_t* );

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
 * @param attr_key  a number derived from the attributes to create a unique key
 * @return          a pointer to the new record structure
 */
symrec_t* symrec_create( char*, int, symrec_type_t, int, int );


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
 * @param attr_key      a number derived from attributes to create a unique key
 *                          if a port: port_class + 1
 *                          if not a port: 0
 * @return              a pointer to the location where the data is stored
 *                      a null pointer if the element was not found
 */
symrec_t* symrec_get( symrec_t**, UT_array*, char*, int, int );

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
 * @param symtab        pointer to the hashtable
 * @param scope_stack   pointer to the scope stack
 * @param name          name of the identifier
 * @param attr_key      a number derived from attributes to create a unique key:
 *                          if a port: port_class + 1
 *                          if not a port: 0
 * @return              a pointer to the location where the data is stored
 *                      a null pointer if the element was not found
 * */
symrec_t* symrec_search( symrec_t**, UT_array*, char*, int );

#endif /* SYMTAB_H */
