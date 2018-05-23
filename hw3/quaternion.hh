#include <cmath>
#include <iostream>

using namespace std;

class Quat {
private:
    void copy(const Quat &rhs) {
	for (int i = 0; i < 4; i++)
	    q[i] = rhs.q[i];
    }
    
public:
    double q[4];

    // **************************************************
    // Constructors and destructor:
    // **************************************************

    Quat(double si = 0, double xi = 0, double yi = 0, double zi = 0) {
	q[0] = si;
	q[1] = xi;
	q[2] = yi;
	q[3] = zi;
    }

    Quat(const Quat &in) {
	copy(in);
    }
    
    ~Quat() {
    }

    // **************************************************
    // Assignment operator
    // **************************************************
    
    Quat & operator=(const Quat &rhs) {
	if (this != &rhs)
	    copy(rhs);
	return *this;
    }

    // **************************************************
    // Boolean operators
    // **************************************************
    
    bool operator==(const Quat &rhs) const {
	for (int i = 0; i < 4; i++) 
	    if (q[i] != rhs.q[i])
		return false;
	return true;
    }

    bool operator!=(const Quat &rhs) const {
	return !(*this == rhs);
    }
    
    // **************************************************
    // Arithmetic operators
    // **************************************************

    Quat & operator+=(const Quat &rhs) {
	for (int i = 0; i < 4; i++)
	    q[i] += rhs.q[i];
	return *this;
    }

    Quat & operator-=(const Quat &rhs) {
	for (int i = 0; i < 4; i++)
	    q[i] -= rhs.q[i];
	return *this;
    }

    Quat & operator*=(double val) {
	for (int i = 0; i < 4; i++)
	    q[i] *= val;
	return *this;
    }

    Quat & operator*=(const Quat &rhs) {
	*this = *this * rhs;
	return *this;
    }

    Quat & operator/=(double val) {
	*this *= (1 / val);
	return *this;
    }
    
    Quat & operator/=(const Quat &rhs) {
	*this *= rhs.inverse();
	return *this;
    }

    const Quat operator+(const Quat &rhs) const {
	Quat ret(*this);
	ret += rhs;
	return ret;
    }

    const Quat operator-(const Quat &rhs) const {
	Quat ret(*this);
	ret -= rhs;
	return ret;
    }

    const Quat operator*(const Quat &rhs) const {
	Quat r;
	r.q[0] = q[0]*rhs.q[0] - q[1]*rhs.q[1] - q[2]*rhs.q[2] - q[3]*rhs.q[3];
	r.q[1] = q[0]*rhs.q[1] + q[1]*rhs.q[0] + q[2]*rhs.q[3] - q[3]*rhs.q[2];
	r.q[2] = q[0]*rhs.q[2] - q[1]*rhs.q[3] + q[2]*rhs.q[0] + q[3]*rhs.q[1];
	r.q[3] = q[0]*rhs.q[3] + q[1]*rhs.q[2] - q[2]*rhs.q[1] + q[3]*rhs.q[0];
	return r;
    }

    const Quat operator*(double val) const {
	Quat ret(*this);
	ret *= val;
	return ret;
    }

    const Quat operator/(double val) const {
	return (*this * (1 / val));
    }

    // **************************************************
    // Access functions
    // **************************************************

    double norm() const {
	double ret = 0;
	for (int i = 0; i < 4; i++)
	    ret += q[i] * q[i];
	return sqrt(ret);
    }

    const Quat conjugate() const {
	Quat ret(*this);
	ret *= -1;
	ret.q[0] *= -1;
	return ret;
    }

    const Quat inverse() const {
	Quat ret = this->conjugate();
	double n = this->norm();
	ret /= (n * n);
	return ret;
    }

    void print() const {
	cout << q[0] << " + " << q[1] << "i + " << q[2] << "j + "
	     << q[3] << "k" << endl;
    }
};
