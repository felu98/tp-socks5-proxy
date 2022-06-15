#include "../../include/parsers/admin_edit.h"

void admin_edit_parser_init (struct admin_edit_parser *p) {
  p -> state = admin_edit_action;
  p -> remaining = 0;
  p -> read = 0;
}

static void remaining_set(admin_edit_parser *p, const int n) {
  p -> remaining = n;
  p -> read = 0;
}

static bool remaining_is_done(admin_edit_parser *p) {
  return p -> read >= p -> remaining;
}

enum admin_edit_state action(admin_edit_parser *p, uint8_t b) {
  enum admin_edit_state next = admin_edit_error_action;
  
  if (b = EDIT_ACTION) {
    p -> action = b;
    next = admin_edit_field;
  }

  return next;
}

enum admin_edit_state field(admin_edit_parser *p, uint8_t b) {
  enum admin_edit_state next = admin_edit_error_field;
  
  if (b = USERS_FIELD) {
    p -> field = b;
    next = admin_edit_keylen;
  }

  return next;
}

enum admin_edit_state key(admin_edit_parser *p, uint8_t b) {
  enum admin_edit_state next = admin_edit_key;
  *( (p->key) + p->read ) = b;
    p -> read ++;

    if (remaining_is_done(p)) {
      *( (p->key) + p->read ) = '\0';
      next = admin_edit_attribute;
    }

  return next;
}

enum admin_edit_state attribute(admin_edit_parser *p, uint8_t b) {
 enum admin_edit_state next = admin_edit_error_attribute;

 if (b == USERNAME_ATTRIBUTE || b == PASSWORD_ATTRIBUTE) {
   next = admin_edit_valuelen;
 }

 return next;
}

enum admin_edit_state value(admin_edit_parser *p, uint8_t b) {
  enum admin_edit_state next = admin_edit_value;
  *( (p->value) + p->read ) = b;
    p -> read ++;

    if (remaining_is_done(p)) {
      *( (p->value) + p->read ) = '\0';
      next = admin_edit_done;
    }

  return next;
}

enum admin_edit_state admin_edit_parser_feed(admin_edit_parser *p, uint8_t b) {
  enum admin_edit_state next;

  switch (p -> state) {
  case admin_edit_action:
    next = action(p,b);
    break;
  case admin_edit_field:
    next = field(p,b);
    break;
  case admin_edit_keylen:
    if (b <= 0) {
      next = admin_edit_error_keylen;
    } else {
      remaining_set(p,b);
      p -> keylen = b;
      next = admin_edit_key;
    }
    break;
  case admin_edit_key:
    next = key(p,b);
    break;
  case admin_edit_attribute:
    next = attribute(p,b);
    break;
  case admin_edit_valuelen:
    if (b <= 0) {
      next = admin_edit_error_valuelen;
    } else {
      remaining_set(p,b);
      p -> valuelen = b;
      next = admin_edit_value;
    }
    break;
  case admin_edit_value:
    next = value(p,b);
    break;
  case admin_edit_done:
  case admin_edit_error:
  case admin_edit_error_action:
  case admin_edit_error_field:
  case admin_edit_error_keylen:
  case admin_edit_error_valuelen:
  case admin_edit_error_attribute:
  default:
    log_print(FATAL, "Invalid state %d.\n", p->state);
    break;
  }
}

bool admin_edit_is_done (const enum admin_edit_state state, bool *err) {
  bool done;
  switch (state) {
  case admin_edit_error:
  case admin_edit_error_action:
  case admin_edit_error_field:
  case admin_edit_error_keylen:
  case admin_edit_error_valuelen:
  case admin_edit_error_attribute:
    if (err != 0)
    {
      *err = true;
    }
    done = true;
    break;
  case admin_edit_done:
    done = true;
    break;
  default:
    done = false;
    break;
  }

  return done;
}

enum admin_edit_state admin_edit_consume (buffer *buff, struct admin_edit_parser *p, bool *err) {

  enum admin_edit_state state = p->state;

  while (buffer_can_read(buff)) {
    const uint8_t c = buffer_read(buff);
    state = admin_edit_parser_feed(p, c);
    if (admin_edit_is_done(state, err)) {
      break;
    }
  }

  return state;
}

extern int admin_edit_marshall (buffer *b, const uint8_t status) {
  size_t n;
  uint8_t *buff = buffer_write_ptr(b, &n);

  if (n < 1) {
    return -1;
  }
  buff[0] = status;
  buffer_write_adv(b, n);
  return 1; 
}