// test/GoboardTest.cpp

#include <gtest/gtest.h>
#include <string>
#include <vector>
#include <unordered_set>

// 内部状態（StoneStringや盤面配列）をテストで検証するため、
// access modifier を public に書き換えて include するハックを使用します。
#define private public
#define protected public
#include "../include/Goboard.hpp"
#include "../include/StoneString.hpp"
#undef private
#undef protected

namespace goboard {
namespace test {

class GoboardTest : public ::testing::Test {
protected:
    Goboard board;

    GoboardTest() : board(19, false) {}

    StoneString* get_string_at(int x, int y) {
        return board.board_[board.xy_to_point(x, y)];
    }

    bool has_liberty(StoneString* str, int x, int y) {
        return str->liberties_.find(board.xy_to_point(x, y)) != str->liberties_.end();
    }

};

// =====================================================================
// 1. 単純正常テスト
// =====================================================================

TEST_F(GoboardTest, SimpleNormalTest_1_1) {
    // 1-1: 初期盤面: 空, 黒番
    EXPECT_EQ(board.get_turn(), Color::BLACK);
    
    bool ok = board.move(14, 3);
    EXPECT_TRUE(ok);
    EXPECT_EQ(board.get_turn(), Color::WHITE); // 手番が白になっていること

    StoneString* str = get_string_at(14, 3);
    ASSERT_NE(str, nullptr);
    EXPECT_EQ(str->get_color(), Color::BLACK);
    EXPECT_EQ(str->get_stones().size(), 1);
    EXPECT_EQ(str->get_stones()[0], board.xy_to_point(14, 3));
    
    // ダメの確認
    EXPECT_EQ(str->get_liberty_count(), 4);
    EXPECT_TRUE(has_liberty(str, 13, 3));
    EXPECT_TRUE(has_liberty(str, 15, 3));
    EXPECT_TRUE(has_liberty(str, 14, 2));
    EXPECT_TRUE(has_liberty(str, 14, 4));
}

TEST_F(GoboardTest, SimpleNormalTest_1_2) {
    board.move(14, 3); // 1-1
    
    // 1-2: 1-1のテスト後の盤面から、move(14, 4) を実行 (白番)
    bool ok = board.move(14, 4);
    EXPECT_TRUE(ok);

    StoneString* b_str = get_string_at(14, 3);
    EXPECT_EQ(b_str->get_liberty_count(), 3);
    EXPECT_TRUE(has_liberty(b_str, 13, 3));
    EXPECT_TRUE(has_liberty(b_str, 15, 3));
    EXPECT_TRUE(has_liberty(b_str, 14, 2));

    StoneString* w_str = get_string_at(14, 4);
    ASSERT_NE(w_str, nullptr);
    EXPECT_EQ(w_str->get_color(), Color::WHITE);
    EXPECT_EQ(w_str->get_stones().size(), 1);
    EXPECT_EQ(w_str->get_stones()[0], board.xy_to_point(14, 4));
    
    EXPECT_EQ(w_str->get_liberty_count(), 3);
    EXPECT_TRUE(has_liberty(w_str, 13, 4));
    EXPECT_TRUE(has_liberty(w_str, 15, 4));
    EXPECT_TRUE(has_liberty(w_str, 14, 5));
}

TEST_F(GoboardTest, SimpleNormalTest_1_3) {
    board.move(14, 3);
    board.move(14, 4);
    
    // 1-3: 黒と白が交互に着手
    board.move(13, 2); // Black
    board.move(13, 5); // White
    board.move(14, 1); // Black
    board.move(14, 6); // White
    board.move(15, 2); // Black
    board.move(15, 5); // White

    std::vector<std::pair<int, int>> b_coords = {{14,3}, {13,2}, {14,1}, {15,2}};
    std::vector<std::pair<int, int>> w_coords = {{14,4}, {13,5}, {14,6}, {15,5}};

    int b_lib4_count = 0, b_lib3_count = 0;
    for (auto& p : b_coords) {
        StoneString* str = get_string_at(p.first, p.second);
        ASSERT_NE(str, nullptr);
        if (str->get_liberty_count() == 4) b_lib4_count++;
        if (str->get_liberty_count() == 3) b_lib3_count++;
    }
    EXPECT_EQ(b_lib4_count, 3);
    EXPECT_EQ(b_lib3_count, 1); // (14,3) は (14,4) に白がいるため 3

    int w_lib4_count = 0, w_lib3_count = 0;
    for (auto& p : w_coords) {
        StoneString* str = get_string_at(p.first, p.second);
        ASSERT_NE(str, nullptr);
        if (str->get_liberty_count() == 4) w_lib4_count++;
        if (str->get_liberty_count() == 3) w_lib3_count++;
    }
    EXPECT_EQ(w_lib4_count, 3);
    EXPECT_EQ(w_lib3_count, 1); // (14,4) は (14,3) に黒がいるため 3
}

TEST_F(GoboardTest, SimpleNormalTest_1_4) {
    board.move(14, 3); board.move(14, 4);
    board.move(13, 2); board.move(13, 5);
    board.move(14, 1); board.move(14, 6);
    board.move(15, 2); board.move(15, 5);
    
    // 1-4: 黒(0,0), 白(1,0), 黒(3,0)
    board.move(0, 0);
    board.move(1, 0);
    board.move(3, 0);

    StoneString* b0 = get_string_at(0, 0);
    StoneString* b3 = get_string_at(3, 0);
    StoneString* w1 = get_string_at(1, 0);

    // プロンプトの「1,0 と...」は、(0,0) のダメ (0,1) の Typo と推定されるため、
    // 実際の正しいルールの挙動でアサートします。
    EXPECT_EQ(b0->get_liberty_count(), 1);
    EXPECT_TRUE(has_liberty(b0, 0, 1)); 

    EXPECT_EQ(b3->get_liberty_count(), 3);
    EXPECT_TRUE(has_liberty(b3, 2, 0));
    EXPECT_TRUE(has_liberty(b3, 4, 0));
    EXPECT_TRUE(has_liberty(b3, 3, 1));

    EXPECT_EQ(w1->get_liberty_count(), 2);
    EXPECT_TRUE(has_liberty(w1, 2, 0));
    EXPECT_TRUE(has_liberty(w1, 1, 1));
}

// =====================================================================
// 2. 石の連結テスト
// =====================================================================

TEST_F(GoboardTest, StoneConnectionTest_2_1) {
    // 1-4 までの状態を再現
    board.move(14, 3); board.move(14, 4); board.move(13, 2); board.move(13, 5);
    board.move(14, 1); board.move(14, 6); board.move(15, 2); board.move(15, 5);
    board.move(0, 0); board.move(1, 0); board.move(3, 0);

    // 2-1: move(14, 5) 白番
    board.move(14, 5);

    StoneString* w_cross = get_string_at(14, 5);
    EXPECT_EQ(w_cross->get_stones().size(), 5); // 4つが結合され1つ追加
    EXPECT_EQ(w_cross->get_liberty_count(), 7);
    
    // 指定されたダメの検証
    EXPECT_TRUE(has_liberty(w_cross, 13, 4));
    EXPECT_TRUE(has_liberty(w_cross, 12, 5));
    EXPECT_TRUE(has_liberty(w_cross, 13, 6));
    EXPECT_TRUE(has_liberty(w_cross, 14, 7));
    EXPECT_TRUE(has_liberty(w_cross, 15, 6));
    EXPECT_TRUE(has_liberty(w_cross, 16, 5));
    EXPECT_TRUE(has_liberty(w_cross, 15, 4));

    // 隣接StoneString (14,3) の確認
    bool found_neighbor = false;
    for (StoneString* neighbor : w_cross->get_neighbors()) {
        for (int p : neighbor->get_stones()) {
            if (p == board.xy_to_point(14, 3)) found_neighbor = true;
        }
    }
    EXPECT_TRUE(found_neighbor);
}

TEST_F(GoboardTest, StoneConnectionTest_2_2_and_2_3_and_2_4) {
    board.move(14, 3); board.move(14, 4); board.move(13, 2); board.move(13, 5);
    board.move(14, 1); board.move(14, 6); board.move(15, 2); board.move(15, 5);
    board.move(0, 0); board.move(1, 0); board.move(3, 0);
    board.move(14, 5); // W

    // 2-2: move(14, 2) 黒番
    board.move(14, 2);

    StoneString* b_cross = get_string_at(14, 2);
    EXPECT_EQ(b_cross->get_stones().size(), 5);
    EXPECT_EQ(b_cross->get_liberty_count(), 7);

    EXPECT_TRUE(has_liberty(b_cross, 14, 0));
    EXPECT_TRUE(has_liberty(b_cross, 13, 1));
    EXPECT_TRUE(has_liberty(b_cross, 12, 2));
    EXPECT_TRUE(has_liberty(b_cross, 13, 3));
    EXPECT_TRUE(has_liberty(b_cross, 15, 3));
    EXPECT_TRUE(has_liberty(b_cross, 16, 2));
    EXPECT_TRUE(has_liberty(b_cross, 15, 1));

    bool found_w_cross = false;
    for (StoneString* neighbor : b_cross->get_neighbors()) {
        if (neighbor->get_stones().size() == 5) {
            found_w_cross = true; // (14,4)等を含む白十字
        }
    }
    EXPECT_TRUE(found_w_cross);

    // 2-3: (14,2) は黒が着手済みのため、ルール上 is_playable は false です。
    EXPECT_FALSE(board.is_playable(14, 2));
    EXPECT_EQ(board.check_move(14, 2), MoveStatus::ILLEGAL_OCCUPIED);
    EXPECT_FALSE(board.is_playable(14, 2, Color::WHITE));
    EXPECT_FALSE(board.move(14, 2, Color::WHITE)); // 変化なし
    
    // 2-4: 黒が白十字を包囲して取る
    board.move(13, 4, Color::BLACK);
    board.move(12, 5, Color::BLACK);
    board.move(13, 6, Color::BLACK);
    board.move(14, 7, Color::BLACK);
    board.move(15, 6, Color::BLACK);
    board.move(16, 5, Color::BLACK);
    board.move(15, 4, Color::BLACK);

    // 白十字(14,5等)が除去されているか確認
    EXPECT_EQ(get_string_at(14, 5), nullptr);
    EXPECT_EQ(get_string_at(14, 4), nullptr);

    // 盤面に残った石数の計算
    int total_stones = 0;
    for (int p = 0; p < 19 * 19; ++p) {
        if (board.color_board_[p] != Color::EMPTY) total_stones++;
    }
    EXPECT_EQ(total_stones, 14 + 1); // 黒14個 + 白1個(1,0) = 計15個

    // 黒石のStoneStringは(0,0) (3,0)にある石を除き、ダメの数がすべて4になっていること
    std::vector<std::pair<int, int>> new_b_stones = {
        {13,4}, {12,5}, {13,6}, {14,7}, {15,6}, {16,5}, {15,4}
    };
    for (auto& p : new_b_stones) {
        StoneString* str = get_string_at(p.first, p.second);
        ASSERT_NE(str, nullptr);
        EXPECT_EQ(str->get_liberty_count(), 4);
    }
}

// =====================================================================
// 3. コウのテスト
// =====================================================================

TEST_F(GoboardTest, KoTest_3_1) {
    // 盤面セットアップ (直接 Color を指定して配置)
    board.move(3, 3, Color::BLACK);
    board.move(4, 2, Color::BLACK);
    board.move(5, 3, Color::BLACK);
    board.move(4, 4, Color::BLACK);
    
    board.move(3, 2, Color::WHITE);
    board.move(2, 3, Color::WHITE);
    board.move(3, 4, Color::WHITE);

    // 白番にする
    board.turn_ = Color::WHITE;
    
    // W がコウを取る
    EXPECT_TRUE(board.move(4, 3)); 
    EXPECT_EQ(get_string_at(3, 3), nullptr);

    // B は直後に取り返せない (単純コウのルール)
    EXPECT_FALSE(board.is_playable(3, 3));
    EXPECT_EQ(board.check_move(3, 3), MoveStatus::ILLEGAL_SIMPLE_KO); // コウ判定を確認

    // B 別の場所に打つ (コウダテ)
    board.move(16, 16); 
    board.move(17, 16);

    // コウが解消され、取り返せるようになっている
    EXPECT_TRUE(board.is_playable(3, 3));
    EXPECT_EQ(board.check_move(3, 3), MoveStatus::VALID);
}

// =====================================================================
// 4. 実戦テスト
// =====================================================================

TEST_F(GoboardTest, RealGameTest_4) {
    std::string sgf_moves = 
        ";B[pp];W[dd];B[pc];W[dp];B[fc];W[cf];B[db];W[cc];B[ic];W[pe];B[qh];W[qc];B[qb];W[qd];B[ob];W[ne];B[mc];W[qk];B[nh];W[ld];B[md];W[me];B[le];W[kd];B[lf];W[mg];B[of];W[ng];B[og];W[oe];B[ok];W[qn];B[jf];W[mh];B[ni];W[id];B[hd];W[ie];B[kh];W[mi];B[mj];W[ki];B[lj];W[li];B[jh];W[kj];B[hg];W[he];B[gd];W[gf];B[gg];W[ff];B[gj];W[kl];B[jc];W[ke];B[qm];W[pm];B[ql];W[pl];B[rk];W[rj];B[qj];W[pk];B[rl];W[qi];B[pj];W[oj];B[pi];W[oi];B[ph];W[nj];B[oh];W[nk];B[pn];W[nm];B[on];W[dj];B[ml];W[nl];B[dh];W[ch];B[df];W[dg];B[eg];W[cg];B[ef];W[ee];B[fe];W[kf];B[lg];W[kg];B[lh];W[jg];B[ih];W[lb];B[mb];W[kb];B[jb];W[rb];B[ra];W[sb];B[if];W[ig];B[hf];W[ge];B[fd];W[hh];B[hi];W[gh];B[fg];W[fi];B[jd];W[ii];B[lc];W[kc];B[ka];W[ji];B[la];W[mf];B[je];W[fq];B[ce];W[be];B[de];W[cd];B[ci];W[di];B[eh];W[bi];B[cm];W[co];B[em];W[qq];B[pq];W[rp];B[qr];W[rr];B[qp];W[rq];B[ps];W[rn];B[qo];W[jp];B[km];W[lm];B[ln];W[mn];B[lo];W[mo];B[lp];W[mp];B[ll];W[lk];B[jl];W[kk];B[jn];W[im];B[in];W[hm];B[hn];W[gm];B[ep];W[eq];B[fo];W[hk];B[hp];W[hq];B[iq];W[ip];B[jq];W[kq];B[hr];W[gq];B[kp];W[ho];B[gn];W[go];B[kr];W[mq];B[lq];W[or];B[pr];W[ro];B[rs];W[rm];B[ri];W[oo];B[po];W[op];B[nr];W[oq];B[os];W[mr];B[ms];W[ls];B[lr];W[ns];B[om];W[ol];B[ms];W[sr];B[sm];W[ns]";

    int move_count = 0;
    int b_move_count = 0;
    int w_move_count = 0;
    
    size_t pos = 0;
    while ((pos = sgf_moves.find(";", pos)) != std::string::npos) {
        if (pos + 4 < sgf_moves.size() && (sgf_moves[pos+1] == 'B' || sgf_moves[pos+1] == 'W') && sgf_moves[pos+2] == '[') {
            std::string coord = sgf_moves.substr(pos+3, 2);
            int x = coord[0] - 'a';
            int y = coord[1] - 'a';
            
            if (sgf_moves[pos+1] == 'B') b_move_count++;
            else w_move_count++;

            // 1. 198手目完了後 (黒が199手目 [om] を打つ直前) の判定
            if (move_count == 198) {
                // (12, 18) はコウだが取り番ではない
                EXPECT_FALSE(board.is_playable(12, 18));
                EXPECT_EQ(board.check_move(12, 18), MoveStatus::ILLEGAL_SIMPLE_KO);
            }

            board.move(x, y);
            move_count++;
        }
        pos++;
    }

    // 2. 最終手の直後の局面で (12, 18) が is_playable ではないこと
    EXPECT_FALSE(board.is_playable(12, 18));

    // 3. アゲハマの計算と確認
    int b_stones = 0;
    int w_stones = 0;
    for (int p = 0; p < 19 * 19; ++p) {
        if (board.color_board_[p] == Color::BLACK) b_stones++;
        else if (board.color_board_[p] == Color::WHITE) w_stones++;
    }

    int black_captures = w_move_count - w_stones; // 黒石が取った白石
    int white_captures = b_move_count - b_stones; // 白石が取った黒石

    EXPECT_EQ(black_captures, 9);
    EXPECT_EQ(white_captures, 10);

    // AB / AW 盤面配置の完全一致確認
    std::string ab_str = "pp,pc,fc,db,ic,qh,qb,ob,mc,nh,md,of,og,jf,ni,hd,mj,lj,hg,gd,gg,gj,jc,qm,ql,rk,qj,rl,pj,pi,ph,oh,pn,on,ml,dh,df,eg,ef,fe,mb,jb,ra,if,hf,fd,hi,fg,jd,lc,ka,la,je,ce,de,ci,eh,cm,em,pq,qr,qp,ps,qo,km,ln,lo,lp,ll,jl,jn,in,hn,ep,fo,hp,iq,jq,hr,kp,gn,kr,lq,pr,rs,ri,po,nr,os,lr,om,sm";
    std::string aw_str = "dd,dp,cf,cc,pe,qc,qd,ne,qk,ld,me,kd,mg,ng,oe,qn,mh,mi,ki,li,kj,kl,ke,pm,pl,rj,pk,oj,oi,nj,nk,nm,dj,nl,ch,dg,cg,ee,kf,kg,jg,lb,kb,rb,sb,ig,hh,gh,fi,ii,kc,ji,mf,fq,be,cd,di,bi,co,qq,rp,rr,rq,rn,jp,mn,mo,mp,lk,kk,im,hm,gm,eq,hk,hq,ip,gq,ho,go,mq,or,ro,rm,oo,op,oq,mr,ls,ns,ol,sr,ns,lm";

    auto parse_coords = [](const std::string& str) {
        std::unordered_set<int> points;
        for (size_t i = 0; i < str.size(); i += 3) {
            int x = str[i] - 'a';
            int y = str[i+1] - 'a';
            points.insert(y * 19 + x);
        }
        return points;
    };

    auto ab_points = parse_coords(ab_str);
    auto aw_points = parse_coords(aw_str);

    for (int p = 0; p < 19 * 19; ++p) {
        if (ab_points.count(p)) {
            EXPECT_EQ(board.color_board_[p], Color::BLACK);
        } else if (aw_points.count(p)) {
            EXPECT_EQ(board.color_board_[p], Color::WHITE);
        } else {
            EXPECT_EQ(board.color_board_[p], Color::EMPTY);
        }
    }
}

// =====================================================================
// 5. 異常入力・境界値テスト (Abnormal Input & Edge Cases)
// =====================================================================

TEST_F(GoboardTest, OutOfBoundsTest) {
    Color initial_turn = board.get_turn();
    HashValue initial_hash = board.current_hash_;

    // 5-1: 負の座標 (PASS_COORD 以外の負数を含む)
    EXPECT_FALSE(board.is_playable(-1, 5));
    EXPECT_EQ(board.check_move(-1, 5), MoveStatus::ILLEGAL_OUT_OF_BOUNDS);
    EXPECT_FALSE(board.is_playable(5, -2));
    EXPECT_FALSE(board.move(-2, -2));

    // 5-2: 盤面サイズ以上の座標 (19x19盤の最大インデックスは18)
    EXPECT_FALSE(board.is_playable(19, 5));
    EXPECT_EQ(board.check_move(19, 5), MoveStatus::ILLEGAL_OUT_OF_BOUNDS);
    EXPECT_FALSE(board.is_playable(5, 19));
    EXPECT_FALSE(board.is_playable(100, 100));
    EXPECT_FALSE(board.move(19, 19));

    // 異常入力があった場合、手番やハッシュ値などの内部状態が一切変化していないこと
    EXPECT_EQ(board.get_turn(), initial_turn);
    EXPECT_EQ(board.current_hash_, initial_hash);
}

TEST_F(GoboardTest, PassCoordinateEdgeCaseTest) {
    // 5-3: is_playable にパスの座標 (-1, -1) が渡された場合
    // 仕様上は想定外だが、配列の範囲外アクセス等でクラッシュせず false を返すこと
    EXPECT_FALSE(board.is_playable(PASS_COORD, PASS_COORD));
    EXPECT_FALSE(board.is_playable(PASS_COORD, PASS_COORD, Color::BLACK));
}

TEST_F(GoboardTest, AlreadyOccupiedTest) {
    // 5-4: すでに石がある場所への着手
    EXPECT_TRUE(board.move(10, 10)); // 黒が着手
    
    Color turn_after_black = board.get_turn(); // 白番になっているはず
    HashValue hash_after_black = board.current_hash_;

    // 同じ場所に白が打とうとする
    EXPECT_FALSE(board.is_playable(10, 10));
    EXPECT_EQ(board.check_move(10, 10), MoveStatus::ILLEGAL_OCCUPIED);
    EXPECT_FALSE(board.move(10, 10));
    
    // 状態が変化せず、手番も白のままであること
    EXPECT_EQ(board.get_turn(), turn_after_black);
    EXPECT_EQ(board.current_hash_, hash_after_black);
    EXPECT_EQ(board.get_color_at(10, 10), Color::BLACK); // 石の色も変わっていない
}

TEST_F(GoboardTest, SuicideMoveTest) {
    // 5-5: 自殺手のテスト (ルール上の境界値)
    // 黒が盤面の隅 (0,0) を囲むように (1,0) と (0,1) に打つ
    board.move(1, 0, Color::BLACK);
    board.move(0, 1, Color::BLACK);

    // 白が (0,0) に打とうとするのは自殺手であり、着手禁止点
    EXPECT_FALSE(board.is_playable(0, 0, Color::WHITE));
    EXPECT_FALSE(board.move(0, 0, Color::WHITE));

    // 内部手番を白に設定してテスト
    board.turn_ = Color::WHITE;
    EXPECT_FALSE(board.is_playable(0, 0));
    EXPECT_EQ(board.check_move(0, 0), MoveStatus::ILLEGAL_SUICIDE);

    // 内部手番を黒に戻してテスト（合法手）
    board.turn_ = Color::BLACK;
    EXPECT_TRUE(board.is_playable(0, 0));
    EXPECT_EQ(board.check_move(0, 0), MoveStatus::VALID);
}

TEST_F(GoboardTest, SuicideMoveWithCaptureTest) {
    // 5-6: 自殺手に見えるが相手を取れる合法手、およびウッテガエシの検証
    // 同じ初期盤面から「黒が打った場合」と「白が打った場合」を分岐してテストするため、
    // 盤面セットアップ用のラムダ式を用意します。
    auto setup_board = [](Goboard& b) {
        // 黒石の配置
        b.move(0, 0, Color::BLACK);
        b.move(0, 3, Color::BLACK);
        b.move(2, 0, Color::BLACK);
        b.move(2, 1, Color::BLACK);
        b.move(1, 2, Color::BLACK);

        // 白石の配置
        b.move(1, 0, Color::WHITE);
        b.move(1, 1, Color::WHITE);
        b.move(0, 2, Color::WHITE);
    };

    // ----------------------------------------------------
    // パターン1: 黒が (0, 1) に着手するケース
    // ----------------------------------------------------
    {
        Goboard b1(19, false);
        setup_board(b1);

        // 黒は (0,1) に打つと自身のダメもゼロになるが、白石を取れるため合法手
        EXPECT_TRUE(b1.is_playable(0, 1, Color::BLACK));
        EXPECT_EQ(b1.check_move(0, 1), MoveStatus::VALID);
        EXPECT_TRUE(b1.move(0, 1, Color::BLACK));

        // 白石がすべて取り除かれていること
        EXPECT_EQ(b1.get_color_at(1, 0), Color::EMPTY);
        EXPECT_EQ(b1.get_color_at(1, 1), Color::EMPTY);
        EXPECT_EQ(b1.get_color_at(0, 2), Color::EMPTY);

        // 黒石が残っていること
        EXPECT_EQ(b1.get_color_at(0, 0), Color::BLACK);
        EXPECT_EQ(b1.get_color_at(0, 1), Color::BLACK);
    }

    // ----------------------------------------------------
    // パターン2: 白が (0, 1) に着手し、黒がウッテガエシで取るケース
    // ----------------------------------------------------
    {
        Goboard b2(19, false);
        setup_board(b2);

        // 白は (0,1) に打つと自身のダメもゼロになるが、黒石(0,0)を取れるため合法手
        EXPECT_TRUE(b2.is_playable(0, 1, Color::WHITE));
        EXPECT_TRUE(b2.move(0, 1, Color::WHITE));

        // 黒石(0,0)が取り除かれていること
        EXPECT_EQ(b2.get_color_at(0, 0), Color::EMPTY);

        // 打った白石と、既存の白石が残っていること
        EXPECT_EQ(b2.get_color_at(1, 0), Color::WHITE);
        EXPECT_EQ(b2.get_color_at(1, 1), Color::WHITE);
        EXPECT_EQ(b2.get_color_at(0, 2), Color::WHITE);
        EXPECT_EQ(b2.get_color_at(0, 1), Color::WHITE);

        // ★ ウッテガエシの検証 ★
        // この状態から、黒が空いた (0,0) に打つことは、ダメがゼロになるように見えるが、
        // 4つの白石のダメを完全に奪うため合法手となる。
        EXPECT_TRUE(b2.is_playable(0, 0, Color::BLACK));
        EXPECT_TRUE(b2.move(0, 0, Color::BLACK));

        // 盤面上の白石がすべて取り除かれていること
        EXPECT_EQ(b2.get_color_at(1, 0), Color::EMPTY);
        EXPECT_EQ(b2.get_color_at(1, 1), Color::EMPTY);
        EXPECT_EQ(b2.get_color_at(0, 2), Color::EMPTY);
        EXPECT_EQ(b2.get_color_at(0, 1), Color::EMPTY);

        // 新たに打った黒石が残っていること
        EXPECT_EQ(b2.get_color_at(0, 0), Color::BLACK);
    }
}

// =====================================================================
// 6. スーパーコウ (Superko) テスト
// =====================================================================

TEST_F(GoboardTest, SuperkoTest_6_1_TripleKo) {
    Goboard board_simple(19, false);
    Goboard board_super(19, true);

    auto setup_board = [](Goboard& b) {
        std::string aw = "[sb][qb][ra][ob][pa][na][mb][rc][nc]";
        std::string ab = "[mc][oc][nd][pd][qc][rd][pb][sc]";

        auto put_stones = [&](const std::string& sgf, Color c) {
            for (size_t i = 0; i < sgf.size(); ++i) {
                if (sgf[i] == '[' && i + 2 < sgf.size()) {
                    int x = sgf[i+1] - 'a';
                    int y = sgf[i+2] - 'a';
                    b.move(x, y, c);
                }
            }
        };
        put_stones(aw, Color::WHITE);
        put_stones(ab, Color::BLACK);

        b.turn_ = Color::BLACK;

        // ★セットアップ汚染を防ぐため、履歴を初期局面のみにリセットする★
        b.hash_history_.clear();
        b.hash_history_.insert(b.current_hash_);
        b.last_hash_ = 0; 
    };

    setup_board(board_simple);
    setup_board(board_super);

    std::vector<std::pair<int, int>> moves = {
        {17, 1}, {15, 2}, {13, 1}, {17, 2}, {15, 1}
    };

    for (auto m : moves) {
        EXPECT_EQ(board_simple.check_move(m.first, m.second), MoveStatus::VALID);
        EXPECT_TRUE(board_simple.move(m.first, m.second));

        EXPECT_EQ(board_super.check_move(m.first, m.second), MoveStatus::VALID);
        EXPECT_TRUE(board_super.move(m.first, m.second));
    }

    EXPECT_EQ(board_simple.check_move(13, 2), MoveStatus::VALID);
    EXPECT_TRUE(board_simple.move(13, 2));

    EXPECT_EQ(board_super.check_move(13, 2), MoveStatus::ILLEGAL_SUPERKO);
    EXPECT_FALSE(board_super.move(13, 2));
}

TEST_F(GoboardTest, SuperkoTest_6_2_Chousei) {
    Goboard board_simple(19, false);
    Goboard board_super(19, true);

    auto setup_board = [&](Goboard& b) {
        std::string ab = "[qb][pb][oc][pd][nd][md][ld][kd][jd][ic][hd][hb][ha][ka][kb][lb][mb][oa]";
        std::string aw = "[pa][ob][nb][nc][mc][lc][kc][jc][jb][ib][ia][ma]";

        auto put_stones = [&](const std::string& sgf, Color c) {
            for (size_t i = 0; i < sgf.size(); ++i) {
                if (sgf[i] == '[' && i + 2 < sgf.size()) {
                    int x = sgf[i+1] - 'a';
                    int y = sgf[i+2] - 'a';
                    b.move(x, y, c);
                }
            }
        };
        put_stones(aw, Color::WHITE);
        put_stones(ab, Color::BLACK);

        b.turn_ = Color::WHITE;

        // ★セットアップ汚染を防ぐため、履歴を初期局面のみにリセットする★
        b.hash_history_.clear();
        b.hash_history_.insert(b.current_hash_);
        b.last_hash_ = 0; 
    };

    setup_board(board_simple);
    setup_board(board_super);

    std::vector<std::pair<int, int>> moves = {
        {11, 0}, {13, 0}, {12, 0}
    };

    for (auto m : moves) {
        EXPECT_EQ(board_simple.check_move(m.first, m.second), MoveStatus::VALID);
        EXPECT_TRUE(board_simple.move(m.first, m.second));

        EXPECT_EQ(board_super.check_move(m.first, m.second), MoveStatus::VALID);
        EXPECT_TRUE(board_super.move(m.first, m.second));
    }

    // 運命の4手目 (黒が 14,0 に打つ)
    EXPECT_EQ(board_simple.check_move(14, 0), MoveStatus::VALID);
    EXPECT_TRUE(board_simple.move(14, 0));

    EXPECT_EQ(board_super.check_move(14, 0), MoveStatus::ILLEGAL_SUPERKO);
    EXPECT_FALSE(board_super.move(14, 0));
}

} // namespace test
} // namespace goboard