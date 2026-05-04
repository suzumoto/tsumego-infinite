# GoogleTestを自動ダウンロードする設定 (モダンCMakeの標準的な手法)
include(FetchContent)
FetchContent_Declare(
  googletest
  URL https://github.com/google/googletest/archive/refs/tags/v1.14.0.zip
)
# Windows + MinGW(GCC)環境でのビルド競合を防ぐための設定
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

# テスト用の実行ファイルを作成
# ※後で StoneStringTest.cpp などを追加した場合はここに追記してください
add_executable(goboard_test GoboardTest.cpp)

# テスト実行ファイルに、goboard_lib と GoogleTest をリンク
target_link_libraries(goboard_test
    PRIVATE
    goboard_lib
    gtest_main
)

# CTestにテストとして登録する
include(GoogleTest)
gtest_discover_tests(goboard_test)