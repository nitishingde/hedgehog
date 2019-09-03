////
//// Created by anb22 on 6/20/19.
////

#include "hedgehog/hedgehog.h"

enum class MatrixType {
  MatrixA,
  MatrixB,
  MatrixC,
  MatrixAny
};

class MatrixRequestData {
 public:
  MatrixRequestData(size_t row, size_t col, MatrixType type) : row(row), col(col), type(type) {}

  size_t getRow() const {
    return row;
  }
  size_t getCol() const {
    return col;
  }
  MatrixType getType() const {
    return type;
  }

 private:
  size_t row;
  size_t col;
  MatrixType type;
};

template<class Type>
class MatrixBlockData {
 public:

  MatrixBlockData(const std::shared_ptr<MatrixRequestData> &request,
                  const Type &matrixData,
                  size_t matrixWidth,
                  size_t matrixHeight,
                  size_t leadingDimension) :
      request(request), matrixData(matrixData), matrixWidth(matrixWidth), matrixHeight(matrixHeight), leadingDimension(leadingDimension) {}

  const std::shared_ptr<MatrixRequestData> &getRequest() const {
    return request;
  }
  const Type &getMatrixData() const {
    return matrixData;
  }
  size_t getMatrixWidth() const {
    return matrixWidth;
  }
  size_t getMatrixHeight() const {
    return matrixHeight;
  }

  size_t getLeadingDimension() const {
    return leadingDimension;
  }

 private:
  std::shared_ptr<MatrixRequestData> request;
  Type matrixData;
  size_t matrixWidth;
  size_t matrixHeight;
  size_t leadingDimension;
};

template <class Type>
class MatrixBlockMulData {
 public:

  MatrixBlockMulData(const std::shared_ptr<MatrixBlockData<Type>> &matrixA,
  const std::shared_ptr<MatrixBlockData<Type>> &matrixB,
  const std::shared_ptr<MatrixBlockData<Type>> &matrixC) :
  matrixA(matrixA), matrixB(matrixB), matrixC(matrixC) {}

  const std::shared_ptr<MatrixBlockData<Type>> &getMatrixA() const {
    return matrixA;
  }
  const std::shared_ptr<MatrixBlockData<Type>> &getMatrixB() const {
    return matrixB;
  }

  const std::shared_ptr<MatrixBlockData<Type>> &getMatrixC() const {
    return matrixC;
  }

 private:
  std::shared_ptr<MatrixBlockData<Type>> matrixA;
  std::shared_ptr<MatrixBlockData<Type>> matrixB;
  std::shared_ptr<MatrixBlockData<Type>> matrixC;
};




int main(){

  return 0;
}