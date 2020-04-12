#include "togasat.hpp"
using namespace std;

constexpr int N = 16;
constexpr int R = 4;
constexpr int M = 64;
constexpr int H = 4;
constexpr int W = 4;
constexpr int Q = 3;
using Cards = vector<vector<int>>;

inline int boxId(int y, int x)
{
    return y * W + x;
}
inline int cardId(int card_num, int rotate)
{
    return card_num * R + rotate;
}
inline int id(int box_id, int card_id)
{
    return box_id * M + card_id + 1;
}
Cards input()
{
    Cards ret(N, vector<int>(R));
    for (size_t i = 0; i < N; i++)
    {
        for (size_t j = 0; j < R; j++)
        {
            cin >> ret[i][j];
        }
    }
    return ret;
}
Cards rotate(Cards data)
{
    Cards ret(M, vector<int>(R));
    for (size_t i = 0; i < N; i++)
    {
        for (size_t j = 0; j < R; j++)
        {
            for (size_t k = 0; k < R; k++)
            {
                ret[cardId(i, j)][k] = data[i][(k + j) % R];
            }
        }
    }
    return ret;
}
void box_const(togasat::Solver &solver)
{
    int ma = 0;
    // 各マスに64枚中ちょうど1枚が入る
    for (size_t i = 0; i < N; i++)
    {
        vector<int> box;
        for (size_t j = 0; j < M; j++)
        {
            box.emplace_back(id(i, j));
        }
        solver.addClause(box);
        for (size_t j = 0; j < M; j++)
        {
            for (size_t k = j + 1; k < M; k++)
            {
                vector<int> twice = {-box[j], -box[k]};
                solver.addClause(twice);
            }
        }
    }
}
void card_const(togasat::Solver &solver)
{
    // 各カード(4枚,16マス)はちょうど1つ使う
    for (size_t i = 0; i < N; i++)
    {
        vector<int> card;
        for (size_t j = 0; j < N; j++)
        {
            for (size_t k = 0; k < R; k++)
            {
                card.emplace_back(id(j, cardId(i, k)));
            }
        }
        solver.addClause(card);
        for (size_t j = 0; j < M; j++)
        {
            for (size_t k = j + 1; k < M; k++)
            {
                vector<int> twice = {-card[j], -card[k]};
                solver.addClause(twice);
            }
        }
    }
}
void boundary_const(togasat::Solver &solver, Cards C)
{
    // 境界でXORが1にならないようなものが同時に置かれない
    vector<pair<int, int>> side;
    vector<pair<int, int>> vert;
    for (size_t i = 0; i < M; i++)
    {
        for (size_t j = 0; j < M; j++)
        {
            if (i / R == j / R)
                continue;
            if ((C[i][2] ^ C[j][0]) != 1)
            {
                side.emplace_back(make_pair(i, j));
            }
            if ((C[i][3] ^ C[j][1]) != 1)
            {
                vert.emplace_back(make_pair(i, j));
            }
        }
    }

    for (size_t i = 0; i < H; i++)
    {
        for (size_t j = 0; j < W; j++)
        {
            if (j + 1 < W)
            {
                int x = boxId(i, j);
                int y = boxId(i, j + 1);
                for (auto p : side)
                {
                    vector<int> pa = {-id(x, p.first), -id(y, p.second)};
                    solver.addClause(pa);
                }
            }
            if (i + 1 < H)
            {
                int x = boxId(i, j);
                int y = boxId(i + 1, j);
                for (auto p : vert)
                {
                    vector<int> pa = {-id(x, p.first), -id(y, p.second)};
                    solver.addClause(pa);
                }
            }
        }
    }
}
int main()
{
    cin.tie(0);
    ios::sync_with_stdio(false);

    // 16枚のカード情報を読み込む
    auto row_data = input();
    // 回転を考慮して64枚と考える
    auto data = rotate(row_data);
    togasat::Solver solver;

    // 白(0,1) 黒(2,3) 灰(4,5) 橙(6,7)
    // 0~3 1カード目, 4~7 2カード目
    // 各マスに64変数あるので64*16変数ある

    // 各マスは64枚中1枚を使う
    box_const(solver);
    // 16枚のカードは1回ずつ使う
    card_const(solver);
    // 境界はうまく繋がる
    boundary_const(solver, data);

    // ans1:{-54,-65,-133,-220,-268,-353,-397,-497,-575,-634,-669,-749,-789,-873,-916,-997};
    vector<int> ans1 = {-54};
    solver.addClause(ans1);
    // ans2:{-50,-105,-161,-246,-314,-358,-430,-477,-529,-582,-654,-732,-777,-896,-918,-964}
    vector<int> ans2 = {-50};
    solver.addClause(ans2);

    solver.solve();
    vector<int> ans(N);

    // 発見した割当を出力
    for (size_t i = 0; i < solver.assigns.size(); i++)
    {
        if (solver.assigns[i] == 0)
        {
            // (i/64)番目のマスはi%64番のカードとなる
            ans[i / 64] = i % 64;
            int ID = i + 1;
            cout << (-ID) << ",";
        }
    }
    cout << "\n";

    // 割当をもとにマップを生成
    vector<vector<char>> map(Q * H, vector<char>(Q * W, '.'));
    for (size_t i = 0; i < N; i++)
    {
        int y = i / W * Q;
        int x = i % W * Q;
        map[y + 1][x] = (char)('0' + data[ans[i]][0]);
        map[y][x + 1] = (char)('0' + data[ans[i]][1]);
        map[y + 1][x + 2] = (char)('0' + data[ans[i]][2]);
        map[y + 2][x + 1] = (char)('0' + data[ans[i]][3]);
    }
    // マップの出力
    for (size_t i = 0; i < Q * H; i++)
    {
        for (size_t j = 0; j < Q * W; j++)
        {
            cout << map[i][j];
        }
        cout << "\n";
    }
    // マップを一行で出力
    for (size_t i = 0; i < Q * H; i++)
    {
        for (size_t j = 0; j < Q * W; j++)
        {
            cout << map[i][j];
        }
    }
    cout << "\n";
    return 0;
}
