#ifndef __MODEL_H__
#define __MODEL_H__

#include <vector>
#include "geometry.h"

class Model {
private:
	std::vector<Vec3f> verts_ = {};
	std::vector<Vec3f> vertsnorm_ = {};
	std::vector<Vec2f> vertsdimtwo_ = {};
	std::vector<std::vector<int> > faces_ = {};
	std::vector<std::vector<int> > facesdimtwo_ = {};
public:
	Model(const char *filename);
	~Model();
	int nverts();
	int nvertsnorm();
	int nfaces();
	Vec3f vert(int i);
	Vec3f vertsnorm(int i);
	Vec2f vertdimtwo(int i);

	std::vector<int> face(int idx);
	std::vector<int> facedimtwo(int idxdimtwo);

};

#endif //__MODEL_H__