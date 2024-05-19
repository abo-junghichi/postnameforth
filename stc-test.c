#include <stdio.h>
#include <stdlib.h>
char *msg;
char jit[11];
void *dict_here = jit;
void hello(void)
{
    msg = "hello,world!\n";
}
void bye(void)
{
    printf("bye!\n");
}
void c_compile(void *execution_token)
{
    /* assume x86(32bit) */
    int i;
    union {
	char byte[4];
	int disp;
    } u;
    char *dist = dict_here;
    dist[0] = 0xe8;
    dict_here += 5;
    u.disp = execution_token - dict_here;
    for (i = 0; i < 4; i++)
	dist[1 + i] = u.byte[i];
}
int main(void)
{
    void (*subroutine)(void) = jit;
    c_compile(hello);
    c_compile(bye);
    *((char *) dict_here) = 0xc3;
    (*subroutine) ();
    printf(msg);
    return 0;
}
