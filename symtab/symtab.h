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
typedef struct symrec_s symrec_t;           /**< ::symrec_s */
typedef struct symrec_list_s symrec_list_t; /**< ::symrec_list_s */
typedef struct attr_box_s attr_box_t;       /**< ::attr_box_s */
typedef struct attr_net_s attr_net_t;       /**< ::attr_net_s */
typedef struct attr_port_s attr_port_t;     /**< ::attr_port_s */
typedef struct attr_prot_s attr_prot_t;     /**< ::attr_prot_s */
typedef struct attr_wrap_s attr_wrap_t;     /**< ::attr_wrap_s */
typedef enum symrec_type_e symrec_type_t;   /**< ::symrec_type_e */
typedef enum port_mode_e port_mode_t;       /**< ::port_mode_e */
typedef enum port_class_e port_class_t;     /**< ::port_class_e */

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
    char*           key;    /**< unique key of the symbol (hh key) */
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
 * @brief   Attributes of a box
 */
struct attr_box_s
{
    bool            attr_pure;  /**< a box can be pure (functional) */
    bool            attr_ext;   /**< location of the box signature */
    char*           impl_name;  /**< implementation name */
    symrec_list_t*  ports;      /**< pointer to the port list of the net */
};

/**
 * @brief   Attributes of a net
 */
struct attr_net_s
{
    virt_net_t* v_net;  /**< pointer to a virtual net */
    igraph_t    g;      /**< graph representing the inner net */
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
    int             ch_len;         /**< length of the channel */
    const char*     alt_name;       /**< alternative name of the port */
};

/**
 * @brief   Attributes of a net prototype
 */
struct attr_prot_s
{
    symrec_list_t*  ports;  /**< pointer to a linked list of ports */
};

/**
 * @brief   Attributes of a wrapper
 */
struct attr_wrap_s
{
    bool            attr_static;/**< wrapper does no proliferation */
    symrec_list_t*  ports;      /**< pointer to the port list of the net */
    virt_net_t*     v_net;      /**< pointer to a virtual net */
    igraph_t        g;          /**< graph representing the inner net */
};

// FUNCTIONS ------------------------------------------------------------------
/**
 * @brief   Create a box attribute structure
 *
 * @param attr_pure flag indicating whether a box is pure or not
 * @param attr_ext  flag indicating whether a box signature is external defined
 * @param impl_name name of the box implementation
 * @param ports     pointer to a port list
 * @return          pointer to the new structure
 */
attr_box_t* symrec_attr_create_box( bool attr_pure, bool attr_ext,
        char* impl_name, symrec_list_t* ports );

/**
 * @brief   Create a net attribute structure
 *
 * @param v_net pointer to a virtual net
 * @param g     pointer to a graph object
 * @return      pointer to the new structure
 */
attr_net_t* symrec_attr_create_net( virt_net_t* v_net, igraph_t* g );

/**
 * @brief   Create a port attribute structure
 *
 * @param port_int      pointer to record list of port names
 * @param mode          direction of the port
 * @param collection    class of the port
 * @param decoupled     flag indicating whether a port is decoupled
 * @param ch_len        length of the channel
 * @return              pointer to the new structure
 */
attr_port_t* symrec_attr_create_port( symrec_list_t* port_int, port_mode_t mode,
        port_class_t collection, bool decoupled, int ch_len );

/**
 * @brief   Create a net prototype attribute structure
 *
 * @param ports     pointer to a port list
 * @return          pointer to the new structure
 */
attr_prot_t* symrec_attr_create_proto( symrec_list_t* ports );

/**
 * @brief   Create a wrap attribute structure
 *
 * @param attr_static   flag incdicating whether a wrapper is static
 * @param ports         pointer to a port list
 * @param v_net         pointer to a virtual net
 * @param g             pointer to a graph object
 * @return              pointer to the new structure
 */
attr_wrap_t* symrec_attr_create_wrap( bool attr_static, symrec_list_t* ports,
        virt_net_t* v_net, igraph_t* g );

/**
 * @brief   Destroy attributes of a box symbol table record
 *
 * @param attr  pointer to the attribute
 */
void symrec_attr_destroy_box( attr_box_t* attr );

/**
 * @brief   Destroy attributes of net a symbol table record
 *
 * @param attr  pointer to the attribute
 * @param deep  if true, also destroy the grap attribute
 */
void symrec_attr_destroy_net( attr_net_t* attr, bool deep );

/**
 * @brief   Destroy attributes of a port symbol table record
 *
 * @param attr  pointer to the attribute
 */
void symrec_attr_destroy_port( attr_port_t* attr );

/**
 * @brief   Destroy attributes of a prototype symbol table record
 *
 * @param attr  pointer to the attribute
 */
void symrec_attr_destroy_proto( attr_prot_t* attr );

/**
 * @brief   Destroy attributes of a wrapper symbol table record
 *
 * @param attr  pointer to the attribute
 */
void symrec_attr_destroy_wrap( attr_wrap_t* attr );

/**
 * @brief   Create a symbol table record.
 *
 * Create a symbol table record where the attribute field is kept non
 * initialised.
 * @attention   the attribute field has to be initialised after the creation
 *
 * @param name      name of the record
 * @param scope     scope of the record
 * @param type      type of the record
 * @param line      position (line number) of the identifier
 * @param attr_key  a number derived from the attributes to create a unique key
 * @return          a pointer to the new record structure
 */
symrec_t* symrec_create( char* name, int scope, symrec_type_t type, int line,
        int attr_key );

/**
 * @brief   Create a box symbol table record.
 *
 * @param name      name of the record
 * @param scope     scope of the record
 * @param line      position (line number) of the identifier
 * @param attr      pointer to the specific attribute structure
 * @return          a pointer to the new record structure
 */
symrec_t* symrec_create_box( char* name, int scope, int line,
        attr_box_t* attr );

/**
 * @brief   Create a net symbol table record.
 *
 * @param name      name of the record
 * @param scope     scope of the record
 * @param line      position (line number) of the identifier
 * @param attr      pointer to the specific attribute structure
 * @return          a pointer to the new record structure
 */
symrec_t* symrec_create_net( char* name, int scope, int line,
        attr_net_t* attr );

/**
 * @brief   Create a port symbol table record.
 *
 * @param name      name of the record
 * @param scope     scope of the record
 * @param line      position (line number) of the identifier
 * @param attr      pointer to the specific attribute structure
 * @return          a pointer to the new record structure
 */
symrec_t* symrec_create_port( char* name, int scope, int line,
        attr_port_t* attr );

/**
 * @brief   Create a prototype symbol table record.
 *
 * @param name      name of the record
 * @param scope     scope of the record
 * @param line      position (line number) of the identifier
 * @param attr      pointer to the specific attribute structure
 * @return          a pointer to the new record structure
 */
symrec_t* symrec_create_proto( char* name, int scope, int line,
        attr_prot_t* attr );

/**
 * @brief   Create a wrapper symbol table record.
 *
 * @param name      name of the record
 * @param scope     scope of the record
 * @param line      position (line number) of the identifier
 * @param attr      pointer to the specific attribute structure
 * @return          a pointer to the new record structure
 */
symrec_t* symrec_create_wrap( char* name, int scope, int line,
        attr_wrap_t* attr );

/**
 * @brief    Remove a record from the symbol table and free the allocated space
 *
 * @param symtab pointer to the hashtable
 * @param rec    pointer to the record to be removed
 */
void symrec_del( symrec_t** symtab, symrec_t* rec );

/**
 * @brief   Remove all records from the symbol table
 *
 * @param recs  pointer to the hashtable
 */
void symrec_del_all( symrec_t** recs );

/**
 * @brief   free the memory of a symrec, excluding attributes
 *
 * @param rec   pointer to the symbol table record
 */
void symrec_destroy( symrec_t* rec );

/**
 * @brief   Remove all elements of a linked list
 *
 * @param list  pointer to the first element of a linked list
 */
void symrec_list_del( symrec_list_t* list );

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
symrec_t* symrec_get( symrec_t** symtab, UT_array* scope_stack, char* name,
        int line, int attr_key );

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
symrec_t* symrec_put( symrec_t** symtab, symrec_t* new_item );

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
symrec_t* symrec_search( symrec_t** symtab, UT_array* scope_stack, char* name,
        int attr_key );

/**
 * @brief   Print debug information of a port of a port record list
 *
 * @param port  pointer to the port record
 * @param name  name of the net instance to port belongs to
 */
void debug_print_rport( symrec_t* port, char* name );

/**
 * @brief   Print debug information of all ports in a port record list
 *
 * @param rports    pointer to the port record list
 * @param name      name of the net instance to port belongs to
 */
void debug_print_rports( symrec_list_t* rports, char* name );

#endif /* SYMTAB_H */
