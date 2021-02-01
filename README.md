Tofu
===
バスケットボールをモチーフにしたネットワークマルチプレイアクションゲームの小規模な実装です。

![Video](video.gif)

## 実装の目的
- QUICをネットワークゲームの通信に使ってみる
- ネットワークゲームの同期システムを自分で書いてみる
- C++20でアプリケーションを書いてみる

## ディレクトリ構成
- ball-client: バスケットボールゲーム(以下ball game)の入力・レンダラ及びクライアントアプリケーション本体が記述されています
- ball-core: ball gameの主な処理が記述されています
- cert: quic通信時に用いる証明書が格納されています
- core-test: コアライブラリのテストが記述されています
- core: コアライブラリです
- libs-build: サードパーティ製ライブラリをビルドするために必要なファイルが格納されています
- libs: サードパーティ製ライブラリが格納されています
- quic: picoquicをラップしたC++ライブラリが記述されています
- sandbox: quicライブラリを使った簡単な通信アプリケーションが書かれています (機能チェック用に実装したもの)

## Requirement
### Windows
- CMake >= 3.16
- Visual Studio 2019 >= 16.8.4
- [OpenSiv3D](https://github.com/Siv3D/OpenSiv3D) >= 0.4.3
### Linux
- CMake >= 3.16
- g++ >= 10.2.0

## Build
### Windows
1. patchを当てる (TODO:作る)
2. Tofu/libs/picotls/picotlsvs/picotlsvs.sln をpicotlsのREADME.mdを参考にビルドする
3. Tofu/libs/picoquic/picoquic.sln をpicoquicのREADME.mdを参考にビルドする
4. Tofu/run-cmake-win.cmd を叩く
5. Tofu/build-win/tofu.sln をビルドする
6. Tofu/ball-client/ball_client.sln をビルドする

### Linux
1. patchを当てる (TODO:作る)
2. Tofu/libs/picotls/CMakeLists.txt をpicotlsのREADME.mdを参考にビルドする
3. Tofu/libs/picoquic/CMakeLists.txt をpicoquicのREADME.mdを参考にビルドする
4. Tofu/build.sh を叩く

## 今後の課題
- std::mutexだと大振りすぎる部分をspinlockに置き換える
- 非完全同期方式の採用


