囲碁の死活(詰碁)をminimax法により解析するプログラムを作成します。

入力:
SGFフォーマットで初期盤面が与えられます。
例: (;GM[1]FF[4]CA[UTF-8]SZ[19]RU[Chinese]KM[7.5]AW[qc][qd][re][qb][sd][pb]AB[pc][pd][qe][qf][ob][nc][rf][sf])
- ただし、簡単のため黒先白死の問題とします。将来的には黒先コウ、黒先生き、など結論が異なる問題も入力として用意する可能性があります。

出力:
初期盤面から、黒はその後白がどんな手を尽くしても白を死形にできる手だけを手順に組み込み、白はその後黒が手抜きできない手(黒が1度パスをすると、その後どんな手順を尽くしても白を生かしてしまう手)のすべてを手順に組み込みます。
回答例: (;GM[1]FF[4]CA[UTF-8]SZ[19]RU[Chinese]KM[7.5]AW[qc][qd][re][qb][sd][pb]AB[pc][pd][qe][qf][ob][nc][rf][sf];B[rd](;W[rc];B[sb](;W[pa];B[ra];W[se];B[sc])(;W[rb];B[se];W[pa];B[ra])(;W[ra];B[pa];W[se];B[sc])(;W[se];B[sc])(;W[sc];B[se];W[pa];B[ra]))(;W[sb];B[rc];W[rb];B[se])(;W[rb];B[se]))
- 黒番と白番で着手のポリシーが異なります。
  - 黒番は最善手を追求します。すなわち、白がどのような手を打っても白を殺せる手だけを着手します。(この手は局面に対して一意ではなく、複数ある可能性もあります)
  - 白番は相手が手抜けるかどうかのみ考えます。相手(黒番)が現局面からパスできない手のすべてを着手します。

言語指定: C++ (g++ Rev14, Build by MSYS2 project) 15.2.0
外部ライブラリの利用可

アルゴリズム:
再帰によって探索します。
局面とは、石の配置(その配置になった手順が違っても同一視する)のみを表す言葉とする。
局面Aで、手番 turn, 着点 P, これまでの手順をtree(初期盤面から着点Pまでの一本道の手順, treeはPを含む) として、その局面Aで turnの手番側がPに着手すると、白の死が確定するときにTrueを返し、白の生きが確定するときにFalseを返す関数をminimax-search(A, turn, P, tree)、局面Aから手番turnがPの着点に石を置いた局面を next(A, turn, P) と表す。またturn の次の手番を !turn で表す。 Pには Pass も許容する。

minimax-search(A, turn, P, tree) のフローは以下である。searchの各段階で、探索手順をTREEに登録することができる。

1. 局面next(A, turn, P)と手番!turn がここまでのminimax-searchですでに探索済みの局面か判定する。これは局面Aをhash化し、探索する中でメモ化したhashリストと照合することにより実現する。探索済みであればメモ化した True / Falseを返却して終了。未探索の局面であれば2.に進む。
  - hashメモの型は、[局面のハッシュ値, 手番, Bool] の三組みである。

2. 局面next(A, turn, P)が死形のリストに含まれているか判定する。
- 含まれている場合 hash メモリストに [hash(next(A, turn, P)), !turn, True] を追加する。
  - turnが黒番で、Pにpassが含まれていない場合、TREEにtreeを追加する。
  - Trueを返却して終了する。

3. 局面next(A, turn, P)が生形のリストに含まれているか判定する。含まれている場合、 hashメモリストに [hash(next(A,turn,P)), !turn, False] を追加して、Falseを返却して終了する。

4. treeにPassが含まれておらず、かつ !turn が黒番なら、minimax-search(next(A, turn, P), !turn , Pass, tree + Pass) を実行し、Trueが返却された場合は、hashメモリストに [hash(next(A,turn,P)), !turn, True] を追加し、Trueを返却して終了する。

5. 局面next(A, turn, P) から !turn が着手可能な着点のリストLを生成する。ただし、着点のリストは盤面全体から選ぶのではなく、石が置かれた内部の点から、囲碁のルール上打つことができる点をリスト化すれば十分である。例えば、入力で与えた例では、初期局面から候補手となりうる着点は[oa][pa][qa][ra][sa][rb][sb][rc][sc][rd][se]の11点のみであり、これが初期局面, 手番黒での着手可能な着点のリストLとなる。

6. l \in L についてのループで、minimax-search(next(A, turn, P), !turn, l, tree + l) を実行する。
  - !turn が黒番のとき、
     - True が返却された場合、ハッシュメモリストに[hash(A), turn, True] を追加する
     - 全てのlについて False が返却された場合、 ハッシュメモリストに[hash(next(A,turn,P)), !turn, False] を追加し、Falseを返却して終了する。
     - 一つ以上のTrue が見つかっていれば、Trueを返却して終了する。
  - !turnが白番のとき、
     - 全てのlについて True が返却された場合、ハッシュメモリスとの[hash(next(A,turn,P)), !turn, True]を追加し、Trueを返却して終了する
     - 一つでも False が見つかった場合、ループをブレイクし、ハッシュメモリストに[hash(next(A,turn,P)), !turn, False] を追加し、Falseを返却して終了する。


以上が minimax-searchのフローです。このロジックが正しいかを検証してください。