# Row-Major vs. Column-Major Access Benchmark

## Project Description

This project demonstrates the performance difference between **row-major** and **column-major** order when accessing a 2D array in memory. The project highlights how **spatial locality** affects performance.

### Background

- In **row-major order**, consecutive elements of each row are stored contiguously in memory.
- In **column-major order**, consecutive elements of each column are stored contiguously in memory.

Modern processors take advantage of **caching** mechanisms, which prefer accessing data laid out contiguously. Therefore, **row-major order** is significantly faster for row-based iteration compared to column-based iteration in languages like C/C++, which store arrays in row-major order by default.

The program initializes a 2D array of size **N x N** (or **N x M** if row and column sizes differ) and measures the time taken to access all elements in both **row-major** and **column-major** order.

---

## How to Build and Run

### Clone the Repository

```bash
git clone https://github.com/LyudmilaKostanyan/Bandwidth-Saturation-Test.git
cd bandwidth-saturation-test
```

### Build the Project

```bash
cmake -S . -B build
cmake --build build
```

### Run the Program

Navigate into the `build` directory:

```bash
cd build
```

### Parameters

You can specify the size of the 2D array using the following command-line arguments:

- **Square Array (N x N):**
    ```bash
    ./main.exe --row_size 5000
    ```
    This sets both rows and columns to size 5000, resulting in a **5000 x 5000** array.

- **Rectangular Array (N x M):**
    ```bash
    ./main.exe --row_size 2000 --col_size 4000
    ```
    This sets the array to **2000 rows x 4000 columns**.

- If **--col_size** is omitted, the array will be square with **--row_size x --row_size**.

---

## Example Output

```text
$ ./main.exe --row_size 50000

Array Size: 50000 x 50000
-----------------------------------------------------------------
                Row (ms)  Column (ms)  Speedup (x)   Difference
-----------------------------------------------------------------
Access Time    5909.00    67403.00       11.41         61494.00
-----------------------------------------------------------------
```

---

## Explanation of Output

| Column         | Explanation |
|----------------|-------------|
| **Row (ms)**   | Time taken to iterate the array in **row-major order** (row-by-row). |
| **Column (ms)**| Time taken to iterate the array in **column-major order** (column-by-column). |
| **Speedup (x)**| How many times faster row-major access is compared to column-major access (`Column Time ÷ Row Time`). |
| **Difference** | The absolute time difference between row-major and column-major access (`Column Time - Row Time`). |

### Key Observations

- Row-major access is significantly faster because **consecutive elements in each row are stored contiguously in memory**, which makes better use of the **CPU cache**.
- Column-major access is slower because accessing elements column-by-column results in frequent cache misses.
- The **speedup factor** quantifies how much more efficient row-major access is compared to column-major access.
- Larger matrices make this performance gap even more noticeable.

### Example Explanation

In the output example above:

- For a **50000 x 50000** array:
    - Row-major access took **5909 ms**.
    - Column-major access took **67403 ms**.
    - Row-major was **11.41 times faster**.
    - The time difference was **61494 ms**, demonstrating how inefficient column-major traversal is on large arrays.
