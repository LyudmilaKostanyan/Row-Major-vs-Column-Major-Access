#include <iostream>
#include <chrono>
#include <iomanip>
#include "kaizen.h"
#include <cstdlib>

using namespace std;
using namespace std::chrono;

void rowMajorAccess(int** arr, int row_size, int col_size) {
    size_t sum = 0;
    for (int i = 0; i < row_size; i++) {
        for (int j = 0; j < col_size; j++) {
            sum += arr[i][j];
        }
    }
    volatile int result = sum;
}

void columnMajorAccess(int** arr, int row_size, int col_size) {
    size_t sum = 0;
    for (int i = 0; i < row_size; i++) {
            for (int j = 0; j < col_size; j++) {
            sum += arr[i][j];
        }
    }
    volatile int result = sum;
}

int **parse_and_initialize(zen::cmd_args &args, int &row_size, int &col_size)
{
    int **arr;

    auto row_size_str = args.get_options("--row_size");
    auto col_size_str = args.get_options("--col_size");
    
    if (row_size_str.empty() && col_size_str.empty())
    {
        std::cout << "Error: please write " << (row_size_str.empty() ? "--row_size" : "--col_size") << " parameter.";
        return NULL;
    }

    row_size = row_size_str.size() ? std::atoi(row_size_str[0].c_str()) : std::atoi(col_size_str[0].c_str());
    col_size = col_size_str.size() ? std::atoi(col_size_str[0].c_str()) : row_size;

    if (!row_size || !col_size)
    {
        std::cout << "Error: Row and column values must be greater than 0.";
        return NULL;
    }
    
    arr = new int*[row_size];
    for (int i = 0; i < row_size; i++) {
        arr[i] = new int[col_size];
    }
    
    for (int i = 0; i < row_size; i++) {
        for (int j = 0; j < col_size; j++) {
            arr[i][j] = i + j;
        }
    }
    return arr;
}

int main(int argc, char **argv) {
    zen::cmd_args  args(argv, argc);
    int row_size;
    int col_size;
    int **arr;
    
    arr = parse_and_initialize(args, row_size, col_size);
    if (!arr)
        return 1;

    auto start_row = high_resolution_clock::now();
    rowMajorAccess(arr, row_size, col_size);
    auto end_row = high_resolution_clock::now();
    auto duration_row = duration_cast<microseconds>(end_row - start_row);
    auto start_col = high_resolution_clock::now();
    columnMajorAccess(arr, row_size, col_size);
    auto end_col = high_resolution_clock::now();
    auto duration_col = duration_cast<microseconds>(end_col - start_col);

    double row_ms = duration_row.count() / 1000.0;
    double col_ms = duration_col.count() / 1000.0;
    double diff_ms = (duration_col.count() - duration_row.count()) / 1000.0;
    double speedup = col_ms / row_ms;

    cout << "Array Size: " << row_size << " x " << col_size << endl;
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
         << setw(12) << diff_ms << endl;
    cout << "-----------------------------------------------------------------";

    for (int i = 0; i < row_size; i++) {
        delete[] arr[i];
    }
    delete[] arr;

    return 0;
}