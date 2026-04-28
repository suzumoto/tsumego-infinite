import os
from dotenv import load_dotenv
from sqlalchemy import create_engine, Column, Integer, String, Text
from sqlalchemy.orm import sessionmaker, declarative_base

# .envファイルからDATABASE_URLを読み込む
load_dotenv()
DATABASE_URL = os.getenv("DATABASE_URL")

if not DATABASE_URL:
    print("エラー: 環境変数 DATABASE_URL が設定されていません。")
    exit(1)

# データベース接続設定
engine = create_engine(DATABASE_URL)
SessionLocal = sessionmaker(autocommit=False, autoflush=False, bind=engine)
Base = declarative_base()

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
    
    # テーブルが存在しない場合は作成
    Base.metadata.create_all(bind=engine)

    added_count = 0
    updated_count = 0

    for filename in files:
        filepath = os.path.join(data_dir, filename)
        with open(filepath, "r", encoding="utf-8") as f:
            new_content = f.read()
            
        # 既存のレコードをファイル名で検索
        existing = db.query(ProblemModel).filter(ProblemModel.filename == filename).first()

        if existing:
            # 内容が変更されている場合のみ更新
            if existing.sgf_content != new_content:
                existing.sgf_content = new_content
                updated_count += 1
                print(f"更新: {filename} (内容の変更を検知)")
            else:
                # 内容が同じなら何もしない
                pass
        else:
            # 新規登録
            new_prob = ProblemModel(
                filename=filename,
                sgf_content=new_content,
                elo=1500
            )
            db.add(new_prob)
            added_count += 1
            print(f"新規登録: {filename}")
        
    db.commit()
    db.close()
    print(f"\n完了! 新規: {added_count} 件 / 更新: {updated_count} 件")

if __name__ == "__main__":
    import_sgfs()