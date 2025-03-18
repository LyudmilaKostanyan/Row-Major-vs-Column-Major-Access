#include <iostream>
#include <iomanip>
#include <cstdlib>
#include <malloc.h>
#include <vector>  // Added for dynamic storage
#include "kaizen.h"

using namespace std;

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

bool parse_input(zen::cmd_args &args, int &row_size, int &col_size) {
    auto row_options = args.get_options("--row_size");
    auto col_options = args.get_options("--col_size");
    
    if (row_options.empty() && col_options.empty()) {
        std::cout << "Error: please write " << (row_options.empty() ? "--row_size" : "--col_size") << " parameter.";
        return false;
    }

    row_size = row_options.size() ? std::atoi(row_options[0].c_str()) : std::atoi(col_options[0].c_str());
    col_size = col_options.size() ? std::atoi(col_options[0].c_str()) : row_size;

    if (!row_size || !col_size) {
        std::cout << "Error: Row and column values must be greater than 0.";
        return false;
    }
    return true;
}

int** initialize_matrix(int row_size, int col_size) {
    int **matrix;
    
    matrix = new int*[row_size];
    if (!matrix)
        return nullptr;
    for (int i = 0; i < row_size; i++) {
        matrix[i] = new int[col_size];
        if (!matrix[i]) {
            for (int j = 0; j < i; j++)
                delete[] matrix[j];
            delete[] matrix;
            return nullptr;
        }
    }
    
    for (int i = 0; i < row_size; i++) {
        for (int j = 0; j < col_size; j++) {
            matrix[i][j] = i + j;
        }
    }
    return matrix;
}

void output_results(auto duration_row, auto duration_col, auto row_size, auto col_size) {
    double row_ms = duration_row;
    double col_ms = duration_col;
    double diff_ms = duration_col - duration_row;
    double speedup = row_ms ? col_ms / row_ms : 0;

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
    cout << "-----------------------------------------------------------------" << endl << endl;
}

void test_matrix_efficiency(int **matrix, int row_size, int col_size) {
    zen::timer timer;

    timer.start();
    rowMajorAccess(matrix, row_size, col_size);
    timer.stop();
    auto duration_row = timer.duration<zen::timer::msec>().count();
    
    timer.start();
    columnMajorAccess(matrix, row_size, col_size);
    timer.stop();
    auto duration_col = timer.duration<zen::timer::msec>().count();
    
    output_results(duration_row, duration_col, row_size, col_size);
}

void delete_matrix(int **matrix, int row_size) {
    for (int i = 0; i < row_size; i++) {
        delete[] matrix[i];
    }
    delete[] matrix;
}

template <typename matrix_t>
void test_aligned_matrix(matrix_t aligned_matrix, int row_size, int col_size, vector<pair<int, int>>& sizes, vector<pair<double, double>>& results) {
    for (int i = 0; i < row_size * col_size; i++)
        aligned_matrix[i] = i;
    
    zen::timer timer;
    volatile int sum = 0;
    
    timer.start();
    for (int i = 0; i < row_size * col_size; i++)
        sum += aligned_matrix[i];
    timer.stop();
    auto duration_row = timer.duration<zen::timer::nsec>().count();
    
    timer.start();
    for (int j = 0; j < col_size; ++j)
        for (int i = 0; i < row_size; ++i) 
            sum += aligned_matrix[i * col_size + j];
    timer.stop();
    auto duration_col = timer.duration<zen::timer::nsec>().count();
    
    sizes.push_back({row_size, col_size});
    results.push_back({duration_row, duration_col});
}

bool test_allocated_aligned_matrix(int row_size, int col_size, vector<pair<int, int>>& sizes, vector<pair<double, double>>& results) {
    #ifdef _WIN32
        alignas(64) int* aligned_matrix = static_cast<int*>(_aligned_malloc(row_size * col_size * sizeof(int), 64));
    #else
        alignas(64) int* aligned_matrix = static_cast<int*>(std::aligned_alloc(64, row_size * col_size * sizeof(int)));
    #endif
    if (!aligned_matrix)
        return false;
    
    test_aligned_matrix(aligned_matrix, row_size, col_size, sizes, results);
    
    #ifdef _WIN32
        _aligned_free(aligned_matrix);
    #else
        free(aligned_matrix);
    #endif
    return true;
}

void test_static_aligned_matrix(int row_size, int col_size, vector<pair<int, int>>& sizes, vector<pair<double, double>>& results) {
    alignas(64) int arr[row_size * col_size];
    test_aligned_matrix(arr, row_size, col_size, sizes, results);
}

void print_aligned_results(const vector<pair<int, int>>& sizes, const vector<pair<double, double>>& results) {
    cout << fixed << setprecision(2);
    cout << "--------------------------------------------------------------------------------------------------------" << endl;
    cout << left << setw(20) << "Matrix Size" 
         << right << setw(9) << "Row (ns)" 
         << setw(13) << "Column (ns)" 
         << setw(13) << "Speedup (x)" 
         << setw(13) << "Difference" 
         << setw(15) << "Row 4x5/4x4" 
         << setw(15) << "Col 4x5/4x4" << endl;
    cout << "--------------------------------------------------------------------------------------------------------" << endl;

    for (size_t i = 0; i < sizes.size(); i++) {
        double row_ns = results[i].first;
        double col_ns = results[i].second;
        double diff_ns = col_ns - row_ns;
        double speedup = row_ns ? col_ns / row_ns : 0;
        double row_ratio = (i == 0) ? 0 : row_ns / results[0].first;
        double col_ratio = (i == 0) ? 0 : col_ns / results[0].second;
        string size = to_string(sizes[i].first) + " x " + to_string(sizes[i].second);

        cout << left << setw(20) << size
             << right << setw(7) << row_ns 
             << setw(12) << col_ns 
             << setw(12) << speedup 
             << setw(17) << diff_ns 
             << setw(15) << row_ratio 
             << setw(15) << col_ratio << endl;
    }
    cout << "--------------------------------------------------------------------------------------------------------" << endl << endl;
}

int main(int argc, char **argv) {
    zen::cmd_args args(argv, argc);
    int row_size, col_size;
    int **matrix;
    
    if (!parse_input(args, row_size, col_size))
        return 1;
    matrix = initialize_matrix(row_size, col_size);
    if (!matrix)
        return 2;
    test_matrix_efficiency(matrix, row_size, col_size);
    delete_matrix(matrix, row_size);

    std::cout << "Testing row-major vs column-major traversal performance with cache-aligned and unaligned matrices: "
        << std::endl << "4x4 fits in one cache line, 4x5 spans two" << std::endl;

    vector<pair<int, int>> alloc_sizes, static_sizes;
    vector<pair<double, double>> alloc_results, static_results;

    std::cout << "Testing allocated aligned matrix performance: " << std::endl;
    test_allocated_aligned_matrix(4, 4, alloc_sizes, alloc_results);
    test_allocated_aligned_matrix(4, 5, alloc_sizes, alloc_results);
    print_aligned_results(alloc_sizes, alloc_results);

    std::cout << "Testing static aligned matrix performance: " << std::endl;
    test_static_aligned_matrix(4, 4, static_sizes, static_results);
    test_static_aligned_matrix(4, 5, static_sizes, static_results);
    print_aligned_results(static_sizes, static_results);

    return 0;
}