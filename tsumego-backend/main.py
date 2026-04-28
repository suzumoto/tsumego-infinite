import os
import random
from fastapi import FastAPI, HTTPException
from fastapi.middleware.cors import CORSMiddleware
from pydantic import BaseModel
from sgfmill import sgf
from sqlalchemy import create_engine, Column, Integer, String, Text, func
from sqlalchemy.ext.declarative import declarative_base
from sqlalchemy.orm import sessionmaker
from dotenv import load_dotenv

load_dotenv()

app = FastAPI(title="Tsumego Infinite API")

# 環境変数からデータベースURLを取得（ローカルテスト用にはデフォルト値を設定）
DATABASE_URL = os.getenv("DATABASE_URL", "postgresql://postgres:password@localhost:5432/postgres")
engine = create_engine(DATABASE_URL)
SessionLocal = sessionmaker(autocommit=False, autoflush=False, bind=engine)
Base = declarative_base()

# --- DBモデル定義 ---
class ProblemModel(Base):
    __tablename__ = "problems"
    id = Column(Integer, primary_key=True, index=True)
    filename = Column(String, unique=True, index=True)
    sgf_content = Column(Text)
    elo = Column(Integer, default=1500)
    success_count = Column(Integer, default=0)
    fail_count = Column(Integer, default=0)

app.add_middleware(
    CORSMiddleware,
    allow_origins=["*"], # デプロイ後はVercelのURLに制限することを推奨
    allow_credentials=True,
    allow_methods=["*"],
    allow_headers=["*"],
)

# --- 座標変換・SGFパースロジック (変更なし) ---
def transform_coord(x: int, y: int, sym_mode: int, board_size: int = 19) -> tuple[int, int]:
    max_idx = board_size - 1
    if sym_mode == 0: return x, y
    elif sym_mode == 1: return max_idx - x, y
    elif sym_mode == 2: return x, max_idx - y
    elif sym_mode == 3: return max_idx - x, max_idx - y
    elif sym_mode == 4: return y, x
    elif sym_mode == 5: return max_idx - y, x
    elif sym_mode == 6: return y, max_idx - x
    elif sym_mode == 7: return max_idx - y, max_idx - x
    return x, y

def build_tree(node, sym_mode: int, board_size: int = 19):
    tree_node = {"children": []}
    color, move = node.get_move()
    if color and move is not None:
        row, col = move
        x = col
        y = (board_size - 1) - row
        tx, ty = transform_coord(x, y, sym_mode, board_size)
        tree_node["x"], tree_node["y"], tree_node["color"] = tx, ty, color.upper()
        if node.has_property('TE'): tree_node["is_correct"] = True
        elif node.has_property('BM'): tree_node["is_correct"] = False
    
    if node.has_property('C'): tree_node["comment"] = node.get('C')
    if node.has_property('LB'):
        tree_node["labels"] = [{"x": tx, "y": ty, "text": text} for (tx, ty), text in 
                              [(transform_coord(col, (board_size - 1) - row, sym_mode, board_size), text) 
                               for (row, col), text in node.get('LB')]]

    for child in node:
        child_tree = build_tree(child, sym_mode, board_size)
        if "x" not in child_tree:
            if "comment" in child_tree: tree_node["comment"] = child_tree["comment"]
            if "labels" in child_tree: tree_node["labels"] = child_tree["labels"]
            tree_node["children"].extend(child_tree["children"])
        else:
            tree_node["children"].append(child_tree)
    if "x" in tree_node and "is_correct" not in tree_node:
        tree_node["is_correct"] = len(tree_node["children"]) == 0 or None
    return tree_node

# --- APIエンドポイント ---
@app.get("/api/problem/next")
def get_next_problem(user_elo: int = 1450):
    db = SessionLocal()
    try:
        # ユーザーのEloに近い問題をDBから5件取得
        problems = db.query(ProblemModel).order_by(
            func.abs(ProblemModel.elo - user_elo)
        ).limit(5).all()

        if not problems:
            raise HTTPException(status_code=404, detail="問題が見つかりません。DBを確認してください。")

        target = random.choice(problems)
        sym_mode = random.randint(0, 7)
        game = sgf.Sgf_game.from_string(target.sgf_content)
        root, board_size = game.get_root(), game.get_size()
        
        initial_stones = []
        for prop, color in [('AB', 'B'), ('AW', 'W')]:
            if root.has_property(prop):
                for row, col in root.get(prop):
                    tx, ty = transform_coord(col, (board_size - 1) - row, sym_mode, board_size)
                    initial_stones.append({"x": tx, "y": ty, "color": color})
                    
        return {
            "problem_id": str(target.id),
            "difficulty": target.elo,
            "initialStones": initial_stones,
            "startTurn": "B",
            "tree": build_tree(root, sym_mode, board_size),
            "sym_mode_applied": sym_mode
        }
    finally:
        db.close()

class JudgmentResult(BaseModel):
    problem_id: str
    user_elo: int
    problem_elo: int
    is_correct: bool

@app.post("/api/rating/update")
def update_rating(result: JudgmentResult):
    db = SessionLocal()
    try:
        problem = db.query(ProblemModel).filter(ProblemModel.id == int(result.problem_id)).first()
        if not problem:
            raise HTTPException(status_code=404, detail="Problem not found")
        
        # Elo計算
        E_u = 1 / (1 + 10 ** ((result.problem_elo - result.user_elo) / 400))
        S = 1 if result.is_correct else 0
        new_user_elo = int(result.user_elo + 32 * (S - E_u))
        new_prob_elo = int(result.problem_elo + 16 * (E_u - S))
        
        # DB更新
        problem.elo = new_prob_elo
        if result.is_correct:
            problem.success_count += 1
        else:
            problem.fail_count += 1
        db.commit()
        
        return {"new_user_elo": new_user_elo, "new_prob_elo": new_prob_elo}
    finally:
        db.close()