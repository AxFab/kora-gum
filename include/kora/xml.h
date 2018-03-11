#ifndef _KORA_XML_H
#define _KORA_XML_H 1

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define XML_BUF_SIZE 8192
#define XML_BUF_XS_SIZE 512

/* List XML node types */
#define XML_ELEMENT 1
#define XML_ATTRIBUTE 2
#define XML_TEXT 3
#define XML_CDATA 4
// 5   ENTITY_REFERENCE_NODE
// 6   ENTITY_NODE
#define XML_DECLARATION 7
#define XML_COMMENT 8
#define XML_DOCUMENT 9
#define XML_DOCTYPE 10
// 11  DOCUMENT_FRAGMENT_NODE
// 12  NOTATION_NODE

/* Create bulding/parsing flags */
#define XML_BLD_CLOSING  (1 << 0)
#define XML_BLD_CLOSED  (1 << 1)

/* List errors */
#define XML_ERR 1 // "Undefined error"
#define XML_ERR_CLOSING 2 // "Wrong closing type"
#define XML_ERR_FOUND_AFTER 3 //"Element found after the root element"
#define XML_ERR_BUFFER_TOO_SMALL 4

/* Define types */
typedef struct XML_node XML_node;
typedef struct XML_attribute XML_attribute;
typedef struct XML_builder XML_builder;

struct XML_node {
  int type_;
  int length_;
  int children_count_;
  char *litteral_;
  XML_node *parent_;
  XML_node *next_sibling_;
  XML_node *previous_sibling_;
  XML_node *first_child_;
  XML_node *last_child_;
  XML_attribute *first_attribute_;
  XML_attribute *last_attribute_;
  char *node_name_;
  char *content_;
  int build_flags_;
  int row_;
  int col_;
  int rows_;
  int ecol_;
};

struct XML_attribute {
  char *key_;
  char *value_;
  XML_attribute *next_;
  XML_attribute *previous_;
};

typedef XML_node * (*XML_pusher)(XML_node *cursor, XML_node *node, void *param);


/* Build XML document */
void xml_add_attribute(XML_node *node, const char* key, const char* value);
XML_node *xml_create_node(int type, const char *name, const char *content);
void xml_add_child_node(XML_node *node, XML_node *child);
void xml_remove_node(XML_node *node);

/* Parse XML sub-strings */
void xml_parse_attributes(XML_node *node, const char* buf, int lg);
XML_node *xml_parse_node(int nodetype, const char *data, int lg);

/* Serialization of XML DOM */
XML_node *xml_read_file(FILE *stream);
XML_node *xml_read_file_with(FILE *stream, XML_pusher pusher, void *param);
void xml_write_file(FILE *stream, XML_node *node, int opt);

/* Free all memory block occupied for the DOM */
void xml_free_node (XML_node *node);


#endif  /* _KORA_XML_H */
