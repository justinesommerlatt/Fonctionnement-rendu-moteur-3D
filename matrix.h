#ifndef __MATRIX_H__
#define __MATRIX_H__

#include <vector>
#include <cmath>

class matrix33 {
public:
	std::vector<double> values;
	matrix33() : values(9, 0){}
	matrix33(double m11, double m12, double m13, double m21, double m22, double m23, double m31, double m32, double m33): values(9, 0){
		values[0] = m11;
		values[1] = m12;
		values[2] = m13;
		values[3] = m21;
		values[4] = m22;
		values[5] = m23;
		values[6] = m31;
		values[7] = m32;
		values[8] = m33;
	}
	matrix33(const matrix33& m): values(9, 0){
		values[0] = m.values[0];
		values[1] = m.values[1];
		values[2] = m.values[2];
		values[3] = m.values[3];
		values[4] = m.values[4];
		values[5] = m.values[5];
		values[6] = m.values[6];
		values[7] = m.values[7];
		values[8] = m.values[8];
	}

	double operator[](int i){
		return values[i];
	}

	matrix33& operator*=(double d) {
		for (int i = 0; i < 3; i++) 
		{
			for (int j = 0; j < 3; j++)
			{
				values[i * 3 + j] *= d;
			}
		}
		return *this;
	}

	double determinant(matrix33 m) {
		return m.values[0] * (m.values[4] * m.values[8] - m.values[5] * m.values[7]) - m.values[1] * (m.values[3] * m.values[8] - m.values[5] * m.values[6]) + m.values[2] * (m.values[3] * m.values[7] - m.values[4] * m.values[6]);
	}

	double determinant(std::vector<double> v) {
		return v[0] * v[3] - v[1] * v[2];
	}

	matrix33 transpose(matrix33 m) {
		matrix33 res;
		for (int i = 0; i < 3; i++)
		{
			for (int j = 0; j < 3; j++)
			{
				res.values[j * 3 + i] = m.values[i * 3 + j];
			}
		}
		return res;
	}

	matrix33 cofactor(matrix33 m) {
		matrix33 sol;
		std::vector<double> subMat(4);

		for (int i = 0; i < 3; i++)
		{
			for (int j = 0; j < 3; j++)
			{
				int p = 0;
				for (int k = 0; k < 3; k++)
				{
					if (i == k) {
						continue;
					}
					int q = 0;
					for (int l = 0; l < 3; l++)
					{
						if (j == l) {
							continue;
						}

						subMat[p * 2 + q] = m.values[k * 3 + l];
						q++;
					}
					p++;
				}
				sol.values[i * 3 + j] = pow(-1, i + j) * determinant(subMat);
			}
		}
		return sol;
	}

	matrix33 inverse(matrix33 m) {
		if (determinant(m) == 0) {
			throw std::runtime_error("Determinant is 0");
		}
		double d = 1.0 / determinant(m);
		matrix33 sol(transpose(cofactor(m)));
		sol *= d;
		return sol;
	}

	std::vector<double> mult3x1(matrix33 m, std::vector<double> m2) {
		std::vector<double> res(3,0);
		res[0] = m[0] * m2[0] + m[1] * m2[1] + m[2] * m2[2];
		res[1] = m[3] * m2[0] + m[4] * m2[1] + m[5] * m2[2];
		res[2] = m[6] * m2[0] + m[7] * m2[1] + m[8] * m2[2];
		return res;
	}
};

#endif
