#pragma once

struct SudokuPoint {
	int row;
	int col;
};

struct EmptyPoint {
	SudokuPoint p;
	unsigned int num_possible_values;
	unsigned short possible_value_bits;
};

struct SudokuData {
	int board[9][9];
	unsigned int rows[9];
	unsigned int cols[9];
	unsigned int squares[9];
	bool answer = false;
};

class SudokuSolver {
public:
	SudokuData produce_sudoku_data(int board[9][9]);
	void print(SudokuData data);
	SudokuData solve(SudokuData data);
private:
	EmptyPoint find_empty_points(SudokuData data);
};