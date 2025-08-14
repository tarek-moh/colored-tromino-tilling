#include <iostream>
#include <vector>
#include <unordered_map>
using namespace std;

enum class Color
{
	RED, GREEN, BLUE, EMPTY, MISSING
};
enum class Quadrant
{
	TL,TR,BL,BR
};

struct DPKey
{
	int size, x, y;
	bool operator==(const DPKey& other) const
	{
		return size == other.size && x == other.x && y == other.y;
	}
};

struct DPKeyHash
{
	size_t operator()(const DPKey& k) const
	{
		return ((hash<int>()(k.size) ^ (hash<int>()(k.x) << 1)) >> 1) ^ (hash<int>()(k.y) << 1);
	}

};

using Board = vector<vector<Color>>;
unordered_map<DPKey, Board, DPKeyHash> DP;

void printBoard(const Board& b) {
	for (auto& row : b) {
		for (auto c : row) {
			char ch = '.';
			switch (c) {
			case Color::RED:     ch = 'R'; break;
			case Color::GREEN:   ch = 'G'; break;
			case Color::BLUE:    ch = 'B'; break;
			case Color::MISSING: ch = 'X'; break;
			case Color::EMPTY:	 ch = '.'; break; // for debugging
			default: break;
			}
			cout << ch << " ";
		}
		cout << "\n";
	}
	cout << "\n";
}
// ----- construct_4x4_pattern -----
Board construct_4x4_pattern(int missing_x, int missing_y) {
    Board board = {
        {Color::GREEN, Color::GREEN, Color::BLUE, Color::BLUE},
        {Color::GREEN, Color::RED,   Color::RED,  Color::BLUE},
        {Color::BLUE,  Color::RED,   Color::RED,  Color::GREEN},
        {Color::BLUE,  Color::BLUE,  Color::GREEN,Color::GREEN}
    };

    // If the missing tile is not red, adjust nearest central red
    if (board[missing_y][missing_x] != Color::RED) {
        int swap_x = missing_x;
        int swap_y = missing_y;

        if (swap_x == 0) swap_x = 1;
        if (swap_x == 3) swap_x = 2;
        if (swap_y == 0) swap_y = 1;
        if (swap_y == 3) swap_y = 2;

        board[swap_y][swap_x] = board[missing_y][missing_x];
    }

    // Mark missing tile
    board[missing_y][missing_x] = Color::MISSING;
    return board;
}

// ----- edge case ------
Board construct2x2(int x, int y)
{
    Board board = {
        {Color::RED, Color::RED},
        {Color::RED, Color::RED}
    };
    board[y][x] = Color::MISSING;
    return board;
}

// ----- pick_center_cells_excluding -----
vector<pair<int, int>> pick_center_cells_excluding(Quadrant q, int mid) {
    vector<pair<int, int>> cells;
    if (q != Quadrant::TL) cells.push_back({ mid - 1, mid - 1 });
    if (q != Quadrant::TR) cells.push_back({ mid,   mid - 1 });
    if (q != Quadrant::BL) cells.push_back({ mid - 1, mid });
    if (q != Quadrant::BR) cells.push_back({ mid,   mid });
    return cells;
}

// ----- combine -----
Board combine(const Board& TLb, const Board& TRb,
    const Board& BLb, const Board& BRb,
    int mid, const vector<pair<int, int>>& central_cells) {
    int boardSize = mid * 2;
    Board board(boardSize, vector<Color>(boardSize, Color::EMPTY));

    // Copy quadrants
    for (int y = 0; y < mid; y++) {
        for (int x = 0; x < mid; x++) {
            board[y][x] = TLb[y][x];               // TL
            board[y][x + mid] = TRb[y][x];           // TR
            board[y + mid][x] = BLb[y][x];           // BL
            board[y + mid][x + mid] = BRb[y][x];       // BR
        }
    }

    // Place the central red tromino
    for (auto coord : central_cells) {
        board[coord.second][coord.first] = Color::RED;
    }
    return board;
}



// ----- tile recursive -----
Board tile(int size, int missing_x, int missing_y) {
    DPKey key{ size, missing_x, missing_y };
    auto it = DP.find(key);
    if (it != DP.end()) return it->second;

    Board solution;
    
    if (size == 2)
    {
        return construct2x2(missing_x, missing_y);
    }

    if (size == 4) {
        solution = construct_4x4_pattern(missing_x, missing_y);
        DP[key] = solution;
        return solution;
    }

    int mid = size / 2;
    Quadrant missing_quadrant;
    if (missing_x < mid && missing_y < mid)
        missing_quadrant = Quadrant::TL;
    else if (missing_x >= mid && missing_y < mid)
        missing_quadrant = Quadrant::TR;
    else if (missing_x < mid && missing_y >= mid)
        missing_quadrant = Quadrant::BL;
    else
        missing_quadrant = Quadrant::BR;

    // Recurse into each quadrant
    Board TLb, TRb, BLb, BRb;
    for (Quadrant q : {Quadrant::TL, Quadrant::TR, Quadrant::BL, Quadrant::BR}) {
        int new_mx, new_my;
        if (q == missing_quadrant) {
            new_mx = missing_x % mid;
            new_my = missing_y % mid;
        }
        else {
            // artificial missing tile at center corner of this quadrant
            new_mx = (q == Quadrant::TR || q == Quadrant::BR) ? 0 : mid - 1;
            new_my = (q == Quadrant::BL || q == Quadrant::BR) ? 0 : mid - 1;
        }

        Board sub = tile(mid, new_mx, new_my);
        if (q == Quadrant::TL) TLb = sub;
        if (q == Quadrant::TR) TRb = sub;
        if (q == Quadrant::BL) BLb = sub;
        if (q == Quadrant::BR) BRb = sub;
    }

    // Get central tromino cells
    auto central_cells = pick_center_cells_excluding(missing_quadrant, mid);

    solution = combine(TLb, TRb, BLb, BRb, mid, central_cells);
    DP[key] = solution;
    return solution;
}

int main() {
    int size = 8;
	int missing_x = 0;
	int missing_y = 0;

	Board b = tile(size, missing_x, missing_y);
	printBoard(b);
}

