# Keyball39 default keymap

## Board clipboard

`rawhid-rust` の対応版を各PCで起動し、このキーマップをビルドして両側へ書き込む。

1. 通常のコピー操作で文字列をPCのクリップボードに入れる。
2. **UTIL + C** (`MC_CSTR`) でボードへ保存する。
3. **UTIL + V** (`MC_CPST`) で保存内容を貼り付ける。トリガーと修飾キーは離す。

先頭100 Unicodeコードポイントまでを、RP2040のフラッシュEEPROMへ保存する。
日本語・絵文字・改行に対応し、結合絵文字などは見た目の1文字が複数コードポイントになる。
保存完了後は電源断・リセットをまたいで保持し、起動時にRAMへ読み戻す。
PCを切り替えたら、接続先の `rawhid-rust` が再接続してから **UTIL + V** を押す。
保存先はUSBを接続している側のボードで、左右間の複製は行わない。
USBケーブルを反対側のボードへ差し替えた場合は、そのボードに保存されている内容が使われる。

貼り付け先PCのクリップボードを保存内容へ置き換え、成功応答後にmacOSはCommand+V、
既知のターミナルはCtrl+Shift+V、それ以外はCtrl+Vを送る。
別のキーを押した場合、USB切断・サスペンド、または3秒のタイムアウトで待機中の貼り付けを中止する。
受信失敗・転送中断では保存済みスロットを変更しない。
保存成功はフラッシュへの書き込み後に通知し、同じ内容の再保存・貼り付けでは書き込まない。
フラッシュ書き込みが失敗した場合は以前の内容を保持し、再起動まで新しい内容の保存を中止する。
EEPROM初期化では保存内容も消える。

永続化は `features/clipboard_storage.c` が担当する。既存のKeyball・薙刀設定を避け、
論理EEPROMの1024〜1855バイトを使う。416バイトのレコードを交互に更新し、
世代番号・CRC32・最後に書く有効マーカーで、不完全または破損したレコードを除外する。
QMKの既存ウェアレベリングを利用するが、そのログ整理ではEEPROM全体の消去・再書き込みが
発生するため、**保存処理の途中で電源を切った場合の保持は完全には保証しない**。
VIA・動的キーマップとの領域共用には未対応で、有効化するとビルド時にエラーにする。

LinuxはWaylandなら `wl-clipboard`、X11なら `xclip` が必要。
プロトコル、PC側の環境設定と制限は `~/workspace/rawhid-rust/README.md` を参照。
ボード側の処理は `features/clipboard_hid.c`、パケット定義は同名ヘッダーで管理する。

## Host regression tests

このディレクトリから実行する。テストは実機やPCのクリップボードを操作しない。

```sh
cc -std=c11 -Wall -Wextra -Werror -fsanitize=address,undefined \
  -DQMK_KEYBOARD_H='"qmk.h"' -Itests/stubs \
  -I../../../../../quantum/wear_leveling \
  tests/clipboard_hid_test.c features/clipboard_hid.c features/clipboard_storage.c \
  -o /tmp/keyball-clipboard-test
ASAN_OPTIONS=detect_leaks=0 /tmp/keyball-clipboard-test
```

`detect_leaks=0` はptrace環境でも実行できるようにするための指定。
境界をまたぐUTF-8、400バイトの往復、異常な転送、タイムアウト、USB切り替え、
修飾キー・トリガー解放待ち、成功応答前に貼り付けないことを検証する。
永続化のテストはフラッシュとRAMキャッシュを模擬し、再起動復元、破損レコードからの復帰、
書き込み途中の各位置での失敗、書き込み失敗後の再試行防止、同一内容の再書き込み省略、
空文字の保存、既存設定領域の保持を確認する。QMK内部のログ整理や実際の電源断は模擬しない。
