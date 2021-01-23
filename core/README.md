tofu.core
=========
コアライブラリを実装するサブプロジェクトです。

## ディレクトリ構成
- src/ : ソースファイルが記述されています
- include/ : ヘッダファイルが記述されています

## Sources

### tofu/containers/ring_buffer.h
シンプルなリングバッファーです。テンプレート引数TOriginによって、先頭を0とするか最後尾を0とするか選択することができます。

### tofu/containers/stack_vector.h
スタック上にデータ領域を確保する動的配列です。
使用するデータ量が少量で、且つ上限が見積もれる際に使用することで、std::vector<T>よりも高速に動作することが期待されます。

### tofu/containers/triple_buffer.h
書き込みスレッドが断続的に更新するデータのうち、最新のものを読み込みスレッドが取得するためのクラスです。

### tofu/ecs/core.h
ゲーム実装上で最低限必要なものが定義されています。

### tofu/ecs/physics.h / cpp
box2dを使った物理シュミレーションを管理するクラスなどが実装されています。

### tofu/utils/circular_queue_allocator.h
循環バッファー3種が実装されています。
#### CircularBufferAllocator
循環バッファー上で任意サイズの領域を確保・解放できます。
#### CircularQueueBuffer
CircularBufferAllocatorをキュー的に利用するためのクラスです。
#### CircularContinuousBuffer
循環バッファーを連続した1データを表現するストリームと見立てて利用するためのクラスです。

### tofu/utils/error.h
実行時エラーを便利に表現するためのクラスです。
### tofu/utils/job.h
簡易ジョブシステムです。
### tofu/utils/observer_ptr.h
所有権を得ないポインタです。将来のC++に提案されているライブラリの部分的な実装です。
### tofu/utils/scheduled_update_thread.h
ある関数を等間隔に呼び出すスレッドを生成するためのクラスです。
### tofu/utils/service_locator.h
シンプルなサービスロケーターです。
### tofu/utils/strong_numeric.h
数値型の強い別名をつけるためのクラスです。
### tofu/utils/tvec2.h
box2dやSiv3Dの2次元ベクトル型と相互変換可能な2次元ベクトル型です。


