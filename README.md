#postnameforth

This is a dialect of forth language
which names each of words after the definion of it's body.

There occurs a problem in ordinary forth
when a word whose name is same with the defining word's one
appears in the difinition of the body.
In the simple implementation such as [milliForth][milli]
which claims the smallest real programming language,
the word is added to the dictionary immediately
before the difinition of it's body
when it's name is declared.
So such a word is compiled as a recursive function.
but because recursive functions needs additional attentions
such as preserving local variables,
most of forth languages employ some tricks
which delay adding a word to the dictionary
till the end of the definition of it's body
to avoid generating recursive functions.
On the other hand, this dialect keeps implementation simple
at avoiding the problem by optimizing the word order.

## for Japanese

���̾����򤽤���������θ�(post-)�ǹԤ�forth������

�̾��forth����Ǥϡ�
������������ˤ��Υ��̾������륱���������꤬�����롣
�����Ǿ��ν����Ϥ���[milliForth][milli]���ͤˡ�
���̾�������ľ���˼���˥���ȥ���ɲä������Ѥʼ����Ǥϡ�
��Υ������ϺƵ��ƤӽФ��˥���ѥ��뤵��롣
���������ɽ��ѿ������������ʤɡ��Ƶ��ƤӽФ��ˤ��͡�����θ��ɬ�פȤʤ�Τǡ�
¿����forth�����ϤǤϡ�������������λ�ޤ�
���̾�μ����ɲä��٤餻�뤿��λ��Ȥߤ��ɲä��Ƥ��롣
�Ф����������Ǥϡ����̾����Ȥ������������ɽ��������Ŭ�����뤳�Ȥǡ�
���Ѥʼ����Τޤ��н褷�Ƥ��롣

[milli]: https://github.com/fuzzballcat/milliForth
