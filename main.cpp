#include "tgaimage.h"
#include "matrix.h"
#include "geometry.h"
#include "model.h"
#include <iostream>
#include <fstream>
#include <sstream>
#include <limits>
#include <vector>
#include <cmath>


constexpr int width = 1024;
constexpr int height = 1024;
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
* Le barycentre d'un triangle est en fait son centre de gravité
* Pour calculer le barycentre d'un triangle il existe plusieurs méthodes : 
* - intersection des médianes
* - ratio 2:1
* - moyenne des coordonnées
* Dans ce cas présent, calculer la moyenne des coordonnées se révèle être bien plus simple  
*/
Vec3f barycentre(Vec3f s1, Vec3f s2, Vec3f s3){
    float x_center = (s1.x + s2.x + s3.x)/3;
    float y_center = (s1.y + s2.y + s3.y)/3;
    float z_center = (s1.z + s2.z + s3.z)/3;
    return Vec3f(x_center, y_center, z_center);

}


/*
* void triangle
* entrées
* s1 : premier sommet du triangle
* s2 : deuxième sommet du triangle
* s3 : troisième sommet du triangle
* &image : 
* color : couleur du triangle
*/
void triangle(Vec2i s1, Vec2i s2, Vec2i s3, TGAImage &image, TGAColor color) {
    //si les trois coordonnées y sont égales alors il ne s'agit plus d'un triangle mais d'un segment
    if (s1.y==s2.y && s1.y==s3.y) return; 
    if (s1.y>s2.y) std::swap(s1, s2);
    if (s1.y>s3.y) std::swap(s1, s3);
    if (s2.y>s3.y) std::swap(s2, s3);

    //avec les swaps, on fait en sorte que le sommet s3 soit le sommet le plus "haut" et s1 le sommet le plus bas
    int total_height = s3.y-s1.y; 

    
    for (int i=0; i<total_height; i++) {
        // on coupe le triangle en deux sous triangles et on regarde si on est dans la deuxième moitié (=triangle supérieur)
        bool second_half = i>s2.y-s1.y || s2.y==s1.y;

        // puisque le sommet s3 est le plus haut et le sommet s1 le plus bas, 
        // la hauteur de notre sous triangle vaut soit s3.y-s2.y si on se trouve dans le triangle supérieur
        // soit s2.y-s1.y si on se trouve dans le triangle inférieur
        int segment_height = second_half ? s3.y-s2.y : s2.y-s1.y;

        // hauteur relative de i dans le triangle courant
        float alpha = (float)i/total_height;

        // si on se trouve dans le triangle supérieur alors on "supprime" le triangle inférieur (si on se trouve dans le triangle inférieur alors on n'enlève rien),
        // afin d'obtenir la hauteur de i relative dans le triangle courant
        float beta  = (float)(i-(second_half ? s2.y-s1.y : 0))/segment_height; 

        // s3-s1 : vecteur allantr de s3 à s1
        // alpha : hauteur relative de i 
        Vec2i A =               s1 + (s3-s1)*alpha;
        Vec2i B = second_half ? s2 + (s3-s2)*beta : s1 + (s2-s1)*beta;
          
        if (A.x>B.x) std::swap(A, B);
        for (int j=A.x; j<=B.x; j++) {
            image.set(j, s1.y+i, color); 
        }
    }
}



int main([[maybe_unused]]int argc, [[maybe_unused]]char const *argv[])
{
     if (2==argc) {
        model = new Model(argv[1]);
    } else {
        //dans les lignes de commande si je mets uniquement ./tinyrenderer, on appelle quand même african_head comme objet
        model = new Model("obj/african_head.obj");
    }


    TGAImage image(width, height, TGAImage::RGB);
    Vec3f light_dir(0,0,-1);
    for (int i=0; i<model->nfaces(); i++) {
        std::vector<int> face = model->face(i);
        Vec2i screen_coords[3];
        Vec3f world_coords[3];
        for (int j=0; j<3; j++) {
            Vec3f v = model->vert(face[j]);
            screen_coords[j] = Vec2i((v.x+1.)*width/2., (v.y+1.)*height/2.);
            world_coords[j]  = v;
        }
        Vec3f n = (world_coords[2]-world_coords[0])^(world_coords[1]-world_coords[0]);
        n.normalize();
        float intensity = n*light_dir;
        if (intensity>0) {
            //pour avoir le portrait d'une même couleur
            triangle(screen_coords[0], screen_coords[1], screen_coords[2], image, TGAColor(intensity*55, intensity*89, intensity*99, 255));
            /*pour mettre la texture sur le visage, il va falloir interpoler les coordonnées 
            de texture des sommets des triangles, les multiplier par largeur-hauteur de l'image de texture
            ce qui nous donnera la couleur à retourner 
            */
            triangle(screen_coords[0], screen_coords[1], screen_coords[2], image, TGAColor(intensity*55, intensity*89, intensity*99, 255));

        }
    }

    //pour avoir l'origine en bas à gauche MAIS retourne l'image
    //image.flip_vertically(); 

    image.write_tga_file("output.tga");
    delete model;

    /*
    if (argc < 2) {
        std::cerr << "Usage " << argv[0] << " model.obj" << std::endl;
    }
    std::vector<float> pts;
    std::vector<int> tri;
    {
        std::ifstream in;
        in.open(argv[1], std::ifstream::in);
        if (in.fail()) {
            std::cerr << "Failed to open " << argv[1] << std::endl;
            return -1;
        }
        std::string line;
        char trash;
        while (!in.eof()) {
            std::getline(in, line);
            std::istringstream iss(line.c_str());
            if (!line.compare(0, 2, "v ")) {
                iss >> trash;
                for (int i : {0, 1, 2}) {
                    float v;
                    iss >> v;
                    pts.push_back(v);
                }
            }
            if (!line.compare(0, 2, "f ")) {
                iss >> trash;
                int f, t, n;
                while (iss >> f >> trash >> t >> trash >> n) {
                    tri.push_back(f-1);
                }
            }
        }
        in.close();
    }

    int nverts = pts.size() / 3;
    int ntri = tri.size() / 3;
    
    TGAImage framebuffer(width, height, TGAImage::RGB);

    //Triangle
    //triangle(vec2d(25, 25), vec2d(50, 175), vec2d(350, 854), framebuffer);

    /*for (int i = 0; i < ntri; i++)
    {
        for (int s : {0, 1, 2}) {
            int aPos = tri[i * 3 + s];
            int bPos = tri[i * 3 + (s + 1) % 3];
            int cPos = tri[i * 3 + (s + 2) % 3];
            vec2d a((tri[aPos * 3 + 0] + 1) / 2 * width, (tri[aPos * 3 + 1] + 1) / 2 * height);
            vec2d b((tri[bPos * 3 + 0] + 1) / 2 * width, (tri[bPos * 3 + 1] + 1) / 2 * height);
            vec2d c((tri[cPos * 3 + 0] + 1) / 2 * width, (tri[cPos * 3 + 1] + 1) / 2 * height);
            triangle(a, b, c, framebuffer);
        }
    }*/


    /*CODE POUR DESSINER EN FILS DE FER
    for (int t = 0; t < ntri; t++)
    {
        for (int s : { 0,1,2 })
        {
            int u = tri[t * 3 + s];
            int v = tri[t * 3 + (s + 1) % 3];
            int x0 = (pts[u * 3 + 0] + 1) / 2 * width;
            int y0 = (pts[u * 3 + 1] + 1) / 2 * height;
            int x1 = (pts[v * 3 + 0] + 1) / 2 * width;
            int y1 = (pts[v * 3 + 1] + 1) / 2 * height;
            //tracer une ligne : 
            line(x0, y0, x1, y1, framebuffer);
        }
    }
    //FIN CODE POUR DESSINER EN FILS DE FER
    
    framebuffer.write_tga_file("framebuffer.tga");*/
    return 0;
}