#include <iostream>
#include <chrono>
#include <iomanip>
#include "kaizen.h"
#include <cstdlib>

using namespace std;
using namespace std::chrono;

void rowMajorAccess(int** matrix, int row_size, int col_size) {
    volatile size_t sum = 0;
    for (int i = 0; i < row_size; i++) {
        for (int j = 0; j < col_size; j++) {
            sum += matrix[i][j];
        }
    }
}

void columnMajorAccess(int** matrix, int row_size, int col_size) {
    volatile size_t sum = 0;
    for (int j = 0; j < col_size; j++) {
        for (int i = 0; i < row_size; i++) {
            sum += matrix[i][j];
        }
    }
}

int** parse_and_initialize(zen::cmd_args &args, int &row_size, int &col_size)
{
    int **matrix;

    auto row_options = args.get_options("--row_size");
    auto col_options = args.get_options("--col_size");
    
    if (row_options.empty() && col_options.empty())
    {
        std::cout << "Error: please write " << (row_options.empty() ? "--row_size" : "--col_size") << " parameter.";
        return nullptr;
    }

    row_size = row_options.size() ? std::atoi(row_options[0].c_str()) : std::atoi(col_options[0].c_str());
    col_size = col_options.size() ? std::atoi(col_options[0].c_str()) : row_size;

    if (!row_size || !col_size)
    {
        std::cout << "Error: Row and column values must be greater than 0.";
        return nullptr;
    }
    
    matrix = new int*[row_size];
    for (int i = 0; i < row_size; i++) {
        matrix[i] = new int[col_size];
    }
    
    for (int i = 0; i < row_size; i++) {
        for (int j = 0; j < col_size; j++) {
            matrix[i][j] = i + j;
        }
    }
    return matrix;
}

void output_results(auto duration_row, auto duration_col, auto row_size, auto col_size)
{
    double row_ms = duration_row.count();
    double col_ms = duration_col.count();
    double diff_ms = (duration_col.count() - duration_row.count());
    double speedup = col_ms / row_ms;

    cout << "Matrix Size: " << row_size << " x " << col_size << endl;
    cout << fixed << setprecision(2);
    cout << "-----------------------------------------------------------------" << endl;
    cout << left << setw(15) << "" 
         << right << setw(9) << "Row (ms)" 
         << setw(13) << "Column (ms)" 
         << setw(13) << "Speedup (x)" 
         << setw(13) << "Difference" << endl;
    cout << "-----------------------------------------------------------------" << endl;
    
    cout << left << setw(15) << "Access Time" 
         << right << setw(7) << row_ms 
         << setw(12) << col_ms 
         << setw(12) << speedup 
         << setw(17) << diff_ms << endl;
    cout << "-----------------------------------------------------------------";
}

int main(int argc, char **argv) {
    zen::cmd_args  args(argv, argc);
    int row_size;
    int col_size;
    int **matrix;
    
    matrix = parse_and_initialize(args, row_size, col_size);
    if (!matrix)
        return 1;

    auto start_row = high_resolution_clock::now();
    rowMajorAccess(matrix, row_size, col_size);
    auto end_row = high_resolution_clock::now();
    auto duration_row = duration_cast<milliseconds>(end_row - start_row);
    auto start_col = high_resolution_clock::now();
    columnMajorAccess(matrix, row_size, col_size);
    auto end_col = high_resolution_clock::now();
    auto duration_col = duration_cast<milliseconds>(end_col - start_col);

    output_results(duration_row, duration_col, row_size, col_size);

    for (int i = 0; i < row_size; i++) {
        delete[] matrix[i];
    }
    delete[] matrix;

    return 0;
}