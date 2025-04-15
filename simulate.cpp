#include <GL/glut.h>
#include <GL/glu.h>
#include <iostream>
#include <math.h>
#include <fstream>
#include <windows.h>
#include <cmath>
#include <stdlib.h>
#include <bits/stdc++.h>
#include "correction00.hpp"

using namespace std;
const int WIDTH = 1000;
const int HEIGHT = 800;
int field_of_view =70;
bool circlearound = false;
float circleradius, circlephase;
float circlecenter[3];
int lastkeyboardcall=0;
float camposition[3]={0, 0.28, 0.245};
float observerposition[3];
float camrenderposition[3];
float camdirection[3]={0,-3,2};
float lightposition[3]={0.0, 0.11, 0.11};
float discposition[3]={0, 0, 0};
float passivemousecompensation[2]={0,0};
float maxstepsbefore=maxinternalsteps;

bool slomo=false;
bool freeze=false;
double timestep =0.00298598;
int colors=20;
//int testsign=1;
bool circleperspective=false;

bool fixobserver=false;









vector<Light> lights;
vector<Disc> cds;

void drawCentimeterGrid(float size = 0.5f) {

   
    glColor3f(0.8f, 0.8f, 0.8f); // Light gray grid

    glLineWidth(1.0f); // 1-pixel lines
    glBegin(GL_LINES);
    
    float spacing = 0.1f; // 1 cm
    int lines = static_cast<int>(size / spacing);
    
    for (int i = -lines; i <= lines; ++i) {
        float pos = i * spacing;

        // Lines parallel to X-axis
        glVertex3f(-size, 0.0f, pos);
        glVertex3f(size, 0.0f, pos);

        // Lines parallel to Z-axis
        glVertex3f(pos, 0.0f, -size);
        glVertex3f(pos, 0.0f, size);
    }

    glEnd();

}


void reshape(int w, int h) {
    glViewport(0, 0, w, h);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(field_of_view, (float)w / (float)h, 0.01*(1-slomo*0.995), 5.0);
    glutPostRedisplay();

}
void renderScene() {

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    glEnable(GL_DEPTH_TEST);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // Set up a perspective view
    if (circlearound) {

        camrenderposition[0]=-cos(circlephase)*circleradius+circlecenter[0];
        camrenderposition[1]=camposition[1];
        camrenderposition[2]=-sin(circlephase)*circleradius+circlecenter[2];

        if (!fixobserver){
            observerposition[0]=camrenderposition[0];
            observerposition[1]=camrenderposition[1];
            observerposition[2]=camrenderposition[2];
        }
        
        gluLookAt(camrenderposition[0], camrenderposition[1], camrenderposition[2], circlecenter[0], 0, circlecenter[2], 0.0, 1.0, 0.0);
        circlephase += PI * 2 * timestep*!freeze;
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        


    }
    else {

        if (!fixobserver){
            observerposition[0]=camposition[0];
            observerposition[1]=camposition[1];
            observerposition[2]=camposition[2];
        }
        gluLookAt(camposition[0], camposition[1], camposition[2], camposition[0] + camdirection[0], camposition[1] + camdirection[1], camposition[2] + camdirection[2], 0.0, 1.0, 0.0);
    }

    glEnable(GL_BLEND);
    glDepthMask(GL_FALSE);
    glBlendFunc(GL_ONE, GL_ONE);
        glColor3f(floorcolor[0],floorcolor[1],floorcolor[2]);
        glBegin(GL_TRIANGLES);
        glVertex3f(-100,0.0f,-100);
        glVertex3f(100,0.0f,-100);
        glVertex3f(-100,0.0f,100);

        glVertex3f(100,0.0f,100);
        glVertex3f(100,0.0f,-100);
        glVertex3f(-100,0.0f,100);
        glEnd();
        drawCentimeterGrid();
    glDisable(GL_BLEND);
    glDepthMask(GL_TRUE);

        for (Disc& cd:cds){
            for (Light& ls:lights){
            cd.draw(ls,observerposition,true);
            }
        }

       


    glFlush();
    if (circlearound)glutPostRedisplay();
    

}

void passivemouse(int x, int y){
    float cx=x+passivemousecompensation[0];
    if (cx> glutGet(GLUT_WINDOW_WIDTH)){
        passivemousecompensation[0] -= glutGet(GLUT_WINDOW_WIDTH);
        cx = x + passivemousecompensation[0];
    }else if(cx<0){
        passivemousecompensation[0] += glutGet(GLUT_WINDOW_WIDTH);
        cx = x + passivemousecompensation[0];
    }

    camdirection[0]=(tan((float(cx)/ glutGet(GLUT_WINDOW_WIDTH) *2-1)*PI));
    camdirection[1]=-(tan((float(y)/ glutGet(GLUT_WINDOW_HEIGHT) *2-1)*PI/2))* sqrt(pow(camdirection[0], 2) + 1);
    if (cx>3* glutGet(GLUT_WINDOW_WIDTH) /4 || cx < glutGet(GLUT_WINDOW_WIDTH) /4){
        camdirection[2]=1.0;
        camdirection[0]*=-1;
    }else{
    camdirection[2]=-1.0;
    }
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glutPostRedisplay();
}

void keyboard(unsigned char key, int x, int y){
    if (time(NULL)-lastkeyboardcall>=0){

    //PlaySound("step.wav", NULL, SND_ASYNC|SND_FILENAME);
    lastkeyboardcall=time(NULL);
    if (key=='w'){
        float sum = sqrt(pow(camdirection[0], 2) + 1);
        camposition[2] += camdirection[2] * 0.02*(1-slomo*0.98) / sum;
        camposition[0] += camdirection[0] * 0.02*(1-slomo*0.98) / sum;
        passivemousecompensation[0] += float(x - glutGet(GLUT_WINDOW_WIDTH) / 2) / 40;
    
        //std::cout << "camposition: " << camposition[0] << ", " << camposition[1] << ", " << camposition[2] << "\n";
        //std::cout << "passivemousecompensation[0]: " << passivemousecompensation[0] << "\n";
    
        passivemouse(x, y);
        glutPostRedisplay();
    }
    else if (key=='s'){
        float sum = sqrt(pow(camdirection[0], 2) + 1);
        camposition[2] -= camdirection[2] * 0.02*(1-slomo*0.98) / sum;
        camposition[0] -= camdirection[0] * 0.02*(1-slomo*0.98) / sum;
    
        //cout << "camposition: " << camposition[0] << ", " << camposition[1] << ", " << camposition[2] << "\n";
    
        glutPostRedisplay();
    }
    else if (key=='a'){
        float sum = sqrt(pow(camdirection[0], 2) + 1);
        camposition[2] -= camdirection[0] * 0.02*(1-slomo*0.98) / sum;
        camposition[0] += camdirection[2] * 0.02*(1-slomo*0.98) / sum;
    
        //cout << "camposition: " << camposition[0] << ", " << camposition[1] << ", " << camposition[2] << "\n";
    
        glutPostRedisplay();
    }
    else if (key=='d'){
        float sum = sqrt(pow(camdirection[0], 2) + 1);
        camposition[2] += camdirection[0] * 0.02*(1-slomo*0.98) / sum;
        camposition[0] -= camdirection[2] * 0.02*(1-slomo*0.98) / sum;
    
        //cout << "camposition: " << camposition[0] << ", " << camposition[1] << ", " << camposition[2] << "\n";
    
        glutPostRedisplay();
    }
    else if (key == 'x') {
        float factor = -camposition[1] / camdirection[1];
        circlecenter[0] = camposition[0] + camdirection[0] * factor;
        circlecenter[1] = 0;
        circlecenter[2] = camposition[2] + camdirection[2] * factor;
        circleradius = dist(camdirection[0] * factor, camdirection[2] * factor);
        circlephase = asin(camdirection[2] * factor / circleradius);
        if (camdirection[0] < 0) circlephase = PI - circlephase;


        circlearound = !circlearound;
        if (circlearound)freeze = false;
    
        //std::cout << "circlecenter: " << circlecenter[0] << ", " << circlecenter[1] << ", " << circlecenter[2] << "\n";
        //std::cout << "circleradius: " << circleradius << "\n";
        //std::cout << "circlephase: " << circlephase << "\n";
        //std::cout << "circlearound: " << circlearound << ", freeze: " << freeze << "\n";
    
        glutPostRedisplay();
    }
    else if (key == 'i') {
        field_of_view -= 1;
        std::cout << "field_of_view: " << field_of_view << "\n";
        reshape(glutGet(GLUT_WINDOW_WIDTH), glutGet(GLUT_WINDOW_HEIGHT));
    }
    else if (key == 'o') {
        field_of_view += 1;
        std::cout << "field_of_view: " << field_of_view << "\n";
        reshape(glutGet(GLUT_WINDOW_WIDTH), glutGet(GLUT_WINDOW_HEIGHT));
    }
    else if (key == 'p') {
        maxinternalsteps += 1;
        std::cout << "maxinternalsteps: " << maxinternalsteps << "\n";
        glutPostRedisplay();
    }
    else if (key == 'l') {
        if (maxinternalsteps > 0) {
            maxinternalsteps -= 1;
            std::cout << "maxinternalsteps: " << maxinternalsteps << "\n";
        } else std::cout << "stop\n";
        glutPostRedisplay();
    }
    else if (key == 'e') {
        camposition[0] = camrenderposition[0];
        camposition[1] = camrenderposition[1];
        camposition[2] = camrenderposition[2];
        camdirection[0] = circlecenter[0] - camrenderposition[0];
        camdirection[1] = -camrenderposition[1];
        camdirection[2] = circlecenter[2] - camrenderposition[2];
    
        std::cout << "camposition reset to render position\n";
        std::cout << "camdirection: " << camdirection[0] << ", " << camdirection[1] << ", " << camdirection[2] << "\n";
        std::cout << "camposition: " << camposition[0] << ", " << camposition[1] << ", " << camposition[2] << "\n";
    }
    else if (key == 'z') {
        raysize += 1;
        std::cout << "raysize: " << raysize << "\n";
        glutPostRedisplay();
    }
    else if (key == 'g') {
        if (raysize > 1) {
            raysize -= 1;
            std::cout << "raysize: " << raysize << "\n";
        } else std::cout << "stop\n";
        glutPostRedisplay();
    }
    else if (key == 'v') {
        simplelinesize += 1;
        std::cout << "simplelinesize: " << simplelinesize << "\n";
        glutPostRedisplay();
    }
    else if (key == 'c') {
        if (simplelinesize > 1) {
            simplelinesize -= 1;
            std::cout << "simplelinesize: " << simplelinesize << "\n";
        } else std::cout << "stop\n";
        glutPostRedisplay();
    }
    else if (key == 'j') {
        lightintensity /= 1.5;
        std::cout << "lightintensity: " << lightintensity << "\n";
        glutPostRedisplay();
    }
    else if (key == 'k') {
        lightintensity *= 1.5;
        std::cout << "lightintensity: " << lightintensity << "\n";
        glutPostRedisplay();
    }
    else if (key == '3') {
        //pathx--;
        //std::cout << "pathx: " << pathx << "\n";
        glutPostRedisplay();
    }
    else if (key == '4') {
        //pathx++;
        //std::cout << "pathx: " << pathx << "\n";
        glutPostRedisplay();
    }
    else if (key == '5') {
        //pathy--;
        //std::cout << "pathy: " << pathy << "\n";
        glutPostRedisplay();
    }
    else if (key == '6') {
        //pathy++;
        //std::cout << "pathy: " << pathy << "\n";
        glutPostRedisplay();
    }
    else if (key == 'q') {
        camposition[1] += 0.01*(1-slomo*0.98);
        std::cout << "camposition[1]: " << camposition[1] << "\n";
        glutPostRedisplay();
    }
    else if (key == 'y') {
        camposition[1] -= 0.01*(1-slomo*0.98);;
        std::cout << "camposition[1]: " << camposition[1] << "\n";
        glutPostRedisplay();
    }
    else if (key == 'u') {
        timestep *= 1.2;
        std::cout << "timestep: " << timestep << "\n";
    }
    else if (key == 'h') {
        timestep /= 1.2;
        std::cout << "timestep: " << timestep << "\n";
    }
    else if (key == 't') {
        internalpossampnum += 10;
        std::cout << "internalpossampnum: " << internalpossampnum << "\n";
        glutPostRedisplay();
    }
    else if (key == 'f') {
        internalpossampnum -= 10;
        std::cout << "internalpossampnum: " << internalpossampnum << "\n";
        glutPostRedisplay();
    }
    else if (key == 'm') {
        maxinternalraydeviation *= 1.2;
        std::cout << "maxinternalraydeviation: " << maxinternalraydeviation << "\n";
        glutPostRedisplay();
    }
    else if (key == 'n') {
        maxinternalraydeviation /= 1.2;
        std::cout << "maxinternalraydeviation: " << maxinternalraydeviation << "\n";
        glutPostRedisplay();
    }
    else if (key == 'r') {
        freeze = !freeze;
        std::cout << "freeze: " << freeze <<"\n";
        reshape(glutGet(GLUT_WINDOW_WIDTH), glutGet(GLUT_WINDOW_HEIGHT));
        glutPostRedisplay();

    }
    else if (key == '7') {
        fixobserver=!fixobserver;
        std::cout << "fixobserver: " << fixobserver <<"\n";
        glutPostRedisplay();

    }
    else if (key == '8') {
        //internaldiffract=!internaldiffract;
        //std::cout << "internaldiffract: " << internaldiffract <<"\n";
        glutPostRedisplay();

    }
    else if (key == '9') {
        slomo=!slomo;
        std::cout << "slomo: " << slomo <<"\n";
        glutPostRedisplay();

    }
    
    else if (key == '1') {
        if (colors>2)colors--;
        internallambdachecks.clear();
        for (int i=0;i<colors;i++){
            internallambdachecks.push_back((380+370.0/(colors-1)*i)*1e-9);
        }
        std::cout << "colors: " << colors << "\n";
        glutPostRedisplay();
    }
    else if (key == '2') {
        colors++;
        internallambdachecks.clear();
        for (int i=0;i<colors;i++){
            internallambdachecks.push_back((380+370.0/(colors-1)*i)*1e-9);
        }
        std::cout << "colors: " << colors << "\n";
        glutPostRedisplay();
    }
   
    else if (key == 'b') {
        if (line) {
            timestep *= 20;
            line = false;
            maxinternalsteps = maxstepsbefore;
        } else {
            timestep /= 20;
            line = true;
            maxstepsbefore = maxinternalsteps;
            maxinternalsteps = 0;
        }
        std::cout << "line: " << line << ", timestep: " << timestep << ", maxinternalsteps: " << maxinternalsteps << "\n";
        glutPostRedisplay();
    }
    
    else{
        std::cout<<"Controls:\n"
                       "W/A/S/D = move forward/left/back/right\n"
                       "W/A/S/D = move forward/left/back/right\n"
                       "Q/Y = move up/down\n"
                       "x = toggle circular motion + set center\n"
                       "I/O = decrease/increase field of view\n"
                       "P/L = more/fewer internal ray steps\n"
                       "J/K = decrease/increase light intensity\n"
                       "N/M = decrease/increase light size\n"
                       "G/Z = decrease/increase point ray size\n"
                       "C/V = decrease/increase simple line size\n"
                       "B = toggle simple line\n"
                       "R = toggle freeze\n"
                       "E = set circle pos to pos\n"
                       "F/T = decrease/increase point sample num\n"
                       "H/U = decrease/increase timestep\n";

    }}}




int main(int argc, char** argv) {

    srand(time(NULL));


    lights.push_back(Light(lightposition));
    for (int i=0;i<1;i++){
        float discpos[3]={-0.06f+i%2*0.12f,0,0.06f+i*0.06f};
        cds.push_back(Disc(discposition));
    }
    for (int i=0;i<colors;i++){
        internallambdachecks.push_back((380+370.0/(colors-1)*i)*1e-9);
    }
    
    





    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_SINGLE | GLUT_RGB | GLUT_DEPTH);
    glutInitWindowSize(WIDTH, HEIGHT);
    glutCreateWindow("Cube with Shadow");
    //glEnable(GL_DEPTH_TEST);

    glutDisplayFunc(renderScene);
    glutReshapeFunc(reshape);
    glutKeyboardFunc(keyboard);
    glutPassiveMotionFunc(passivemouse);

    glutMainLoop();
    return 0;
}
