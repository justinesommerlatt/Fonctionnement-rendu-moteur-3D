#include "tgaimage.h"
#include "matrix.h"
#include "geometry.h"
#include "model.h"
#include <array>
#include <iostream>
#include <fstream>
#include <sstream>
#include <limits>
#include <vector>
#include <cmath>


const int width  = 800;
const int height = 800;
const TGAColor green = TGAColor(0,   255, 0,   255);
Model *model = NULL;
TGAImage tex;
int *zbuffer = NULL;
Vec3f light_dir(0,0,-1);



void line(int x0, int y0, int x1, int y1, TGAImage &img)
{
    bool steep = std::abs(x0 - x1) < std::abs(y0 - y1);
    if (steep)
    {
        std::swap(x0, y0);
        std::swap(x1, y1);
    }
    if (x1 < x0)
    {
        std::swap(x0, x1);
        std::swap(y0, y1);
    }
    int dx = x1-x0;
    int dy = y1-y0;
    int derror = 2*std::abs(dy);
    int error = 0;
    int y = y0;
    for (int x = x0; x < x1; x++)
    {
        if (steep)
        {
            img.set(y, x, TGAColor(255, 255, 255));
        }
        else
        {
            img.set(x, y, TGAColor(255, 255, 255));
        }
        error += derror;
        if (error > dx){
            y += y1 > y0 ? 1 : -1;
            error -= 2*dx;
        }
    }
}

/*
* Le barycentre d'un triangle est en fait son centre de gravite
* Pour calculer le barycentre d'un triangle il existe plusieurs methodes : 
* - intersection des medianes
* - ratio 2:1
* - moyenne des coordonnees
* Dans ce cas present, calculer la moyenne des coordonnÃ©es se revele etre bien plus simple  
*/
Vec2i barycentre(Vec2i s1, Vec2i s2, Vec2i s3){
    float x_center = (s1.x + s2.x + s3.x)/3;
    float y_center = (s1.y + s2.y + s3.y)/3;
    //float z_center = (s1.z + s2.z + s3.z)/3;
    return Vec2i(x_center, y_center);

}

/*
Calcul de l'aire d'un triangle
*/

double triangle_area_2d(Vec2f a, Vec2f b, Vec2f c) {
    return .5*((b.y-a.y)*(b.x+a.x) + (c.y-b.y)*(c.x+b.x) + (a.y-c.y)*(a.x+c.x));
}

/*
* Calculer les coordonnees barycentriques 
* Pour cela, il nous faut calculer 4 aires de triangles (le grand triangles ainsi que les trois sous triangles qui le forment)
* Puisque la somme des coordonees vaut 1, il nous suffit de calculer deux coordonnees pour connaitre la troisieme, soit : 
* c' = 1 - a' - b'
* Aire d'un triangle = H*b /2
* Aire d'un triangles Ã  l'aide des coordonnÃ©es = 1/2 (s1.x(s2.Y - s3.y) + s2.x(s3.y - s1;Y) + s3.x(s1.y - s2.y))
* s1 : sommet 1
* s2 : sommet 2
* s3 : sommet 3
* bp : point dont on calcule les coordonnees barycentriques
*/
Vec3f barycentric(Vec2f s1, Vec2f s2, Vec2f s3, Vec2f bp){
    double total_area = triangle_area_2d(s1, s2, s3);
    if (total_area<0) return {-1,0,0}; // backface culling
    double a = triangle_area_2d(bp, s2, s3);
    double b = triangle_area_2d(bp, s3, s1);
    double c = triangle_area_2d(bp, s1, s2);
    return {static_cast<float>(a/total_area),static_cast<float>(b/total_area),static_cast<float>(c/total_area)};
}

void triangle(std::array<Vec3f,3> pts, std::array<Vec2f,3> pts2, float *zbuffer, TGAImage &image, TGAColor color) {
    Vec2f bboxmin( std::numeric_limits<float>::max(),  std::numeric_limits<float>::max());
    Vec2f bboxmax(-std::numeric_limits<float>::max(), -std::numeric_limits<float>::max());
    Vec2f clamp(image.get_width()-1, image.get_height()-1);
    for (int i=0; i<3; i++) {
        for (int j=0; j<2; j++) {
            bboxmin[j] = std::max(0.f,      std::min(bboxmin[j], pts[i][j]));
            bboxmax[j] = std::min(clamp[j], std::max(bboxmax[j], pts[i][j]));
        }
    }
    for (double x=bboxmin.x; x<=bboxmax.x; x+=1) {
        for (double y=bboxmin.y; y<=bboxmax.y; y+=1) {
            Vec3f bc_screen  = barycentric(Vec2f{pts[0].x, pts[0].y}, Vec2f{pts[1].x, pts[1].y}, Vec2f{pts[2].x, pts[2].y}, Vec2f{static_cast<float>(x), static_cast<float>(y)});
            if (bc_screen.x<0 || bc_screen.y<0 || bc_screen.z<0) continue;
            double z = pts[0].z * bc_screen[0] + pts[1].z * bc_screen[1] + pts[2].z * bc_screen[2];
            Vec2f uv = pts2[0] * bc_screen[0] + pts2[1] * bc_screen[1] + pts2[2] * bc_screen[2];
      //      std::cerr << pts2[0] << std::endl;
            color = tex.get(uv.x*tex.get_width(), uv.y*tex.get_height());
            if (zbuffer[int(x+y*width)]>z) continue;
            zbuffer[int(x+y*width)] = z;
            image.set(x, y, color);
        }
    }
}


Vec3f world2screen(Vec3f v) {
    return Vec3f(int((v.x+1.)*width/2.+.5), int((v.y+1.)*height/2.+.5), v.z);
}

/* compareZ
* je souhaite calculer la "moyenne" des coordonnees z afin d'ensuite pouvoir les trier
*/ 
float compareZ(Vec3f vect[]){
    return (vect[0].z + vect[1].z + vect[2].z)/3;
}

bool compareV(Vec3f v1[], Vec3f v2[]){
  return (v1[0].z + v1[1].z + v1[2].z)/3 > (v2[0].z + v2[1].z + v2[2].z)/3;
}

int main(int argc, char** argv) {
    if (2==argc) {
        model = new Model(argv[1]);
    } else {
        model = new Model("obj/african_head/african_head.obj");
    }
    //je lis ma texture afin de l'appliquer sur mon objet
    tex.read_tga_file("../obj/african_head/african_head_diffuse.tga");
    //je retourne verticalement ma texture (en + de mon objet)
    tex.flip_vertically();
    

    float *zbuffer = new float[width*height];
    for (int i=width*height; i--; zbuffer[i] = -std::numeric_limits<float>::max());

    TGAImage image(width, height, TGAImage::RGB);

    // un vecteur qui va contenir tous mes triangles
    std::vector<std::array<Vec3f, 3>> vec_triangles ;

    for (int i=0; i<model->nfaces(); i++) {
        std::vector<int> face = model->face(i);
        //distance de la caméra
        float c = 1/5.2;
        
        std::array<Vec3f,3> pts;
        for (int i=0; i<3; i++) {
            Vec3f v = model->vert(face[i]);
            //formule 3 fin de la leçon 4
            v = Vec3f{v.x/(1.f-v.z*c), v.y/(1.f-v.z*c), v.z/(1.f-v.z*c)};
            pts[i] = world2screen(v);
        }
        //je mets dans mon vecteur tous les triangles
        vec_triangles.push_back(pts);
    }

    
    for (int i=0; i<vec_triangles.size(); i++){
      std::array<Vec2f, 3> uvs = {model->vertdimtwo(model->facedimtwo(i)[0]), model->vertdimtwo(model->facedimtwo(i)[1]), model->vertdimtwo(model->facedimtwo(i)[2])};
      
         triangle(vec_triangles[i], uvs, zbuffer, image, TGAColor(rand()%255, rand()%255, rand()%255, 255));
    }

    //je tourne l'image verticalement 
    image.flip_vertically(); 

    //Fonction qui dessine l'image
    image.write_tga_file("output.tga");
    delete model;
    return 0;
}