#include <iostream>
#include <string>
#include <fstream>
#include <sstream>
#include <vector>
#include "model.h"

Model::Model(const char *filename)  {
    std::ifstream in;
    in.open (filename, std::ifstream::in);
    if (in.fail()) return;
    std::string line;
    while (!in.eof()) {
        std::getline(in, line);
        std::istringstream iss(line.c_str());
        char trash;
        if (!line.compare(0, 2, "v ")) {
            iss >> trash;
            Vec3f v;
            for (int i=0;i<3;i++) iss >> v[i];
            verts_.push_back(v);
        } else if (!line.compare(0, 3, "vt ")) {
            iss >> trash >> trash;
            Vec2f v;
            for (int i=0;i<2;i++) iss >> v[i];
            vertsdimtwo_.push_back(v);
        } else if (!line.compare(0, 2, "f ")) {
            std::vector<int> f,g,h;
            int itrash, idx, idxdimtwo,idxdimthree;
            iss >> trash;
            while (iss >> idx >> trash >> idxdimtwo >> trash >> idxdimthree) {
                idx--; // tous les indices commencent à 1
                idxdimtwo--;
                idxdimthree--;
                f.push_back(idx);
                //je récupère les indices 2D
                g.push_back(idxdimtwo);
                h.push_back(idxdimthree);
            }
            faces_.push_back(f);
            facesdimtwo_.push_back(g);
            facesdimthree_.push_back(h);
        } else if (!line.compare(0, 3, "vn ")) {
            iss >> trash >> trash;
            Vec3f vn;
            for (int i=0;i<3;i++) iss >> vn[i];
            //je récupère les vecteurs normaux
            vertsnorm_.push_back(vn);
        }
    }
    std::cerr << "# v# " << verts_.size() << " f# "  << faces_.size() << std::endl;
}

Model::~Model() {
}

int Model::nverts() {
    return (int)verts_.size();
}

int Model::nvertsnorm() {
    return (int)vertsnorm_.size();
}

int Model::nfaces() {
    return (int)faces_.size();
}

std::vector<int> Model::face(int idx) {
    return faces_[idx];
}

Vec3f Model::vert(int i) {
    return verts_[i];

}
Vec2f Model::vertdimtwo(int i) {
    return vertsdimtwo_[i];

}

Vec3f Model::vertsnorm(int i) {
    return vertsnorm_[i];

}

std::vector<int> Model::facedimtwo(int idxdimtwo) {
    return facesdimtwo_[idxdimtwo];
}
std::vector<int> Model::facedimthree(int idxdimtwo) {
    return facesdimthree_[idxdimtwo];
}