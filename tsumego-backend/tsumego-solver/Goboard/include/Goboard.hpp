#pragma once
#include "Types.hpp"
#include "StoneString.hpp"
#include <vector>
#include <unordered_set>

namespace goboard {

class Goboard {
public:
    // コンストラクタ
    explicit Goboard(int size = 19, bool enable_superko = false);
    Goboard(int width, int height, bool enable_superko = false);

    // ルール判定
    // x, y に PASS_COORD が入ることは想定していないため、事前に呼び出し側で弾く前提です。
    bool is_playable(int x, int y) const;
    bool is_playable(int x, int y, Color turn) const; // 手番を指定（コウ判定無視）
    
    // 詳細な着手判定（内部手番を使用し、コウ・スーパーコウを含むすべての理由を判定）
    MoveStatus check_move(int x, int y) const;

    // 着手とゲーム進行 (x == PASS_COORD && y == PASS_COORD の場合はパスとして処理)
    bool move(int x, int y);
    bool move(int x, int y, Color turn); // 手番を指定

    // 状態取得
    Color get_turn() const { return turn_; }
    Color get_color_at(int x, int y) const;
    int get_width() const { return width_; }
    int get_height() const { return height_; }
    bool is_game_over() const { return consecutive_passes_ >= 2; }

private:
    int width_;
    int height_;
    Color turn_;
    int consecutive_passes_;
    bool enable_superko_;

    // 盤面データ: 1次元配列。各交点に存在する StoneString へのポインタ。空点は nullptr
    std::vector<StoneString*> board_;
    
    // ハッシュ計算や単純な色確認用の配列
    std::vector<Color> color_board_;

    // メモリアロケーション最適化のためのオブジェクトプール
    std::vector<StoneString> string_pool_;
    std::vector<StoneString*> free_strings_;

    // ハッシュと履歴（コウ・スーパーコウ判定用）
    HashValue current_hash_;
    HashValue last_hash_; // 2手前（1順前）のハッシュ値（通常のコウ用）
    std::unordered_set<HashValue> hash_history_; // 過去の全ハッシュ値（スーパーコウ用）

    // 初期化ヘルパー
    void initialize_board();

    // ユーティリティ
    Point xy_to_point(int x, int y) const;
    bool is_on_board(Point p) const;
    std::vector<Point> get_adjacent_points(Point p) const;

    // 石のグループ管理
    StoneString* allocate_string();
    void free_string(StoneString* str);
    void remove_string(StoneString* target);

    // ハッシュ更新
    void toggle_hash(Point p, Color color);
};

} // namespace goboard