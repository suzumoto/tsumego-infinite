# 無限詰碁 (Infinite Tsumego)

ユーザーの現在のレーティング（Elo）に合わせて、適切な難易度の詰碁問題を自動出題するWebアプリケーションです。SGFファイルを解析し、反転や回転などの対称操作をランダムに加えることで、暗記に頼らない実戦的な読みの力を鍛えることを目的としています。

## 🏛 システム構成 (Tech Stack)

フロントエンドとバックエンドを分離したアーキテクチャを採用しています。

### フロントエンド
* **Framework:** React + Vite
* **Language:** JavaScript / JSX
* **Hosting:** Vercel
* **主要ディレクトリ:** `tsumego-vite/`
* **備考:** ユーザーのブラウザ上で盤面を描画し、バックエンドAPIと通信して問題データや正誤判定を行います。

### バックエンド
* **Framework:** FastAPI (Uvicorn)
* **Language:** Python 3 (Python 3.14検証済)
* **Hosting:** Render (Web Service / Free Tier)
* **Go Logic:** `sgfmill` (SGFファイルの構文解析用)
* **ORM:** SQLAlchemy + `psycopg2-binary`
* **主要ディレクトリ:** `tsumego-backend/`

### データベース
* **Database:** PostgreSQL (Supabase)
* **特徴:** 本番環境（Render）のIPv6通信制限を回避するため、Supavisor（Connection Pooler / ポート6543）を利用したIPv4接続を行っています。

---

## 📝 現在の主な仕様

1. **問題の抽出ロジック**
   * データベース(`problems`テーブル)の中から、リクエストされた `user_elo` と問題の `elo` の差が最も小さいものを5件取得し、その中からランダムに1件を出題します。
2. **盤面のランダム化 (`sym_mode`)**
   * SGFから読み込んだ初期配置や正解ツリーの座標に対して、0〜7の対称操作（回転・反転）をランダムに適用してフロントエンドへ返却します。
3. **SGFファイルの動的インポート**
   * バックエンドの `import_sgf.py` を実行することで、ローカルの `data/problems/*.sgf` をDBに一括登録・更新します。ファイル名で重複をチェックし、内容が変更されている場合のみ上書き更新します。

---

## 🛠 ローカル開発環境の構築手順

開発に参加するには、Node.js および Python の環境が必要です。

### 1. リポジトリのクローン
```bash
git clone git@github.com:suzumoto/tsumego-infinite.git
cd tsumego-infinite
```

### 2. データベース環境変数の設定
`tsumego-backend` フォルダ内に `.env` ファイルを作成し、Supabaseの接続URLを記述します。
**※ `.env` ファイルはGitの管理対象外（`.gitignore` 指定済）です。絶対にコミットしないでください。**

```env
# tsumego-backend/.env
# 必ず Connection Pooler (ポート6543) のURLを使用してください
DATABASE_URL=postgresql://postgres.[PROJECT_ID]:[YOUR-PASSWORD]@aws-0-ap-northeast-1.pooler.supabase.com:6543/postgres
```

### 3. バックエンドの起動
Pythonの仮想環境を作成し、必要なライブラリをインストールしてからFastAPIサーバーを起動します。

**Windows (PowerShell) の場合:**
```powershell
cd tsumego-backend
python -m venv venv
.\venv\Scripts\activate
pip install -r requirements.txt
fastapi dev ./main.py --port 8000
```

### 4. フロントエンドの起動
別のターミナルを開き、Vite開発サーバーを起動します。

```powershell
cd tsumego-vite
npm install
npm run dev
```
起動後、ブラウザで `http://localhost:5173` （またはViteが指定したポート）にアクセスしてください。

---

## 📦 データの管理と運用 (SGFファイルの追加)

ローカル環境で新しい詰碁問題を追加したり、既存の問題を修正したりした場合は、以下の手順でデータベースを同期します。

1. `tsumego-backend/data/problems/` フォルダに `.sgf` ファイルを配置（または上書き）します。
2. バックエンドの仮想環境が有効な状態で、以下のコマンドを実行します。

```powershell
cd tsumego-backend
python import_sgf.py
```
*※ ファイルが新規であれば `INSERT`、内容が変更されていれば `UPDATE` が自動で行われます。*

---

## 🚀 デプロイメント

本プロジェクトはGitHubと連携した自動デプロイメント（CI/CD）が構築されています。

* `main` ブランチにプッシュすると、**Vercel**（フロントエンド）および **Render**（バックエンド）の両方で自動的にビルドとデプロイが開始されます。
* **注意点:** * Renderの無料枠を使用しているため、一定時間アクセスがないとバックエンドサーバーがスリープします。スリープからの復帰には数十秒〜数分かかる場合があります。
  * フロントエンド（Vercel）側のビルドの方が早く完了するため、API側の仕様変更を伴うリリースの直後は、一時的に通信エラーが発生する可能性があります。
  * クラウド上で環境変数（DBのパスワードなど）を追加・変更した場合は、Vercel/Renderのダッシュボードから手動で環境変数を設定し直す必要があります。

---

## 👨‍💻 開発者情報
**Author:** Motoi Morishita