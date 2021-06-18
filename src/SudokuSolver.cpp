#include "SudokuSolver.h"
#include <iostream>

static const unsigned int shifts[10] = { 0, 1, 2, 4, 8, 16, 32, 64, 128, 256 };

static const unsigned int bit_set_counts[65536] =
{
#   define B2(n)  n,      n+1,      n+1,      n+2
#   define B4(n)  B2(n),  B2(n+1),  B2(n+1),  B2(n+2)
#   define B6(n)  B4(n),  B4(n+1),  B4(n+1),  B4(n+2)
#   define B8(n)  B6(n),  B6(n+1),  B6(n+1),  B6(n+2)
#   define B10(n) B8(n),  B8(n+1),  B8(n+1),  B8(n+2)
#   define B12(n) B10(n), B10(n+1), B10(n+1), B10(n+2)
#   define B14(n) B12(n), B12(n+1), B12(n+1), B12(n+2)
           B14(0),B14(1), B14(1),   B14(2)
};

static unsigned int calculate_one_count(unsigned short n) {
    return bit_set_counts[n];
}

SudokuData SudokuSolver::produce_sudoku_data(int board[9][9])
{
    SudokuData sudoku_data = {};
    memcpy(sudoku_data.board, board, sizeof(int) * 9 * 9);

    for (int i = 0; i < 9; i++) {
        sudoku_data.rows[i] = 0;
        sudoku_data.cols[i] = 0;
        sudoku_data.squares[i] = 0;
    }

    for (int i = 0; i < 9; i++) {
        for (int j = 0; j < 9; j++) {
            int shift = shifts[board[i][j]];
            sudoku_data.rows[i] |= shift;
            sudoku_data.cols[j] |= shift;
            sudoku_data.squares[(i / 3) * 3 + j / 3] |= shift;
        }
    }

    return sudoku_data;
}

void SudokuSolver::print(SudokuData data) {
    for (int i = 0; i < 9; i++) {
        if (i % 3 == 0 && i != 0) {
            std::cout << "------ ------- ------ \n";
        }
        for (int j = 0; j < 9; j++) {
            if (j % 3 == 0 && j != 0) {
                std::cout << "| ";
            }
            std::cout << data.board[i][j] << " ";
        }
        std::cout << "\n";
    }
}

EmptyPoint SudokuSolver::find_empty_points(SudokuData data) {
    unsigned short possible_value_bits[9][9];

    EmptyPoint selected_point;
    selected_point.p.row = -1;
    selected_point.p.col = -1;
    selected_point.num_possible_values = INT_MAX;

    for (int i = 0; i < 9; i++) {
        for (int j = 0; j < 9; j++) {
            if (data.board[i][j] == 0) {
                possible_value_bits[i][j] = ~(data.rows[i] | data.cols[j] | data.squares[(i / 3) * 3 + j / 3]);

                if (calculate_one_count(possible_value_bits[i][j]) == 7) {
                    selected_point.p.row = -2;
                    return selected_point;
                }
            }
        }
    }

    for (int i = 0; i < 9; i++) {
        for (int j = 0; j < 9; j++) {
            if (data.board[i][j] == 0) {
                EmptyPoint ep;
                ep.p.row = i;
                ep.p.col = j;
                ep.possible_value_bits = ~(data.rows[i] | data.cols[j] | data.squares[(i / 3) * 3 + j / 3]);
                ep.num_possible_values = calculate_one_count(ep.possible_value_bits);

                //check if there is actually a single solution
                unsigned short current_value_bits = ep.possible_value_bits;
                int squareRow = i / 3;
                int squareCol = j / 3;

                for (int r = 0; r < 3; r++) {
                    for (int c = 0; c < 3; c++) {
                        int coordRow = squareRow * 3 + r;
                        int coordCol = squareCol * 3 + c;

                        if (!(coordRow == i && coordCol == j) && data.board[coordRow][coordCol] == 0) {
                            unsigned short value_bits = possible_value_bits[coordRow][coordCol];
                            current_value_bits ^= current_value_bits & value_bits;
                        }
                    }
                }
                current_value_bits |= 65024; //adding back the initial ones, decimal of: 1111111000000000

                if (calculate_one_count(current_value_bits) == 8) {
                    selected_point.p.row = i;
                    selected_point.p.col = j;
                    selected_point.possible_value_bits = current_value_bits;
                    return selected_point;
                }

                if (ep.num_possible_values < selected_point.num_possible_values) {
                    selected_point.p.row = i;
                    selected_point.p.col = j;
                    selected_point.possible_value_bits = ep.possible_value_bits;
                    selected_point.num_possible_values = ep.num_possible_values;
                }
            }
        }
    }

    return selected_point;
}

SudokuData SudokuSolver::solve(SudokuData data) {
    EmptyPoint epoint = find_empty_points(data);

    if (epoint.p.row == -2) {
        data.answer = false;
        
        return data;
    }

    Point p = epoint.p;

    if (p.col == -1 || p.row == -1) {
        data.answer = true;
        return data;
    }

    unsigned short valid_bits = epoint.possible_value_bits;

    for (int possible_value = 1; possible_value <= 9; possible_value++) {
        int shifted_val = shifts[possible_value];
        if (valid_bits & shifted_val) {
            data.board[p.row][p.col] = possible_value;
            data.rows[p.row] |= shifted_val;
            data.cols[p.col] |= shifted_val;
            int squareIndex = (p.row / 3) * 3 + p.col / 3;
            data.squares[squareIndex] |= shifted_val;

            auto new_data = solve(data);

            if (new_data.answer) {
                return new_data;
            }
            else {
                data.board[p.row][p.col] = 0;
                data.rows[p.row] ^= shifted_val;
                data.cols[p.col] ^= shifted_val;
                data.squares[squareIndex] ^= shifted_val;
            }
        }
    }

    data.answer = false;
    return data;
}

