#pragma once
#include "Types.hpp"
#include <vector>
#include <unordered_set>

namespace goboard {

class StoneString {
public:
    StoneString() : color_(Color::EMPTY), is_active_(false) {}

    // 初期化と破棄（オブジェクトプール用）
    void init(Color color, Point p);
    void clear();

    // 状態取得
    Color get_color() const { return color_; }
    int get_liberty_count() const { return liberties_.size(); }
    const std::vector<Point>& get_stones() const { return stones_; }
    bool is_active() const { return is_active_; }
    
    // 自身に隣接する敵の連のリスト（盤面から石が取り除かれる際に使用）
    const std::unordered_set<StoneString*>& get_neighbors() const { return neighbor_strings_; }

    // 状態更新
    void add_liberty(Point p);
    void remove_liberty(Point p);
    void add_neighbor(StoneString* neighbor);
    void remove_neighbor(StoneString* neighbor);
    
    // 別の同色の連と結合する
    void merge_with(StoneString* other);

private:
    Color color_;
    std::vector<Point> stones_;                    // この連に属する石の座標
    std::unordered_set<Point> liberties_;          // この連が持つダメ（空点）の座標集合
    std::unordered_set<StoneString*> neighbor_strings_; // 隣接する敵の連へのポインタ集合
    bool is_active_;                               // プール内で現在使用中かどうか
};

} // namespace goboard