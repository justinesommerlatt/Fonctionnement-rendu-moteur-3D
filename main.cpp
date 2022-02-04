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


const int width = 1024;
const int height = 1024;
const TGAColor green = TGAColor(0,   255, 0,   255);
Model *model = NULL;

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
* bp : point dont on calcule les coordonnÃ©es barycentriques
* s1p : s1' 
* s2p : s2'
* s3p : s3'
*/
Vec3f barycentric(Vec2f s1, Vec2f s2, Vec2f s3, Vec2f bp){
    double total_area = triangle_area_2d(s1, s2, s3);
    if (total_area<0) return {-1,0,0}; // backface culling
    double a = triangle_area_2d(bp, s2, s3);
    double b = triangle_area_2d(bp, s3, s1);
    double c = triangle_area_2d(bp, s1, s2);
    return {static_cast<float>(a/total_area),static_cast<float>(b/total_area),static_cast<float>(c/total_area)};
}

/*
* void triangle
* entrÃ©es
* s1 : premier sommet du triangle
* s2 : deuxiÃ¨me sommet du triangle
* s3 : troisiÃ¨me sommet du triangle
* &image : 
* color : couleur du triangle
*
void triangle(Vec2i s1, Vec2i s2, Vec2i s3, TGAImage &image, TGAColor color) {
    //si les trois coordonnÃ©es y sont Ã©gales alors il ne s'agit plus d'un triangle mais d'un segment
    if (s1.y==s2.y && s1.y==s3.y) return; 
    if (s1.y>s2.y) std::swap(s1, s2);
    if (s1.y>s3.y) std::swap(s1, s3);
    if (s2.y>s3.y) std::swap(s2, s3);

    //avec les swaps, on fait en sorte que le sommet s3 soit le sommet le plus "haut" et s1 le sommet le plus bas
    int total_height = s3.y-s1.y; 



    for (int i=0; i<total_height; i++) {
        // on coupe le triangle en deux sous triangles et on regarde si on est dans la deuxiÃ¨me moitiÃ© (=triangle supÃ©rieur)
        bool second_half = i>s2.y-s1.y || s2.y==s1.y;

        // puisque le sommet s3 est le plus haut et le sommet s1 le plus bas, 
        // la hauteur de notre sous triangle vaut soit s3.y-s2.y si on se trouve dans le triangle supÃ©rieur
        // soit s2.y-s1.y si on se trouve dans le triangle infÃ©rieur
        int segment_height = second_half ? s3.y-s2.y : s2.y-s1.y;

        // hauteur relative de i dans le triangle courant
        float alpha = (float)i/total_height;

        // si on se trouve dans le triangle supÃ©rieur alors on "supprime" le triangle infÃ©rieur (si on se trouve dans le triangle infÃ©rieur alors on n'enlÃ¨ve rien),
        // afin d'obtenir la hauteur de i relative dans le triangle courant
        float beta  = (float)(i-(second_half ? s2.y-s1.y : 0))/segment_height; 

        // s3-s1 : vecteur allant de s3 Ã  s1
        // alpha : hauteur relative de i 
        Vec2i A =               s1 + (s3-s1)*alpha;
        Vec2i B = second_half ? s2 + (s3-s2)*beta : s1 + (s2-s1)*beta;
          
        if (A.x>B.x) std::swap(A, B);
        for (int j=A.x; j<=B.x; j++) {
            
            Vec3f pt=barycentric(s1, s2, s3, Vec2i(j,i));
            if (pt.x <0 || pt.y < 0 || pt.z < 0){ 
                continue;
            }
            else{
                image.set(j, s1.y+i, color); 
            }
           
        }
    }
}*/
void triangle(std::array<Vec3f,3> pts, float *zbuffer, TGAImage &image, TGAColor color) {
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
* je souhaite calculer la "moyenne" des coordonnÃ©es z afin d'ensuite pouvoir les trier
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

    float *zbuffer = new float[width*height];
    for (int i=width*height; i--; zbuffer[i] = -std::numeric_limits<float>::max());

    TGAImage image(width, height, TGAImage::RGB);

    // un vecteur qui va contenir tous mes triangles
    std::vector<std::array<Vec3f, 3>> vec_triangles ;

    for (int i=0; i<model->nfaces(); i++) {
        std::vector<int> face = model->face(i);
        
        std::array<Vec3f,3> pts;
        for (int i=0; i<3; i++) pts[i] = world2screen(model->vert(face[i]));

        //je mets dans mon vecteur tous les triangles
        vec_triangles.push_back(pts);
        //triangle(pts, zbuffer, image, TGAColor(rand()%255, rand()%255, rand()%255, 255));
    }

    // je trie mes triangles dans mon vecteur en fonction de la moyenne de leurs z afin de les traiter du plus profond au plus avancÃ© et "superposer"
//  std::sort(vec_triangles.begin(), vec_triangles.end(), compareV);
    for (int i=0; i<vec_triangles.size(); i++){
         triangle(vec_triangles[i], zbuffer, image, TGAColor(rand()%255, rand()%255, rand()%255, 255));
    }

//  image.flip_vertically(); // i want to have the origin at the left bottom corner of the image
    image.write_tga_file("output.tga");
    delete model;
    return 0;
}