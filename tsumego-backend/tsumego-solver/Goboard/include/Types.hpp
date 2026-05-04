#pragma once
#include <cstdint>

namespace goboard {

// 石の色と空点
enum class Color {
    EMPTY = 0,
    BLACK = 1,
    WHITE = 2
};

// 相手の色を取得するヘルパー関数
inline Color get_opponent(Color color) {
    if (color == Color::BLACK) return Color::WHITE;
    if (color == Color::WHITE) return Color::BLACK;
    return Color::EMPTY;
}

// 1次元化された座標の型
using Point = int;

// ゾブリストハッシュ値の型
using HashValue = std::uint64_t;

// パスを表す特殊な座標定数
constexpr Point PASS_COORD = -1;

// 着手の判定結果ステータス
enum class MoveStatus {
    VALID,                  // 着手可能
    ILLEGAL_OCCUPIED,       // 既に石がある
    ILLEGAL_OUT_OF_BOUNDS,  // 盤外
    ILLEGAL_SUICIDE,        // 自殺手
    ILLEGAL_SIMPLE_KO,      // 単純コウ
    ILLEGAL_SUPERKO         // スーパーコウ
};

} // namespace goboard