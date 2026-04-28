import os
from dotenv import load_dotenv
from sqlalchemy import create_engine, Column, Integer, String, Text
from sqlalchemy.orm import sessionmaker, declarative_base

# .envファイルからDATABASE_URLを読み込む
load_dotenv()
DATABASE_URL = os.getenv("DATABASE_URL")

if not DATABASE_URL:
    print("エラー: 環境変数 DATABASE_URL が設定されていません。.envファイルを確認してください。")
    exit(1)

# データベース接続設定
engine = create_engine(DATABASE_URL)
SessionLocal = sessionmaker(autocommit=False, autoflush=False, bind=engine)
Base = declarative_base()

# main.py と同じモデル定義
class ProblemModel(Base):
    __tablename__ = "problems"
    id = Column(Integer, primary_key=True, index=True)
    filename = Column(String, unique=True, index=True)
    sgf_content = Column(Text)
    elo = Column(Integer, default=1500)
    success_count = Column(Integer, default=0)
    fail_count = Column(Integer, default=0)

def import_sgfs():
    db = SessionLocal()
    data_dir = "data/problems"
    
    if not os.path.exists(data_dir):
        print(f"ディレクトリが見つかりません: {data_dir}")
        return

    files = [f for f in os.listdir(data_dir) if f.endswith(".sgf")]
    if not files:
        print(f"{data_dir} 内にSGFファイルが見つかりません。")
        return

    # テーブルが存在しない場合は作成
    Base.metadata.create_all(bind=engine)

    added_count = 0
    for filename in files:
        # 既に同じファイル名が登録されているかチェック（重複登録の防止）
        existing = db.query(ProblemModel).filter(ProblemModel.filename == filename).first()
        if existing:
            print(f"スキップ: {filename} (既に登録されています)")
            continue
            
        filepath = os.path.join(data_dir, filename)
        with open(filepath, "r", encoding="utf-8") as f:
            content = f.read()
            
        new_prob = ProblemModel(
            filename=filename,
            sgf_content=content,
            elo=1500  # 初期レーティング
        )
        db.add(new_prob)
        added_count += 1
        print(f"登録: {filename}")
        
    db.commit()
    db.close()
    print(f"\n完了! {added_count} 件のSGFファイルをデータベースに新しく登録しました。")

if __name__ == "__main__":
    import_sgfs()