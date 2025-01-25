## リンクレイヤーでパケットをキャプチャする練習用コード
書籍「ルーター自作でわかるパケットの流れ」（`https://gihyo.jp/book/2011/978-4-7741-4745-1`）に掲載されている「ltest」というプログラムを、ほぼそのまま書いただけのもの。  
L2 でパケットをキャプチャして、MAC 情報を表示するだけのもの。

## コンパイル
```
g++ -g -Wall ltest.cpp -o ltest
```

## 実行例
キャプチャするデバイス名を指定して実行する。  
パケットを 10個受信したら、終了するようになっている。
```
./ltest enp1s0

ether_header----------------------------
ether_dhost = fc:aa:14:25:37:4e
ether_shost = d8:43:ae:91:73:49
ether_type = 800(IP)
ether_header----------------------------
ether_dhost = d8:43:ae:91:73:49
ether_shost = fc:aa:14:25:37:4e
ether_type = 800(IP)
ether_header----------------------------
ether_dhost = d8:43:ae:91:73:49
ether_shost = fc:aa:14:25:37:4e
ether_type = 800(IP)
...
```
