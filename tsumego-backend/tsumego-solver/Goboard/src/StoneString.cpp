#include "../include/StoneString.hpp"

namespace goboard {

void StoneString::init(Color color, Point p) {
    color_ = color;
    stones_.clear();
    stones_.push_back(p);
    liberties_.clear();
    neighbor_strings_.clear();
    is_active_ = true;
}

void StoneString::clear() {
    stones_.clear();
    liberties_.clear();
    neighbor_strings_.clear();
    is_active_ = false;
}

void StoneString::add_liberty(Point p) {
    liberties_.insert(p);
}

void StoneString::remove_liberty(Point p) {
    liberties_.erase(p);
}

void StoneString::add_neighbor(StoneString* neighbor) {
    neighbor_strings_.insert(neighbor);
}

void StoneString::remove_neighbor(StoneString* neighbor) {
    neighbor_strings_.erase(neighbor);
}

void StoneString::merge_with(StoneString* other) {
    if (this == other) return;

    // 石のリストとダメの集合を結合
    stones_.insert(stones_.end(), other->stones_.begin(), other->stones_.end());
    liberties_.insert(other->liberties_.begin(), other->liberties_.end());

    // 結合対象(other)に隣接していた敵の連に対して、自身の参照をつなぎ直す
    for (StoneString* enemy : other->neighbor_strings_) {
        enemy->remove_neighbor(other);
        enemy->add_neighbor(this);
        neighbor_strings_.insert(enemy);
    }
    
    // other の無効化は Goboard 側 (free_string) でも行われますが、念のためクリア
    other->clear();
}

} // namespace goboard