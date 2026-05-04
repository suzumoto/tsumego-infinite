#include "../include/Goboard.hpp"
#include "../include/ZobristTable.hpp"
#include <algorithm>
#include <iostream>

namespace goboard {

Goboard::Goboard(int size, bool enable_superko)
    : Goboard(size, size, enable_superko) {}

Goboard::Goboard(int width, int height, bool enable_superko)
    : width_(width), height_(height), enable_superko_(enable_superko) {
    initialize_board();
}

void Goboard::initialize_board() {
    int max_points = width_ * height_;
    ZobristTable::init(max_points);

    board_.assign(max_points, nullptr);
    color_board_.assign(max_points, Color::EMPTY);
    
    turn_ = Color::BLACK;
    consecutive_passes_ = 0;
    current_hash_ = 0;
    last_hash_ = 0;
    hash_history_.clear();
    hash_history_.insert(current_hash_);

    // オブジェクトプールの初期化
    string_pool_.resize(max_points);
    free_strings_.reserve(max_points);
    for (int i = max_points - 1; i >= 0; --i) {
        free_strings_.push_back(&string_pool_[i]);
    }
}

Point Goboard::xy_to_point(int x, int y) const {
    return y * width_ + x;
}

bool Goboard::is_on_board(Point p) const {
    return p >= 0 && p < width_ * height_;
}

std::vector<Point> Goboard::get_adjacent_points(Point p) const {
    std::vector<Point> adj;
    int x = p % width_;
    int y = p / width_;
    if (y > 0) adj.push_back(p - width_);
    if (y < height_ - 1) adj.push_back(p + width_);
    if (x > 0) adj.push_back(p - 1);
    if (x < width_ - 1) adj.push_back(p + 1);
    return adj;
}

StoneString* Goboard::allocate_string() {
    if (free_strings_.empty()) return nullptr;
    StoneString* str = free_strings_.back();
    free_strings_.pop_back();
    return str;
}

void Goboard::free_string(StoneString* str) {
    str->clear();
    free_strings_.push_back(str);
}

void Goboard::remove_string(StoneString* target) {
    Color target_color = target->get_color();
    Color opponent_color = get_opponent(target_color);

    // 1. 各 neighbor (隣接していた敵) から target への参照を消す
    for (StoneString* neighbor : target->get_neighbors()) {
        neighbor->remove_neighbor(target);
    }

    // 2. 盤面から石を消去し、ハッシュを更新する
    for (Point p : target->get_stones()) {
        board_[p] = nullptr;
        color_board_[p] = Color::EMPTY;
        toggle_hash(p, target_color);
    }

    // 3. 消去した各座標 p について、周囲4方向を見る
    //    相手の石（元 neighbor）があれば、正確に p をダメとして追加する
    for (Point p : target->get_stones()) {
        for (Point adj : get_adjacent_points(p)) {
            StoneString* adj_str = board_[adj];
            if (adj_str != nullptr && adj_str->get_color() == opponent_color) {
                adj_str->add_liberty(p);
            }
        }
    }

    free_string(target);
}

void Goboard::toggle_hash(Point p, Color color) {
    current_hash_ ^= ZobristTable::get_hash(p, color);
}

Color Goboard::get_color_at(int x, int y) const {
    if (x < 0 || x >= width_ || y < 0 || y >= height_) return Color::EMPTY;
    return color_board_[xy_to_point(x, y)];
}

// ==========================================
// ルール判定と着手処理
// ==========================================

MoveStatus Goboard::check_move(int x, int y) const {
    if (x < 0 || x >= width_ || y < 0 || y >= height_) return MoveStatus::ILLEGAL_OUT_OF_BOUNDS;
    Point p = xy_to_point(x, y);
    if (board_[p] != nullptr) return MoveStatus::ILLEGAL_OCCUPIED;

    bool captures = false;
    bool connects_to_safe_friendly = false;
    bool has_empty_adj = false;
    std::vector<StoneString*> captured_enemies;
    
    for (Point adj : get_adjacent_points(p)) {
        StoneString* adj_str = board_[adj];
        if (adj_str == nullptr) {
            has_empty_adj = true;
        } else if (adj_str->get_color() == turn_) {
            if (adj_str->get_liberty_count() > 1) connects_to_safe_friendly = true;
        } else {
            if (adj_str->get_liberty_count() == 1) {
                captures = true;
                if (std::find(captured_enemies.begin(), captured_enemies.end(), adj_str) == captured_enemies.end()) {
                    captured_enemies.push_back(adj_str);
                }
            }
        }
    }

    // 自殺手判定
    if (!has_empty_adj && !connects_to_safe_friendly && !captures) {
        return MoveStatus::ILLEGAL_SUICIDE;
    }

    // ハッシュ計算 (コウ判定用)
    HashValue next_hash = current_hash_;
    next_hash ^= ZobristTable::get_hash(p, turn_);
    for (StoneString* enemy_str : captured_enemies) {
        for (Point cp : enemy_str->get_stones()) {
            next_hash ^= ZobristTable::get_hash(cp, enemy_str->get_color());
        }
    }

    // 単純コウ判定
    if (next_hash == last_hash_) {
        return MoveStatus::ILLEGAL_SIMPLE_KO;
    }

    // スーパーコウ判定
    if (enable_superko_) {
        if (hash_history_.find(next_hash) != hash_history_.end()) {
            return MoveStatus::ILLEGAL_SUPERKO;
        }
    }

    return MoveStatus::VALID;
}

// 既存の is_playable は、check_move の結果を bool に変換するだけにする
bool Goboard::is_playable(int x, int y) const {
    return check_move(x, y) == MoveStatus::VALID;
}

// ※ bool Goboard::is_playable(int x, int y, Color turn) const は
// これまで通り（コウを無視する盤面編集用として）そのまま残しておきます。

bool Goboard::is_playable(int x, int y, Color turn) const {
    // 外部からの手番指定（盤面編集用）。コウ判定をスキップします。
    if (x < 0 || x >= width_ || y < 0 || y >= height_) return false;
    Point p = xy_to_point(x, y);
    if (board_[p] != nullptr) return false;

    bool captures = false;
    bool connects_to_safe_friendly = false;
    bool has_empty_adj = false;
    
    for (Point adj : get_adjacent_points(p)) {
        StoneString* adj_str = board_[adj];
        if (adj_str == nullptr) {
            has_empty_adj = true;
        } else if (adj_str->get_color() == turn) {
            if (adj_str->get_liberty_count() > 1) connects_to_safe_friendly = true;
        } else {
            if (adj_str->get_liberty_count() == 1) captures = true;
        }
    }

    if (!has_empty_adj && !connects_to_safe_friendly && !captures) return false;
    return true;
}

bool Goboard::move(int x, int y) {
    if (x == PASS_COORD && y == PASS_COORD) {
        consecutive_passes_++;
        last_hash_ = current_hash_;
        hash_history_.insert(current_hash_);
        turn_ = get_opponent(turn_);
        return true;
    }

    if (!is_playable(x, y)) return false;

    Point p = xy_to_point(x, y);
    HashValue prev_hash = current_hash_;

    StoneString* new_str = allocate_string();
    new_str->init(turn_, p);
    
    // 空点をダメとして追加
    for (Point adj : get_adjacent_points(p)) {
        if (board_[adj] == nullptr) new_str->add_liberty(adj);
    }

    board_[p] = new_str;
    color_board_[p] = turn_;
    toggle_hash(p, turn_);

    std::unordered_set<StoneString*> friendly_strings;
    std::unordered_set<StoneString*> enemy_strings;

    for (Point adj : get_adjacent_points(p)) {
        StoneString* adj_str = board_[adj];
        if (adj_str) {
            if (adj_str->get_color() == turn_) friendly_strings.insert(adj_str);
            else enemy_strings.insert(adj_str);
        }
    }

    // 敵のダメを減らす
    for (StoneString* enemy : enemy_strings) {
        enemy->remove_liberty(p);
        enemy->add_neighbor(new_str);
        new_str->add_neighbor(enemy);
    }

    // 取られた敵石の除去（味方の結合前に実行し、ダメ回復を反映させる）
    for (StoneString* enemy : enemy_strings) {
        if (enemy->get_liberty_count() == 0) {
            remove_string(enemy);
        }
    }

    // 味方の連との結合
    for (StoneString* friendly : friendly_strings) {
        new_str->merge_with(friendly);
        free_string(friendly);
    }
    
    // 味方の連が持っていたダメ「p」を削除
    new_str->remove_liberty(p);

    // 盤面のポインタを結合後の新しい連に書き換える
    for (Point sp : new_str->get_stones()) {
        board_[sp] = new_str;
    }

    // 状態更新
    last_hash_ = prev_hash;
    hash_history_.insert(current_hash_);
    consecutive_passes_ = 0;
    turn_ = get_opponent(turn_);

    return true;
}

bool Goboard::move(int x, int y, Color turn) {
    if (x == PASS_COORD && y == PASS_COORD) return true;
    if (!is_playable(x, y, turn)) return false;

    // 盤面編集用の着手処理。ゲームの内部手番(turn_)やパス回数は進めませんが、
    // 盤面の状態・ハッシュは更新します。（ロジック自体は上記 move と同一です）
    Point p = xy_to_point(x, y);
    HashValue prev_hash = current_hash_;

    StoneString* new_str = allocate_string();
    new_str->init(turn, p);
    
    for (Point adj : get_adjacent_points(p)) {
        if (board_[adj] == nullptr) new_str->add_liberty(adj);
    }

    board_[p] = new_str;
    color_board_[p] = turn;
    toggle_hash(p, turn);

    std::unordered_set<StoneString*> friendly_strings;
    std::unordered_set<StoneString*> enemy_strings;

    for (Point adj : get_adjacent_points(p)) {
        StoneString* adj_str = board_[adj];
        if (adj_str) {
            if (adj_str->get_color() == turn) friendly_strings.insert(adj_str);
            else enemy_strings.insert(adj_str);
        }
    }

    for (StoneString* enemy : enemy_strings) {
        enemy->remove_liberty(p);
        enemy->add_neighbor(new_str);
        new_str->add_neighbor(enemy);
    }

    for (StoneString* enemy : enemy_strings) {
        if (enemy->get_liberty_count() == 0) {
            remove_string(enemy);
        }
    }

    for (StoneString* friendly : friendly_strings) {
        new_str->merge_with(friendly);
        free_string(friendly);
    }
    
    new_str->remove_liberty(p);

    for (Point sp : new_str->get_stones()) {
        board_[sp] = new_str;
    }

    last_hash_ = prev_hash;
    hash_history_.insert(current_hash_);

    return true;
}

} // namespace goboard