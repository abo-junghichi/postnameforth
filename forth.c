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
    return syscall3(SYS_read, 0, buf, 1);
}
static int my_write(int fd, const void *buf, size_t count)
{
    return syscall3(SYS_write, fd, buf, count);
}
static void debug(int len, const char *msg)
{
    my_write(2, msg, len);
}
void *here;
static int readword(void)
{
    int len = 0, rst;
    unsigned char *buf = here;
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
    unsigned char *name;
    dict_entry *prev;
};
dict_entry *dictionary;
#define MEMSIZE (0x10000)
/* assume memory[] is readable-writable-executable. */
intptr_t memory[MEMSIZE];
static int find(int word_length, unsigned char *word_addr,
		void **execution_token)
{
    dict_entry *dict = dictionary;
    while (dict > memory) {
	int i, len, imm;
	dict_entry *cur = dict - 1;
	dict = cur->prev;
	if (cur->imm_len < 0) {
	    imm = 1;
	    len = ~cur->imm_len;
	} else {
	    imm = -1;
	    len = cur->imm_len;
	}
	if (word_length != len)
	    goto next;
	for (i = 0; i < word_length; i++)
	    if (word_addr[i] != cur->name[i])
		goto next;
	*execution_token = dict;
	return imm;
      next:;
    }
    return 0;
}
#define PSPBASE (&memory[MEMSIZE])
register intptr_t tos asm("ebx"), *psp asm("esi");
//intptr_t tos, *psp;
static void memcpy(char *dest, const char *src, unsigned int n)
{
    while (n--)
	*(dest++) = *(src++);
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
static void reset(char *msg)
{
    int len;
    for (len = 0; '\0' != msg[len]; len++);
    here = dictionary;
    psp = PSPBASE;
    postpone = 0;
    my_write(1, msg, len);
}
static void *alignup_ptr(void *addr)
{
    intptr_t mask = sizeof(intptr_t) - 1;
    return (void *) ((((intptr_t) addr) + mask) & ~mask);
}
static void create(int word_imm, int word_length, unsigned char *word_addr)
{
    dict_entry *dict = alignup_ptr(here);
    dict->prev = dictionary;
    dict->name = word_addr;
    if (word_imm)
	word_length = ~word_length;
    dict->imm_len = word_length;
    here = dictionary = dict + 1;
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
#define FORTH(name) static void f_##name(void)
FORTH(dot)
{
    int len = 0;
    uintptr_t num = tos;
    char buf[8];
    static const char table[16] = {
	'0', '1', '2', '3', '4', '5', '6', '7', '8', '9',
	'A', 'B', 'C', 'D', 'E', 'F'
    };
    while (num) {
	buf[len++] = table[num & 0xf];
	num >>= 4;
    }
    if (0 == len) {
	buf[0] = '0';
	len = 1;
    }
    tos = *(psp++);
    while (len--)
	my_write(1, &buf[len], 1);
}
FORTH(hexread)
{
    uintptr_t rst = 0;
    int len = readword();
    unsigned char *head = here, *tail = &head[len];
    while (head != tail) {
	unsigned char c = *head;
	int adder;
	head++;
	if ('0' <= c && c <= '9')
	    adder = c - '0';
	else {
	    c &= ~0x20;
	    if ('A' <= c && c <= 'F')
		adder = c - 'A' + 10;
	    else
		break;
	}
	rst = (rst << 4) + adder;
    }
    if (postpone)
	emit_lit(rst);
    else {
	*--psp = tos;
	tos = rst;
    }
}
FORTH(comment)
{
    while (1) {
	char *buf = here;
	int len = readword();
	if (1 == len && ')' == buf[0])
	    break;
    }
}
FORTH(emit_byte)
{
    *(char *) here = tos;
    here++;
    tos = *(psp++);
}
FORTH(align)
{
    here = alignup_ptr(here);
}
FORTH(begin_emit)
{
    postpone = 1;
}
FORTH(end_emit)
{
    postpone = 0;
}
FORTH(create)
{
    unsigned char *word_addr = here;
    int len = readword();
    here += len;
    create(0, len, word_addr);
}
FORTH(immediate)
{
    dict_entry *dict = dictionary - 1;
    if (dict->imm_len >= 0)
	dict->imm_len = ~dict->imm_len;
}
FORTH(compile_camma)
{
    compile(tos);
    tos = *(psp++);
}
static void interpret(void)
{
    int rst, length;
    void *execution_token;
    length = readword();
    rst = find(length, here, &execution_token);
    if (!rst)
	reset("word not found.\n");
    else
	switch (rst = postpone - ((rst + 1) >> 1)) {
	case 2:
	    emit_lit(execution_token);
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
FORTH(postpone)
{
    postpone++;
    interpret();
    if (postpone > 0)
	postpone--;
}
FORTH(dump)
{
    my_write(2, memory, here - (void *) memory);
}
static void init_dictionary_dict_add(int imm, char *c_name,
				     void *code_addr)
{
    /* assume x86(32bit) */
    int len;
    compile_core(0xe9, code_addr);
    for (len = 0; '\0' != c_name[len]; len++);
    create(imm, len, c_name);
}
static void init_dictionary(void)
{
#define DICT_ADD(imm,name,code) init_dictionary_dict_add(imm,name,f_##code)
    DICT_ADD(0, ".", dot);
    DICT_ADD(1, "0x", hexread);
    DICT_ADD(1, "(", comment);
    DICT_ADD(0, "c,", emit_byte);
    DICT_ADD(0, "align", align);
    DICT_ADD(0, "[", begin_emit);
    DICT_ADD(1, "]", end_emit);
    DICT_ADD(0, "create", create);
    DICT_ADD(0, "immediate", immediate);
    DICT_ADD(1, "postpone", postpone);
    DICT_ADD(0, "dump", dump);
}
void _start(void)
//int main(void)
{
    here = dictionary = memory;
    init_dictionary();
    reset("");
    while (1)
	interpret();
}
