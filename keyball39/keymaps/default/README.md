# Keyball39 default keymap

## Board clipboard

`rawhid-rust` の対応版を各PCで起動し、このキーマップをビルドして両側へ書き込む。

1. 通常のコピー操作で文字列をPCのクリップボードに入れる。
2. **UTIL + C** (`MC_CSTR`) でボードへ保存する。
3. **UTIL + V** (`MC_CPST`) で保存内容を貼り付ける。トリガーと修飾キーは離す。

保存先はRAMの1スロット。先頭100 Unicodeコードポイントまでで、日本語・絵文字・改行に対応する。
結合絵文字などは見た目の1文字が複数コードポイントになる。電源断・リセットで内容は消える。
電源が維持されるKVM切り替えでは保存内容を保持する。

貼り付け先PCのクリップボードを保存内容へ置き換え、成功応答後にmacOSはCommand+V、
既知のターミナルはCtrl+Shift+V、それ以外はCtrl+Vを送る。
別のキーを押した場合、USB切断・サスペンド、または3秒のタイムアウトで待機中の貼り付けを中止する。
受信失敗・転送中断では保存済みスロットを変更しない。フラッシュへの書き込みは行わない。

LinuxはWaylandなら `wl-clipboard`、X11なら `xclip` が必要。
プロトコル、PC側の環境設定と制限は `~/workspace/rawhid-rust/README.md` を参照。
ボード側の処理は `features/clipboard_hid.c`、パケット定義は同名ヘッダーで管理する。

## Host regression tests

このディレクトリから実行する。テストは実機やPCのクリップボードを操作しない。

```sh
cc -std=c11 -Wall -Wextra -Werror -fsanitize=address,undefined \
  -DQMK_KEYBOARD_H='"qmk.h"' -Itests/stubs \
  tests/clipboard_hid_test.c features/clipboard_hid.c \
  -o /tmp/keyball-clipboard-test
ASAN_OPTIONS=detect_leaks=0 /tmp/keyball-clipboard-test
```

`detect_leaks=0` はptrace環境でも実行できるようにするための指定。
境界をまたぐUTF-8、400バイトの往復、異常な転送、タイムアウト、USB切り替え、
修飾キー・トリガー解放待ち、成功応答前に貼り付けないことを検証する。
