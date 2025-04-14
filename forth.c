#include <stdint.h>
#include <unistd.h>
#include <sys/syscall.h>
static int syscall3(int syscall, int arg1, int arg2, int arg3)
{
    int ret;
    __asm__ volatile ("int $0x80":"=a" (ret):"a"(syscall), "b"(arg1),
		      "c"(arg2), "d"(arg3):"memory");
    return ret;
}
static int readkey(char *buf)
{
    return syscall3(SYS_read, 0, (int) buf, 1);
}
static int my_write(int fd, const void *buf, size_t count)
{
    return syscall3(SYS_write, fd, (int) buf, count);
}
static void debug(int len, const char *msg)
{
    my_write(2, msg, len);
}
void *here;
static int readword(unsigned char *buf)
{
    int len = 0, rst;
    while (1) {
	rst = readkey(buf);
	if (' ' < *buf || 0 >= rst)
	    break;
    }
    while (1) {
	len++;
	rst = readkey(&buf[len]);
	if (' ' >= buf[len] || 0 >= rst)
	    break;
    }
    return len;
}
typedef struct dict_entry dict_entry;
struct dict_entry {
    int imm_len;
    void *execution_token;
    dict_entry *prev;
    unsigned char name[1];
};
dict_entry *dictionary;
#define MEMSIZE (0x10000)
/* assume memory[] is readable-writable-executable. */
intptr_t memory[MEMSIZE];
static int find(int word_length, unsigned char *word_addr,
		void **execution_token)
{
    dict_entry *dict = dictionary;
    while (dict) {
	int i, len, imm;
	imm = (dict->imm_len < 0);
	if (imm)
	    len = ~dict->imm_len;
	else
	    len = dict->imm_len;
	if (word_length != len)
	    goto next;
	for (i = 0; i < word_length; i++)
	    if (word_addr[i] != dict->name[i])
		goto next;
	*execution_token = dict->execution_token;
	return imm;
      next:
	dict = dict->prev;
    }
    return -1;
}
static void memcpy(void *dest, const void *src, unsigned int n)
{
    char *d = dest;
    const char *s = src;
    while (n--)
	*d++ = *s++;
}
static void compile_core(char op, void *execution_token)
{
    /* assume x86(32bit) */
    char *dest = here;
    int disp;
    dest[0] = op;
    here += 5;
    disp = execution_token - here;
    memcpy(dest + 1, &disp, 4);
}
static void compile(void *execution_token)
{
    compile_core(0xe8, execution_token);
}
static void execute(void *execution_token)
{
    void (*subroutine)(void) = execution_token;
    (*subroutine) ();
}
int postpone;
static void *end_of_dictionary(void)
{
    int len = dictionary->imm_len;
    if (len < 0)
	len = ~len;
    return dictionary->name + len;
}
static void *alignup_ptr(void *addr)
{
    intptr_t mask = sizeof(intptr_t) - 1;
    return (void *) ((((intptr_t) addr) + mask) & ~mask);
}
static dict_entry *create_alloc(void)
{
    dict_entry *dict = alignup_ptr(here);
    here = dict->name;
    return dict;
}
static void create_fill(dict_entry *dict, void *execution_token,
			int word_imm, int word_length)
{
    if (word_imm)
	word_length = ~word_length;
    dict->imm_len = word_length;
    dict->execution_token = execution_token;
    dict->prev = dictionary;
    dictionary = dict;
}
static void emit_lit(intptr_t lit)
{
    static const char head[] = {
	0x8d, 0x76, 0xfc,	/* lea -0x4(%esi),%esi */
	0x89, 0x1e,		/* mov %ebx,(%esi) */
	0xbb			/* 4byte : mov $4byte,%ebx */
    };
    char *dest = here;
    here += 4 + sizeof(head);
    memcpy(dest, head, sizeof(head));
    dest += sizeof(head);
    memcpy(dest, &lit, 4);
}
/* prepare some margin for stack underflow */
#define PSPBASE (&memory[MEMSIZE-9])
register intptr_t tos asm("ebx"), *psp asm("esi");
//intptr_t tos, *psp;
static intptr_t pop(void)
{
    intptr_t rtn = tos;
    tos = *(psp++);
    return rtn;
}
static void push(intptr_t item)
{
    *--psp = tos;
    tos = item;
}
static void reset(char *msg)
{
    int len;
    for (len = 0; '\0' != msg[len]; len++);
    here = end_of_dictionary();
    psp = PSPBASE;
    postpone = 0;
    my_write(1, msg, len);
}
static int tick(unsigned char *buf, void **execution_token)
{
    int rst, length = readword(buf);
    rst = find(length, buf, execution_token);
    if (0 > rst)
	reset("word not found.\n");
    return rst;
}
static void f_compile_camma(void);
static void interpret(void)
{
    int rst;
    void *execution_token;
    rst = tick(here, &execution_token);
    if (0 <= rst)
	switch (postpone - rst) {
	case 2:
	    emit_lit((intptr_t) execution_token);
	    execution_token = f_compile_camma;
	    /* fall through */
	case 1:
	    compile(execution_token);
	    break;
	case 0:
	case -1:
	    execute(execution_token);
	    if (PSPBASE < psp)
		reset("stack underflow.\n");
	    break;
	default:
	    __builtin_unreachable();
	}
}
#include "prims.c"
static void init_dictionary_dict_add(int imm, char *c_name,
				     void *code_addr)
{
    int len;
    char *name;
    dict_entry *dict;
    dict = create_alloc();
    name = here;
    for (len = 0; '\0' != c_name[len]; len++)
	name[len] = c_name[len];
    here += len;
    create_fill(dict, code_addr, imm, len);
}
static void init_dictionary(void)
{
#include "prims.c"
}
void _start(void)
//int main(void)
{
    here = memory;
    dictionary = (void *) 0;
    init_dictionary();
    reset("");
    while (1)
	interpret();
}
