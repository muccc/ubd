#ifndef __DEBUG_H_
#define __DEBUG_H_
#include <stdint.h>
#include <glib.h>

#include "packet.h"

#define ub_assert(expr)                  do { if G_LIKELY (expr) ; else \
                                               ub_assertion_message_expr (G_LOG_DOMAIN, __FILE__, __LINE__, G_STRFUNC, \
                                                 #expr); } while (0)
void debug_hexdump(uint8_t * data, uint16_t len);
void debug_packet(gchar *reporter, struct ubpacket* p);
void ub_assertion_message_expr        (const char     *domain,
                                         const char     *file,
                                         int             line,
                                         const char     *func,
                                         const char     *expr) G_GNUC_NORETURN;
#endif
