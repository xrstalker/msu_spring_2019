#pragma once

#include <exception>

class Matrix 
{
    static constexpr size_t size_limit = 2000000000ul;
    const size_t rows;
    const size_t cols;
    const int n;
    double *data;

    static bool 
    check_range(int i, size_t l) 
    {
        return ((i < 0 && -i <= l) || (i >= 0 && i < l));
    }

    class MatrixProxy {
        const size_t cols;
        double *data;
    public:
        MatrixProxy(double *data, const size_t cols):
            data(data), cols(cols)
        { }

        const double &
        operator[] (int j) const
        {
            if (not check_range(j, cols))
                throw std::out_of_range("last index out of range");
            
            if (j < 0)
                return data[cols + j];
            else
                return data[j];
        }
        double &
        operator[] (int j)
        {
            if (not check_range(j, cols))
                throw std::out_of_range("last index out of range");
            
            if (j < 0)
                return data[cols + j];
            else
                return data[j];
        }
    };

public:
    Matrix(const size_t rows, const size_t cols):
        rows(rows), cols(cols), n(rows*cols)
    {
        if (rows > size_limit || cols > size_limit || rows * cols > size_limit)
            throw std::bad_array_new_length();

        data = new double[n];
    }

    ~Matrix()
    {
        delete[] data;
    }

    size_t getRows() const
    {
        return rows;
    }
    size_t getColumns() const
    {
        return cols;
    }

    const MatrixProxy
    operator[] (int i) const
    {
        if (not check_range(i, rows))
            throw std::out_of_range("first index out of range");
        
        if (i < 0)
            return MatrixProxy(data + (rows + i) * cols, cols);
        else
            return MatrixProxy(data + i * cols, cols);
    }

    MatrixProxy
    operator[] (int i)
    {
        if (not check_range(i, rows))
            throw std::out_of_range("first index out of range");
        
        if (i < 0)
            return MatrixProxy(data + (rows + i) * cols, cols);
        else
            return MatrixProxy(data + i * cols, cols);
    }

    const Matrix &
    operator*= (double x) const
    {
        for (int i = 0; i < n; ++i)
            data[i] *= x;
        return *this;
    }
    
    Matrix &
    operator*= (double x)
    {
        for (int i = 0; i < n; ++i)
            data[i] *= x;
        return *this;
    }

    bool operator== (const Matrix &m) const
    {
        if (this == &m)
            return true;
        if (m.n != n)
            return false;
        for (int i = 0; i < n; ++i)
            if (data[i] != m.data[i])
                return false;
        return true;
    }
    bool operator!= (const Matrix &m) const
    {
        return !(*this == m);
    }
};
