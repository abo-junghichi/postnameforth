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

ワード名宣言をその本体定義の後(post-)で行うforth方言。

通常のforth言語では、
ワード本体定義中にそのワード名が現れるケースで問題が生じる。
世界最小の処理系を謳う[milliForth][milli]の様に、
ワード名宣言時に直ちに辞書にエントリを追加する素朴な実装では、
先のケースは再帰呼び出しにコンパイルされる。
しかし、局所変数の退避・復元など、再帰呼び出しには様々な配慮が必要となるので、
多くのforth処理系では、ワード本体定義終了まで
ワード名の辞書追加を遅らせるための仕組みを追加している。
対して当方言では、ワード名宣言とその本体定義の表記順序を最適化することで、
素朴な実装のまま対処している。

[milli]: https://github.com/fuzzballcat/milliForth
