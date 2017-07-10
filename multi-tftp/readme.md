# Network lab 2

Complete List

- [x] Print message
- [x] Unicast transfer
- [x] Multicast transfer
- [x] Multi-media transfer
- [x] Multi-thread

執行方式：

1.make

compile完會建一個接收的recvs資料夾
warning是對特殊字元的，可以忽略

2.執行
```bash
./mserv [要傳的檔案] [multicast/unicast]

./client [server_address] [client_address] [multicast/unicast] [如果是multicast的話，標注一個number以分辨不同檔案]
```

範例：

```bash
# multicast

./mserv 123.mp4 multicast
./client 140.116.122.22 140.118.222.00 multicast 1
./client 140.116.122.22 140.118.222.01 multicast 2
./client 140.116.122.22 140.118.222.02 multicast 3

# unicast

./mserv ladygaga.mp3 unicast
./client 140.119.222.12 140.223.111.00 unicast
./client 140.119.222.12	140.223.112.01 unicast
./client 140.119.222.12	140.223.113.01 unicast
```
* 執行狀況解說：

目前我把 client 連進 server 後觸發為3個client經tcp連線進去之後，就開始做multicast/unicast
所以client數現在是寫死服務三個的。

然後可能會有一些(初始化/memory問題？)讓我程式執行偶爾會有在開始執行接收檔案長度的時候會出問題，
此時重新執行/編譯，清空再執行即可。

* 程式簡單解說(demo時再細講)：

server端：

server端我是按multi-thread的sample code做修改的，

1. 做基礎設定的main function
2. connection handler(tcp connection)
滿三個client之後，抓取其ip address
並且判斷multicast/unicast，再呼叫trans_handler function，做udp file transfer
3. trans_handler function
傳送檔案的function

client端：
1. 基礎設定
2. tcp 連線完後，做udp multicast/unicast 的接收

* 特別說明：

multi-thread multicast的寫法

我利用frame(packet)的seq來做parallel傳送
例：
thread1傳餘數為1的
thread2傳餘數為2的
...

multi-thread unicast的寫法

multi-thread unicast我是用sequential的方式來傳送
首先利用thread number(thread id)來做exclusion
等前一條thread(client)傳送完後再做接續的動作


* 由於沒有做FEC，所以就沒有做實驗數據了
