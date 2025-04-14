#ifdef PRIM
#define PRIM(imm,str,func) init_dictionary_dict_add(imm,str,f_##func); if(0)
#else
#define PRIM(imm,str,func) static void f_##func(void)
#endif
PRIM(0, ".", dot)
{
    int len = 0;
    uintptr_t num = pop();
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
    while (len--)
	my_write(1, &buf[len], 1);
}
PRIM(1, "0x", hexread)
{
    uintptr_t rst = 0;
    int len = readword(here);
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
    push(rst);
}
PRIM(1, "(", comment) {
    while (1) {
	char *buf = here;
	int len = readword(buf);
	if (1 == len && ')' == buf[0])
	    break;
    }
}
PRIM(0, "c,", emit_byte)
{
    *(char *) here = pop();
    here++;
}
PRIM(0, "here", here)
{
    push((intptr_t) here);
}
PRIM(0, "[", begin_emit)
{
    postpone = 1;
}
PRIM(1, "]", end_emit)
{
    postpone = 0;
}
PRIM(0, "create", create)
{
    dict_entry *dict;
    void *execution_token;
    int len;
    dict = create_alloc();
    len = readword(here);
    here += len;
    /* assume code-field is not aligned. */
    execution_token = end_of_dictionary();
    create_fill(dict, execution_token, 0, len);
}
PRIM(0, "immediate", immediate)
{
    if (dictionary->imm_len >= 0)
	dictionary->imm_len = ~dictionary->imm_len;
}
PRIM(0, "compile,", compile_camma)
{
    compile((void *) tos);
    tos = *(psp++);
}
PRIM(1, "'", tick)
{
    void *execution_token;
    if (0 <= tick(here, &execution_token))
	push((intptr_t) execution_token);
}
PRIM(1, "postpone", postpone)
{
    postpone++;
    interpret();
    if (postpone > 0)
	postpone--;
}
PRIM(0, "dump", dump)
{
    my_write(2, memory, here - (void *) memory);
}
/*********/
PRIM(0, "allot", allot)
{
    here += pop();
}
PRIM(0, "align", align)
{
    here = alignup_ptr(here);
}
PRIM(0, "execute", execute)
{
    execute((void *) pop());
}
PRIM(1, "literal", literal)
{
    emit_lit(pop());
}
PRIM(0, "@", load_cell)
{
    tos = *((intptr_t *) tos);
}
PRIM(0, "!", store_cell)
{
    intptr_t *addr, value;
    addr = (intptr_t *) tos;
    value = psp[0];
    *addr = value;
    tos = psp[1];
    psp += 2;
}
PRIM(0, "does", does)
{
    void *xt = (void *) pop();
    void *here_back = here;
    here = end_of_dictionary();
    emit_lit((intptr_t) here_back);
    compile_core(0xe9, xt);
    here = here_back;
}
