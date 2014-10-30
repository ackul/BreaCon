global tool_busy_loop
global tool_busy_break
global tool_busy_length

section .text

; entry point to the busy loop 'function'.
; note: do not call this directly--jump here!
; modifies rip and rcx and in some cases, rflags.
tool_busy_loop:
    xchg rax, rax
    xchg rbx, rbx
    xchg rdx, rdx
    xchg rsi, rsi
    xchg rdi, rdi
    xchg rsp, rsp
    xchg rbp, rbp
    dec rcx
    cmp rcx, 0
    jnz tool_busy_loop

; the breakpoint used to break out of the loop.
tool_busy_break:    
    db 0xcc
    times 8 - ($ - $$) % 8 db 0x90
tool_end:
    
align 8

; length of the busy loop code to be copied.
tool_busy_length:
    dq (tool_end - tool_busy_loop)
