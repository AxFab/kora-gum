#include <kora/xml.h>

const char *XML_NODE_NAMES[] = {
  NULL, NULL, NULL, "#text", "#cdata-section",
  NULL, NULL, NULL, "#comment", "#document",
  NULL, "#document-fragment", NULL
};

const char *memrchr (const void *ptr, int byte, size_t length);

int memcnt (const void *ptr, int byte, size_t length) {
  int count = 0;
  const char *buf = (const char*)ptr;
  while (length) {
    if (*buf == byte) count++;
    --length;
    ++buf;
  }
  return count;
}

/* -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-= */

static void xml_error(int errno, ...)
{
  fprintf(stderr, "XML Error n°:%d\n", errno);
}

static void xml_parse_element(XML_node *node, const char *data, int lg)
{
  int start = 1, finish = 1;
  const char *end = memchr(data, ' ', lg);

  // Check start and finish
  if (!memcmp(data, "</", 2)) {
    node->build_flags_ = XML_BLD_CLOSING;
    start = 2;
  } else if (!memcmp(data, "<?", 2)) {
    node->build_flags_ = XML_BLD_CLOSED;
    start = 2;
    finish = 2;
  } else if (!memcmp(&data[lg - 2], "/>", 2)) {
    node->build_flags_ = XML_BLD_CLOSED;
    finish = 2;
  }

  if (end == NULL) {
    end = &data[lg -1];
  }

  // Extract the name
  node->node_name_ = malloc(end - data);
  memcpy(node->node_name_, &data[start], end - data - start);
  node->node_name_[end - data - start] = '\0';

  // Extract attribute
  if (!(node->build_flags_ & XML_BLD_CLOSING)) {
    xml_parse_attributes(node, end, lg - (end - data) - finish);
  }
}

static XML_node *xml_add_parsed_node(XML_node *cursor, XML_node *node, void* param)
{
  ((void)param);

  if (node->type_ != XML_ELEMENT) {
    xml_add_child_node(cursor, node);
  } else if (node->build_flags_ & XML_BLD_CLOSING) {
    if (strcmp(cursor->node_name_, node->node_name_)) {
      xml_error(XML_ERR_CLOSING);
    }
    return cursor->parent_;
  } else {
    xml_add_child_node(cursor, node);
    if (!(node->build_flags_ & XML_BLD_CLOSED)) {
      return node;
    }
  }
  return cursor;
}

static int xml_read_buffer_next(char *buffer, int len, int *pSz)
{
  int type;
  int idx = 0;
  const char *until;

  if (*buffer != '<') {
    // Read TEXT node
    while (idx < len && buffer[idx] != '<') {
      idx++;
    }

    *pSz = idx;
    return XML_TEXT;
  }

  // Read other nodes
  if (len >= 8 && !memcmp(buffer, "<[CDATA[", 8)) {
    type = XML_CDATA;
    until = "]]>";
  } else if  (len >= 2 && !memcmp(buffer, "<?", 2)) {
    type = XML_DECLARATION;
    until = "?>";
  } else if  (len >= 4 && !memcmp(buffer, "<!--", 4)) {
    type = XML_COMMENT;
    until = "-->";
  } else if  (len >= 2 && !memcmp(buffer, "<!", 2)) {
    type = XML_DOCTYPE;
    until = ">";
  } else {
    type = XML_ELEMENT;
    until = ">";
  }

  char *end = strstr(buffer, until);
  if (end == 0 || end - buffer > len) {
    xml_error(XML_ERR_BUFFER_TOO_SMALL);
    return 0;
  }

  end += strlen(until);
  *pSz = end - buffer;
  return type;
}

static void xml_indent(FILE *stream, int depth)
{
  while (depth-- > 0) {
    fprintf(stream, "  ");
  }
}

static void xml_write_attributes(FILE *stream, XML_node *node)
{
  XML_attribute *a = node->first_attribute_;
  while (a) {
    fprintf(stream, " %s=\"%s\"", a->key_, a->value_);
    a = a->next_;
  }
}

static void xml_write(FILE *stream, XML_node *node, int depth)
{
  XML_node *child;
  switch (node->type_) {
    case XML_DECLARATION:
      xml_indent(stream, depth);
      fprintf(stream, "<?%s", node->node_name_);
      xml_write_attributes(stream, node);
      fprintf(stream, "?>\n");
      return;
    case XML_ELEMENT:
        xml_indent(stream, depth);
        fprintf(stream, "<%s", node->node_name_);
        xml_write_attributes(stream, node);
        fprintf(stream, node->first_child_ ? ">\n" : "/>\n");
      break;
    // case XML_TEXT:
    //   printf("%s", node->litteral_);
  }

  if (!node->first_child_) {
    return;
  }

  child = node->first_child_;
  while (child) {
    xml_write(stream, child, depth + 1);
    child = child->next_sibling_;
  }

  switch (node->type_) {
    case XML_ELEMENT:
        xml_indent(stream, depth);
        fprintf(stream, "</%s>\n", node->node_name_);
      break;
  }
}

/* -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-= */

/* Add a new attribute to a XML node, XML_ELEMENT or XML_DECLARATION */
void xml_add_attribute(XML_node *node, const char* key, const char* value)
{
  if (node->type_ != XML_ELEMENT && node->type_ != XML_DECLARATION) {
    return;
  }

  XML_attribute *attribute = (XML_attribute*)malloc(sizeof(XML_attribute));
  attribute->key_ = strdup(key);
  attribute->value_ = strdup(value);
  attribute->next_ = NULL;
  if (node->first_attribute_ == NULL) {
    attribute->previous_ = NULL;
    node->first_attribute_ = attribute;
  } else {
    node->last_attribute_->next_ = attribute;
    attribute->previous_ = node->last_attribute_;
  }
  node->last_attribute_ = attribute;
}

XML_node *xml_create_node(int type, const char *name, const char *content)
{
  // Create node
  XML_node *node = (XML_node*)malloc(sizeof(XML_node));
  memset(node, 0, sizeof(*node));
  node->type_ = type;

  // Set name and content
  switch (type) {
    case XML_TEXT:
    case XML_COMMENT:
    case XML_CDATA:
      node->content_ = strdup(content);
    case XML_DOCTYPE:
      node->node_name_ = strdup(XML_NODE_NAMES[type]);
      break;
    case XML_DECLARATION:
    case XML_ELEMENT:
      node->node_name_ = strdup(name);
      break;
  }
  return node;
}

void xml_add_child_node(XML_node *parent, XML_node *child)
{
  child->parent_ = parent;
  child->previous_sibling_ = parent->last_child_;
  if (parent->last_child_ != NULL) {
    parent->last_child_->next_sibling_ = child;
  } else {
    parent->first_child_ = child;
  }
  parent->last_child_ = child;
  parent->children_count_++;
}

void xml_remove_node(XML_node *node)
{
  if (!node->parent_) {
    return;
  }

  if (node->previous_sibling_) {
    node->previous_sibling_->next_sibling_ = node->next_sibling_;
  } else {
    node->parent_->first_child_ = node->next_sibling_;
  }

  if (node->next_sibling_) {
    node->next_sibling_->previous_sibling_ = node->previous_sibling_;
  } else {
    node->parent_->last_child_ = node->previous_sibling_;
  }

  node->parent_->children_count_--;
  node->parent_ = NULL;
  node->previous_sibling_ = NULL;
  node->next_sibling_ = NULL;
}

/* ---------------------------- */

/* Parse XML attribute sub-strings,
 * beeing a serie of "key=quoted-value" split by spaces */
void xml_parse_attributes(XML_node *node, const char* buf, int lg)
{
  char scope = '\0'; // symbol use for parsing state (' ', '=', '"', '\'' or '\0')
  int i;
  int kp = 0, vp = 0;
  char *key = (char*)malloc(XML_BUF_XS_SIZE);
  char *value = (char*)malloc(XML_BUF_XS_SIZE);
  for (i = 0; i < lg; ++i) {
    if (scope == '=') {
      // We expect a quoted-value
      if (buf[i] == '"' || buf[i] == '\'') {
        scope = buf[i];
      } else {
        scope = ' ';
      }
    } else if (scope == '"' || scope == '\'' || scope == ' ') {
      if (buf[i] == scope) {
        // We can create the attribute
        key[kp] = '\0';
        value[vp] = '\0';
        xml_add_attribute(node, key, value);
        kp = vp = 0;
        scope = '\0';
      } else {
        value[vp++] = buf[i];
        if (vp == XML_BUF_XS_SIZE) {
          xml_error(XML_ERR_BUFFER_TOO_SMALL);
          break;
        }
      }
    } else if (buf[i] == '=') {
      scope = '=';
    } else if (buf[i] != ' ') {
      key[kp++] = buf[i];
      if (kp == XML_BUF_XS_SIZE) {
        xml_error(XML_ERR_BUFFER_TOO_SMALL);
        break;
      }
    } else if (kp != 0) {
      kp = vp = 0;
      scope = '\0';
    }
  }

  free(key);
  free(value);
}

XML_node *xml_parse_node(int nodetype, const char *data, int lg)
{
  // Create node
  XML_node *node = (XML_node*)malloc(sizeof(XML_node));
  memset(node, 0, sizeof(*node));
  node->type_ = nodetype;

  // Set name and content
  if (nodetype == XML_ELEMENT || nodetype == XML_DECLARATION) {
    xml_parse_element(node, data, lg);
  } else {
    node->node_name_ = strdup(XML_NODE_NAMES[nodetype]);
    node->content_ = (char*)malloc(lg+1);
    memcpy(node->content_, data, lg);
    node->content_[lg] = '\0';
  }

  return node;
}

/* ---------------------------- */

/* Serialization of XML DOM */
XML_node *xml_read_file(FILE *stream)
{
  return xml_read_file_with(stream, xml_add_parsed_node, NULL);
}

XML_node *xml_read_file_with(FILE *stream, XML_pusher pusher, void *param)
{
  int type;
  int len = 0; // Size of the data present on the buffer
  int size = 0; // Size of a single node data
  int row = 1, col = 1; // File cursor position

  XML_node *document = xml_create_node(XML_DOCUMENT, NULL, NULL);
  XML_node *cursor = document;
  XML_node *node;

  char *buffer = (char*)malloc(XML_BUF_SIZE);
  memset(buffer, 0, XML_BUF_SIZE);

  setvbuf(stream, NULL, _IONBF, 0);
  for (;;) {
    // Ensure the buffer is filled
    if (len < XML_BUF_SIZE && !feof(stream)) {
      len += fread(&buffer[len], 1, XML_BUF_SIZE - len - 1, stream);
    }
    if (len == 0) {
      break;
    }

    // Find next node data length
    type = xml_read_buffer_next(buffer, len, &size);
    if (type == 0) {
      free(buffer);
      return NULL;
    }

    // Parsing of single node
    node = xml_parse_node(type, buffer, size);
    if (node == NULL) {
      free(buffer);
      return NULL;
    }

    // Attach node to the document
    cursor = pusher(cursor, node, param);
    if (cursor == NULL) {
      xml_error(XML_ERR_FOUND_AFTER);
    }

    // Handler stream cursor position
    node->row_ = row;
    node->col_ = col;
    node->rows_ = memcnt(buffer, '\n', size);
    if (node->rows_) {
      const char *eol = memrchr(buffer, '\n', size);
      node->ecol_ = size - (eol - buffer);
    } else {
      node->ecol_ += size;
    }
    row += node->rows_;
    col = node->ecol_;

    // Shift buffer
    memmove(buffer, &buffer[size], XML_BUF_SIZE - size);
    memset(&buffer[XML_BUF_SIZE - size], 0, size);
    len -= size;
  }

  free(buffer);
  return document;
}

/* Write XML DOM to stream */
void xml_write_file(FILE *stream, XML_node *node, int opt) {
  ((void)opt);
  xml_write(stream, node, node->type_ == XML_DOCUMENT ? -1 : 0);
}

/* ---------------------------- */

/* Free all memory block occupied for the DOM */
void xml_free_node (XML_node *node) {
  XML_attribute *a, *b;
  a = node->first_attribute_;
  while (a) {
    b = a->next_;
    free(a->key_);
    free(a->value_);
    free(a);
    a = b;
  }

  XML_node *s, *t;
  s = node->first_child_;
  while (s) {
    t = s->next_sibling_;
    xml_free_node(s);
    s = t;
  }

  free(node->node_name_);
  if (node->content_) {
    free(node->content_);
  }
  free(node);
}
