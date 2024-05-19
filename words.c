#include <stdint.h>
#include <sys/syscall.h>
register intptr_t tos asm("ebx"), *psp asm("esi");
#define FORTH(name) void f_##name(void)
FORTH(sub)
{
    tos = *(psp++) - tos;
}
FORTH(sub_asm)
{
    __asm__ volatile ("movl (%esi),%eax\n" "leal 0x4(%esi),%esi\n"
		      "subl %ebx,%eax\n" "movl %eax,%ebx\n");
}
FORTH(add)
{
    tos = *(psp++) + tos;
}
FORTH(add_asm)
{
    __asm__ volatile ("addl (%esi),%ebx\n" "leal 0x4(%esi),%esi\n");
}
FORTH(lit_template)
{
    *--psp = tos;
    tos = 0x76543210;
}
FORTH(lit_template_asm)
{
    __asm__ volatile ("leal -0x4(%esi),%esi\n" "movl %ebx,(%esi)\n"
		      "movl $0x76543210,%ebx\n");
}
static int syscall1(int syscall, int arg1)
{
    int ret;
    __asm__ volatile ("int $0x80":"=a"(ret):"a"(syscall),
		      "b"(arg1):"memory");
    return ret;
}
FORTH(bye)
{
    syscall1(SYS_exit, 0);
    __builtin_unreachable();
}
