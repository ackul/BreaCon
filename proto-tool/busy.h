#ifndef BUSY_H
#define BUSY_H
/* these are defined in busy.s and are read-only. */
extern const char tool_busy_loop; // first byte of busy loop
extern const char tool_busy_break; // breakpoint out of loop
extern const long tool_busy_length; // length of the code

#endif
