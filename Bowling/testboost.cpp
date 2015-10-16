#include <boost/numeric/ublas/matrix.hpp>         // (1) 普通の行列用のヘッダ
#include <boost/numeric/ublas/triangular.hpp>     // (2) 三角行列用のヘッダ
#include <boost/numeric/ublas/symmetric.hpp>      // (3) 対称行列用のヘッダ
#include <boost/numeric/ublas/hermitian.hpp>      // (4) エルミート行列用のヘッダ
#include <boost/numeric/ublas/matrix_sparse.hpp>  // (5) 疎行列用のヘッダ
#include <boost/numeric/ublas/vector.hpp>         // (6) ベクトル用のヘッダ
#include <boost/numeric/ublas/vector_sparse.hpp>  // (7) 疎ベクトル用のヘッダ
#include <boost/numeric/ublas/io.hpp>
using namespace boost::numeric::ublas;  // boost::numeric::ublas 名前空間を使用
using namespace boost::numeric;         // boost::numeric 名前空間を使用(※下注)



class Point {
public:
    vector<double> a;
    //Point(vector<double> _a(3)){a = _a;}
    Point(vector<double> _a): a(_a){
        //a(0) = _a(0); a(1) = _a(1); a(2) = _a(2);
    }
    Point(std::vector<double> _a) {
        a(1) = _a[1]; a(0) = _a[0]; a(2) = _a[2];
    }
};

int main () {
    vector<double> v (3);
    for (unsigned i = 0; i < v.size (); ++ i)
        v (i) = i;
    Point a(v);
    Point b(std::vector<double> {0,1,2});
    std::cout << a.a << std::endl;
    std::cout << b.a << std::endl;
    std::cout << v << std::endl;
    for (int i = 0; i < 3; ++ i) {
        unit_vector<double> v1 (3, i);
        std::cout << v1 << std::endl;
    }
    zero_vector<double> v2 (3);
    std::cout << v2 << std::endl;

    v(0) = 1; v(1) = 2; v(2) = 3;
    std::cout << v << std::endl;    
}