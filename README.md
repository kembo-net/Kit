# Kit
gitへのコミットを予告するコマンド

##機能(予定)
###kit init ...
Gitリポジトリ内の場合は.kitstackを生成する。
そうでない場合は任意のオプション付きでgit initを行ってから.kitstackを生成する。
###kit do "..."
git commitの予定を.kitstackに積む。
###kit do after "..."
###kit do (after)? [0-9]+ "..."
末尾、もしくは特定の場所に予定を追加する。
###kit done ([0-9]+)?
.kitstackに最後に積まれた予定をコミットする。
数字のオプションが付いた場合は最後に積まれたものから
n番目(先頭を0とする)の予定をコミットする。
実行された予定は破棄される。(破棄してから実行する)
###kit now
.kitstackの一番上を表示
###kit list
.kitstackの内容を番号付きで表示
###kit edit "..." ([0-9]+)?
.kitstackに保存された予定を書き換える
###kit remove ([0-9]+)?
.kitstackから最後に積まれた予定を破棄する。

##一言
これ英語にした方が良いんだろうなぁ。英語苦手だなぁ。
