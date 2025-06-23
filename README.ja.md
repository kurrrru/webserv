# webserv

## 概要

`webserv` は、C++98で実装された高性能なHTTP/1.1ウェブサーバーです。このプロジェクトは、`epoll` を用いたノンブロッキングかつイベント駆動のアーキテクチャを採用しており、多数の同時接続を効率的に処理します。Nginxスタイルの柔軟な設定ファイルを通じて、サーバーのカスタマイズ性が高く、仮想サーバー、ロケーションごとのルーティング、静的コンテンツの配信、そしてCGI（Common Gateway Interface）による動的コンテンツの生成をサポートしています。

-----

## 技術スタック

  * **言語**: C++98
  * **ビルドシステム**: `make`
  * **I/Oモデル**: ノンブロッキングI/Oと`epoll`（Linux）
  * **プロトコル**: HTTP/1.1

-----

## 主な機能

  * **柔軟な設定**: 複数の仮想サーバーの設定を含む、Nginxライクな設定ファイルを使用してサーバーの動作をカスタマイズできます。
  * **仮想サーバー**: 単一のサーバーインスタンスで、複数のドメインやIP/ポートの組み合わせをホストします。
  * **ロケーションベースのルーティング**: 特定のURLパス（ロケーション）に対して、異なるルールや設定を適用します。
  * **HTTP/1.1メソッド**: `GET`、`HEAD`、`POST`、`DELETE`リクエストを完全にサポートしています。
  * **静的ファイルの配信**: HTML、CSS、画像などの静的ファイルを効率的に配信します。
  * **CGIの実行**: CGIスクリプト（例：Python、Bash）を実行して、動的なウェブページを生成します。サーバーはMETA変数を正しく設定し、`GET`および`POST`の両方のデータストリームを処理します。
  * **ファイルのアップロード**: `POST`リクエストによるファイルのアップロードを管理し、設定可能なボディサイズ制限と保存場所を提供します。
  * **ディレクトリリスティング**: `autoindex`が有効で、インデックスファイルが見つからない場合に、ディレクトリのリストページを自動的に生成して表示します。
  * **カスタムエラーページ**: 特定のHTTPエラーコード（例：404、500）に対して、カスタムHTMLページを定義できます。
  * **HTTPリダイレクト**: `return`ディレクティブを使用して、恒久的および一時的なリダイレクトをサポートします。

-----

## プロジェクト構成

リポジトリは以下のように構成されています。

```
.
├── conf/              # 設定ファイル
│   ├── default.conf
│   └── ...
├── docs/              # ウェブコンテンツのドキュメントルート
│   ├── cgi-bin/       # CGIスクリプトの例
│   └── html/          # HTML, CSS, JSファイルの例
├── src/               # ソースコード
│   ├── config/        # 設定ファイルのパーサーとバリデーション
│   ├── core/          # メインのサーバーロジック、クライアント処理
│   ├── event/         # Epollイベントループ管理
│   └── http/          # HTTPリクエスト/レスポンスの解析と処理
│       ├── cgi/
│       ├── parsing/
│       ├── request/
│       └── response/
├── toolbox/           # ユーティリティ関数
├── Makefile
└── webserv            # コンパイルされたサーバー実行ファイル
```

-----

## ビルドと実行

### 前提条件

  * C++98準拠のコンパイラ（例: `c++`）
  * `make`

### ビルド

プロジェクトをコンパイルするには、ルートディレクトリから以下のコマンドを実行してください。

```bash
make
```

### 実行

サーバーを起動するには、設定ファイルを引数として渡す必要があります。

```bash
./webserv [your_config_file.confへのパス]
```

設定ファイルが指定されていない場合、`conf/default.conf`が使用されます。

```bash
./webserv
```

-----

## 設定ディレクティブ

サーバーの動作は、設定ファイル内のディレクティブによって制御されます。以下に、利用可能な主要なディレクティブの一部を示します。

| ディレクティブ           | コンテキスト                | 説明                                                   | 例                                          |
| ---------------------- | --------------------------- | ------------------------------------------------------ | ------------------------------------------- |
| `listen`               | `server`                    | リッスンするポートとオプションのIPアドレスを指定します。 | `listen 8080;`                              |
| `server_name`          | `server`                    | 仮想サーバーの名前を定義します。                       | `server_name example.com www.example.com;`  |
| `root`                 | `http`, `server`, `location`| リクエストのルートディレクトリを設定します。             | `root /var/www/html;`                       |
| `index`                | `http`, `server`, `location`| 提供するデフォルトのファイルを指定します。               | `index index.html index.htm;`               |
| `allowed_methods`      | `http`, `server`, `location`| 許可するHTTPメソッドを制限します。                     | `allowed_methods GET POST;`                 |
| `client_max_body_size` | `http`, `server`, `location`| クライアントリクエストボディの最大許容サイズを設定します。| `client_max_body_size 8M;`                  |
| `error_page`           | `http`, `server`, `location`| 特定のエラーコードに対するカスタムページを定義します。   | `error_page 404 /404.html;`                 |
| `autoindex`            | `http`, `server`, `location`| ディレクトリリスティングを有効または無効にします。     | `autoindex on;`                             |
| `cgi_path`             | `http`, `server`, `location`| CGIインタプリタへのパスを指定します。                  | `cgi_path /usr/bin/python3;`                |
| `cgi_extension`        | `http`, `server`, `location`| ファイル拡張子をCGIスクリプトに関連付けます。          | `cgi_extension .py;`                        |
| `upload_store`         | `http`, `server`, `location`| アップロードされたファイルを保存するディレクトリを定義します。 | `upload_store /var/uploads;`                |
| `return`               | `server`, `location`        | HTTPリダイレクトを実行します。                         | `return 301 http://new.example.com;`        |

-----

## 使用例

サーバーの機能をテストするための`curl`コマンドの例をいくつか示します（サーバーが`localhost:8080`でデフォルト設定で実行されていると仮定します）。

**メインページを取得する:**

```bash
curl http://localhost:8080/
```

**CGIスクリプトをテストする (GET):**

```bash
curl http://localhost:8080/cgi-bin/script.py
```

**CGIスクリプトをテストする (POST):**

```bash
curl -X POST -d "name=Gemini" http://localhost:8080/cgi-bin/script.py
```

**ファイルをアップロードする:**

```bash
# まず、ダミーファイルを作成します
echo "This is a test file." > test.txt
# 次に、アップロードします
curl -X POST --data-binary "@test.txt" http://localhost:8080/uploads/
```

**アップロードされたファイルを削除する:**

```bash
curl -X DELETE http://localhost:8080/upload/test.txt
```

-----

## ライセンス

このプロジェクトは、MITライセンスの下でライセンスされています。
