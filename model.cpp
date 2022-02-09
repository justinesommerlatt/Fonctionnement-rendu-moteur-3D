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
            std::vector<int> f;
            std::vector<int> g;
            int itrash, idx, idxdimtwo;
            iss >> trash;
            while (iss >> idx >> trash >> idxdimtwo >> trash >> itrash) {
                idx--; // tous les indices commencent à 1
                idxdimtwo--;
                f.push_back(idx);
                g.push_back(idxdimtwo);
            }
            faces_.push_back(f);
            facesdimtwo_.push_back(g);
        }
    }
    std::cerr << "# v# " << verts_.size() << " f# "  << faces_.size() << std::endl;
}

Model::~Model() {
}

int Model::nverts() {
    return (int)verts_.size();
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
std::vector<int> Model::facedimtwo(int idxdimtwo) {
    return facesdimtwo_[idxdimtwo];
}