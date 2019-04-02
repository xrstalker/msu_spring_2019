#pragma once

#include <exception>

class Matrix 
{
    const size_t rows;
    const size_t cols;
    const size_t n;
    double *data;

    static bool 
    check_range(size_t i, size_t l) 
    {
        return ((0 <= i && i < l) || (0 <= i+l && i+l < l));
    }

    static int
    ord2(size_t x) 
    {
        int r = 0;
        while (x) 
            r += 1, x /= 2;
        return r;
    }

    class MatrixProxy {
        const size_t cols;
        double *data;
    public:
        MatrixProxy(double *data, const size_t cols):
            data(data), cols(cols)
        { }

        const double &
        operator[] (size_t j) const
        {
            if (not check_range(j, cols))
                throw std::out_of_range("last index out of range");
            
            if (j < cols)
                return data[j];
            else
                return data[j + cols];
        }
        double &
        operator[] (size_t j)
        {
            if (not check_range(j, cols))
                throw std::out_of_range("last index out of range");
            
            if (j < cols)
                return data[j];
            else
                return data[j + cols];
        }
    };

public:
    Matrix(const size_t rows, const size_t cols):
        rows(rows), cols(cols), n(rows*cols)
    {
        if (ord2(rows) + ord2(cols) + 2 > 8*sizeof n)
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
    operator[] (size_t i) const
    {
        if (not check_range(i, rows))
            throw std::out_of_range("first index out of range");
        
        if (i < rows)
            return MatrixProxy(data + i * cols, cols);
        else
            return MatrixProxy(data + (i + rows) * cols, cols);
    }

    MatrixProxy
    operator[] (size_t i)
    {
        if (not check_range(i, rows))
            throw std::out_of_range("first index out of range");
        
        if (i < rows)
            return MatrixProxy(data + i * cols, cols);
        else
            return MatrixProxy(data + (i + rows) * cols, cols);
    }

    const Matrix &
    operator*= (double x) const
    {
        for (size_t i = 0; i < n; ++i)
            data[i] *= x;
        return *this;
    }
    
    Matrix &
    operator*= (double x)
    {
        for (size_t i = 0; i < n; ++i)
            data[i] *= x;
        return *this;
    }

    bool operator== (const Matrix &m) const
    {
        if (this == &m)
            return true;
        if (m.n != n)
            return false;
        for (size_t i = 0; i < n; ++i)
            if (data[i] != m.data[i])
                return false;
        return true;
    }
    bool operator!= (const Matrix &m) const
    {
        return !(*this == m);
    }
};
