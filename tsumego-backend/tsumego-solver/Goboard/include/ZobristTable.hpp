#pragma once
#include "Types.hpp"
#include <vector>

namespace goboard {

class ZobristTable {
public:
    // 盤面の最大交点数に合わせてテーブルを初期化する
    // プログラム起動時、あるいは最初のGoboard生成時に1度だけ呼ぶ
    static void init(int max_points);

    // 指定した座標・色に対応するハッシュ値（XOR用のビット列）を取得
    static HashValue get_hash(Point p, Color color);

private:
    // [Point][Color (1:BLACK, 2:WHITE)] の乱数テーブル
    static std::vector<std::vector<HashValue>> table_;
    static bool is_initialized_;
};

} // namespace goboard