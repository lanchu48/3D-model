#include <GL/glew.h>
#include <GL/freeglut.h>
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <iostream>

// Biến toàn cục cho mô hình
const aiScene* scene = nullptr;
Assimp::Importer importer;
GLuint displayList = 0;

// Camera
float angleX = 0.0f, angleY = 0.0f; // Góc xoay
float zoom = 1.0f; // Zoom

// Khai báo hàm thiết lập ánh sáng
void initLighting() {
    glEnable(GL_LIGHTING);
    glEnable(GL_LIGHT0); // Sử dụng nguồn sáng 0

    glEnable(GL_NORMALIZE); // Chuẩn hóa các vector pháp tuyến

    // Thiết lập ánh sáng
    GLfloat lightPosition[] = { 5.0f, 5.0f, 10.0f, 1.0f }; // Vị trí  nguồn sáng
    GLfloat lightAmbient[] = { 0.8f, 0.8f, 0.8f, 1.0f };   // AS môi trường 
    GLfloat lightDiffuse[] = { 0.8f, 0.8f, 0.8f, 1.0f };   //  AS khuếch tán
    GLfloat lightSpecular[] = { 1.0f, 1.0f, 1.0f, 1.0f };  //  AS phản chiếu

    glLightfv(GL_LIGHT0, GL_POSITION, lightPosition);
    glLightfv(GL_LIGHT0, GL_AMBIENT, lightAmbient);
    glLightfv(GL_LIGHT0, GL_DIFFUSE, lightDiffuse);
    glLightfv(GL_LIGHT0, GL_SPECULAR, lightSpecular);



}

void applyMaterial(aiMaterial* material) {
    aiColor4D ambient, diffuse, specular;

    // Lấy màu ambient (xung quanh)
    if (AI_SUCCESS == aiGetMaterialColor(material, AI_MATKEY_COLOR_AMBIENT, &ambient)) {
        // Kiểm tra nếu Ka quá thấp (bằng 0 hoặc gần 0)
        if (ambient.r <= 0.01f && ambient.g <= 0.01f && ambient.b <= 0.01f) {
            GLfloat matAmbient[] = { 0.3f, 0.3f, 0.3f, 1.0f }; // Giá trị mặc định
            glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, matAmbient);
        }
        else {
            GLfloat matAmbient[] = { ambient.r, ambient.g, ambient.b, ambient.a };
            glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, matAmbient);
        }
    }
    else {
        // Nếu không tìm thấy giá trị Ka, sử dụng giá trị mặc định
        GLfloat matAmbient[] = { 0.3f, 0.3f, 0.3f, 1.0f };
        glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, matAmbient);
    }

    // Lấy màu diffuse (khuếch tán)
    if (AI_SUCCESS == aiGetMaterialColor(material, AI_MATKEY_COLOR_DIFFUSE, &diffuse)) {
        GLfloat matDiffuse[] = { diffuse.r, diffuse.g, diffuse.b, diffuse.a };
        glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, matDiffuse);
    }

    // Lấy màu specular (phản chiếu)
    if (AI_SUCCESS == aiGetMaterialColor(material, AI_MATKEY_COLOR_SPECULAR, &specular)) {
        GLfloat matSpecular[] = { specular.r, specular.g, specular.b, specular.a };
        glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, matSpecular);
    }

    // Đặt độ bóng (shininess)
    float shininess = 0.0f;
    if (AI_SUCCESS == aiGetMaterialFloat(material, AI_MATKEY_SHININESS, &shininess)) {
        glMaterialf(GL_FRONT_AND_BACK, GL_SHININESS, shininess);
    }
}

void loadModel(const std::string& path) {
    scene = importer.ReadFile(path, aiProcess_Triangulate | aiProcess_JoinIdenticalVertices);
    if (!scene || !scene->mRootNode || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE) {
        std::cerr << "Lỗi: " << importer.GetErrorString() << std::endl;
        exit(EXIT_FAILURE);
    }

    // Tạo display list
    displayList = glGenLists(1);
    glNewList(displayList, GL_COMPILE);
    for (unsigned int i = 0; i < scene->mNumMeshes; i++) {
        aiMesh* mesh = scene->mMeshes[i];
        aiMaterial* material = scene->mMaterials[mesh->mMaterialIndex];

        // Áp dụng vật liệu
        applyMaterial(material);

        // Vẽ mesh
        for (unsigned int f = 0; f < mesh->mNumFaces; f++) {
            aiFace face = mesh->mFaces[f];
            glBegin(GL_TRIANGLES);
            for (unsigned int v = 0; v < face.mNumIndices; v++) {
                unsigned int vertexIndex = face.mIndices[v];

                // Đặt pháp tuyến (normals)
                if (mesh->HasNormals()) {
                    aiVector3D normal = mesh->mNormals[vertexIndex];
                    glNormal3f(normal.x, normal.y, normal.z);
                }

                // Đặt toạ độ đỉnh
                aiVector3D vertex = mesh->mVertices[vertexIndex];
                glVertex3f(vertex.x, vertex.y, vertex.z);
            }
            glEnd();
        }
    }
    glEndList();
}

void display() {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glLoadIdentity();

    // Đặt camera
    glTranslatef(0.0f, 0.0f, -10.0f); // Lùi camera ra xa
    glScalef(zoom, zoom, zoom);
    glRotatef(angleX, 1.0f, 0.0f, 0.0f);
    glRotatef(angleY, 0.0f, 1.0f, 0.0f);

    // Vẽ mô hình
    if (displayList) {
        glCallList(displayList);
    }

    glutSwapBuffers();
}

void reshape(int width, int height) {
    glViewport(0, 0, width, height);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(45.0, (double)width / (double)height, 0.1, 100.0);
    glMatrixMode(GL_MODELVIEW);
}
// Dùng z để zoom in, x để zoom out
void keyboard(unsigned char key, int x, int y) {
    if (key == 'z') zoom += 0.1f;
    if (key == 'x') zoom -= 0.1f;
    glutPostRedisplay();
}
// Dùng phím mũi tên để xoay 
void specialKeys(int key, int x, int y) {
    if (key == GLUT_KEY_UP) angleX -= 5.0f;
    if (key == GLUT_KEY_DOWN) angleX += 5.0f;
    if (key == GLUT_KEY_LEFT) angleY -= 5.0f;
    if (key == GLUT_KEY_RIGHT) angleY += 5.0f;
    glutPostRedisplay();
}

int main(int argc, char** argv) {
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
    glutInitWindowSize(800, 600);
    glutCreateWindow("Load Model with Assimp");
    glClearColor(0.5f, 0.7f, 1.0f, 1.0f); // Màu nền bầu trời

    glEnable(GL_DEPTH_TEST);
    initLighting();

    // Load mô hình (thay bằng đường dẫn file .obj)
    loadModel("E:/C++/VM/OBJ/VanMieu.obj");
    glutDisplayFunc(display);
    glutReshapeFunc(reshape);
    glutKeyboardFunc(keyboard);
    glutSpecialFunc(specialKeys);

    glutMainLoop();
    return 0;
}
