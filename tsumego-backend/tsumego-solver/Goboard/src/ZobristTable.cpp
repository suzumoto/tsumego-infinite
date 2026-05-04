#include "../include/ZobristTable.hpp"
#include <random>

namespace goboard {

std::vector<std::vector<HashValue>> ZobristTable::table_;
bool ZobristTable::is_initialized_ = false;

void ZobristTable::init(int max_points) {
    if (is_initialized_) return;

    // 決定論的な乱数生成（検証・テストの再現性のためシードを固定）
    std::mt19937_64 rng(1337); 
    std::uniform_int_distribution<HashValue> dist;

    // table_[座標][色 (0:EMPTY, 1:BLACK, 2:WHITE)]
    table_.resize(max_points, std::vector<HashValue>(3, 0));
    for (int p = 0; p < max_points; ++p) {
        table_[p][static_cast<int>(Color::BLACK)] = dist(rng);
        table_[p][static_cast<int>(Color::WHITE)] = dist(rng);
    }

    is_initialized_ = true;
}

HashValue ZobristTable::get_hash(Point p, Color color) {
    if (color == Color::EMPTY) return 0;
    return table_[p][static_cast<int>(color)];
}

} // namespace goboard