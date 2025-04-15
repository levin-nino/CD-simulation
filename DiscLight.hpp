#include <bits/stdc++.h>
#include <tuple>
using namespace std;

int linesamplenum=100;
int maxinternalsteps=1;
const float PI = 3.14159265358979323;
vector<float> internallambdachecks={};
float maxinternalraydeviation=0.0120563;
int internalpossampnum=60;
bool line=false;
float floorcolor[3]={0.6,0.4,0.2};
float cdcolor[3]={0.22,0.2,0.21};
float lightintensity=0.0685095;
int raysize=7;
int simplelinesize=4;
int pathx=20;
int pathy=20;
bool internaldiffract=true;




std::tuple<int, int, int> wavelengthToRGB(double wavelength) {
    const double gamma = 0.8;
    double R = 0.0, G = 0.0, B = 0.0;
    double factor = 0.0;

    if (wavelength >= 380.0 && wavelength < 440.0) {
        R = -(wavelength - 440.0) / (440.0 - 380.0);
        G = 0.0;
        B = 1.0;
    } else if (wavelength >= 440.0 && wavelength < 490.0) {
        R = 0.0;
        G = (wavelength - 440.0) / (490.0 - 440.0);
        B = 1.0;
    } else if (wavelength >= 490.0 && wavelength < 510.0) {
        R = 0.0;
        G = 1.0;
        B = -(wavelength - 510.0) / (510.0 - 490.0);
    } else if (wavelength >= 510.0 && wavelength < 580.0) {
        R = (wavelength - 510.0) / (580.0 - 510.0);
        G = 1.0;
        B = 0.0;
    } else if (wavelength >= 580.0 && wavelength < 645.0) {
        R = 1.0;
        G = -(wavelength - 645.0) / (645.0 - 580.0);
        B = 0.0;
    } else if (wavelength >= 645.0 && wavelength <= 750.0) {
        R = 1.0;
        G = 0.0;
        B = 0.0;
    } else {
        // Wavelength outside visible range
        return std::make_tuple(0, 0, 0);
    }

    // Intensity factor
    if (wavelength >= 380.0 && wavelength < 420.0) {
        factor = 0.3 + 0.7 * (wavelength - 380.0) / (420.0 - 380.0);
    } else if (wavelength >= 420.0 && wavelength < 645.0) {
        factor = 1.0;
    } else if (wavelength >= 645.0 && wavelength <= 750.0) {
        factor = 0.3 + 0.7 * (750.0 - wavelength) / (750.0 - 645.0);
    }

    // Gamma correction and scaling
    auto correct = [&](double c) -> int {
        if (c <= 0.0) return 0;
        return std::clamp(static_cast<int>(std::round(255.0 * std::pow(c * factor, gamma))), 0, 255);
    };

    return std::make_tuple(correct(R), correct(G), correct(B));
}




struct Light{
    float pos[3];
    float rescolor[3];
    function<float(float)> spectralcomp;
    Light(float* lightpos, function<float(float)> spectralcomparg=[](float x){return 1;}):spectralcomp(move(spectralcomparg)){
        pos[0]=lightpos[0];pos[1]=lightpos[1];pos[2]=lightpos[2];
        rescolor[0]=1;
        rescolor[1]=1;
        rescolor[2]=1;
    }

    void draw(){
        glColor3f(rescolor[0],rescolor[1],rescolor[2]);
        glPointSize(maxinternalraydeviation*1000);
        glBegin(GL_POINTS);
        glVertex3f(pos[0],pos[1],pos[2]);
        glEnd();
    }
};

pair<float,float> getreflectionpoint(float* campos, float* lightpos){// reflection point on xy plane
    float tval=campos[1]/(campos[1]+lightpos[1]);
    pair<float,float> reflectp={tval*(lightpos[0]-campos[0])+campos[0], tval*(lightpos[2]-campos[2])+campos[2]};
    return reflectp;
}


float xdif(float* a, float* b){
    return b[0]-a[0];
}
float ydif(float* a, float* b){
    return b[2]-a[2];
}
float zdif(float* a, float* b){
    return b[1]-a[1];
}

float dist(float a, float b){
    return sqrt(a*a+b*b);
}




struct Disc{

    float d=1.6e-6;
    float pos[3];
    float thickness=0.00115; //1mm
    float inner,outer;
    float* currentinternalreflectiontarget;
    bool drawpaths=false;
    vector<array<float,6>> upline;
    vector<array<float,6>> downline;
    vector<array<float,6>> internalupline;
    vector<array<float,6>> incomeline;
    vector<array<float,6>> outgoingline;
    Disc(float* discpos, float innerrad=0.02, float outerrad=0.06):inner(innerrad),outer(outerrad){
        pos[0]=discpos[0];pos[1]=discpos[1];pos[2]=discpos[2];
    }

    void renderDiscWithHole(int slices = 20) {
        
        
        /*glColor3f(cdcolor[0],cdcolor[1],cdcolor[2]);
        glBegin(GL_TRIANGLES);
        for (int i=0;i<slices;i++){
            glVertex3f(pos[0],pos[1]+thickness,pos[2]);
            glVertex3f(outer*cos(i*PI*2/slices)+pos[0],pos[1]+thickness,outer*sin(i*PI*2/slices)+pos[2]);
            glVertex3f(outer*cos((i+1)*PI*2/slices)+pos[0],pos[1]+thickness,outer*sin((i+1)*PI*2/slices)+pos[2]);
        }
        glEnd();*/

        glColor3f(cdcolor[0],cdcolor[1],cdcolor[2]);
        glBegin(GL_TRIANGLES);
        for (int i=0;i<slices;i++){
            glVertex3f(pos[0],pos[1],pos[2]);
            glVertex3f(outer*cos(i*PI*2/slices)+pos[0],pos[1],outer*sin(i*PI*2/slices)+pos[2]);
            glVertex3f(outer*cos((i+1)*PI*2/slices)+pos[0],pos[1],outer*sin((i+1)*PI*2/slices)+pos[2]);
        }
        glEnd();

        glColor3f(floorcolor[0],floorcolor[1],floorcolor[2]);
        glBegin(GL_TRIANGLES);
        for (int i=0;i<slices;i++){
            glVertex3f(pos[0],pos[1]+thickness+0.0001,pos[2]);
            glVertex3f(inner*cos(i*PI*2/slices)+pos[0],pos[1]+thickness+0.0001,inner*sin(i*PI*2/slices)+pos[2]);
            glVertex3f(inner*cos((i+1)*PI*2/slices)+pos[0],pos[1]+thickness+0.0001,inner*sin((i+1)*PI*2/slices)+pos[2]);
        }
        glEnd();
    }
    


    void calclinecol(float* campos, float* lightpos){//positions relative to cd
        float angle=atan((campos[0]*lightpos[1]+campos[1]*lightpos[0])/(campos[1]*lightpos[2]+campos[2]*lightpos[1]));
        float T[2][2] = {{sin(angle),cos(angle)}, {cos(angle), -sin(angle)}}; //custom->norm
        float det=T[0][0]*T[1][1]-T[1][0]*T[0][1];
        float T_inv[2][2]= {{-sin(angle)/det,-cos(angle)/det},{-cos(angle)/det,sin(angle)/det}}; //norm->custom
        
        float customlight[2]={T_inv[0][0]*lightpos[0]+T_inv[0][1]*lightpos[2], T_inv[1][0]*lightpos[0]+T_inv[1][1]*lightpos[2]};
        float customobserv[2]={T_inv[0][0]*campos[0]+T_inv[0][1]*campos[2], T_inv[1][0]*campos[0]+T_inv[1][1]*campos[2]};

        
        float n1=1,n2=1.58;
        for (int i=-linesamplenum/2+1;i<linesamplenum/2;i++){
            float rgbout[3]={0,0,0};
            float custompos[2]={float(i)/(linesamplenum/2-1)*outer,0}; //in custom base
            if (abs(custompos[0])<inner)continue;
            //custom base outer*{sin(angle) -cos(angle)
            //              cos(angle)  sin(angle)}

            float incomingray[3]={custompos[0]-customlight[0], -lightpos[1], customlight[1]};
            float outgoingray[3]={customobserv[0]-custompos[0], campos[1], customobserv[1]};

            float incang=atan(dist(incomingray[2],incomingray[0])/incomingray[1]);
            float outang=atan(dist(outgoingray[2],outgoingray[0])/outgoingray[1]);
            //float pcincang=asin(n1/n2*sin(incang)); //angles inside polyacarbonate layer
            //float pcoutang=asin(n1/n2*sin(outang));

            float diffractincray[3]={incomingray[0],-dist(incomingray[0],incomingray[2])/tan(incang),incomingray[2]};
            float diffractoutray[3]={outgoingray[0],dist(outgoingray[0],outgoingray[2])/tan(outang),outgoingray[2]};
            float diffractincang=atan(diffractincray[0]/dist(diffractincray[1],diffractincray[2]));
            float diffractoutang=atan(diffractoutray[0]/dist(diffractoutray[1],diffractoutray[2]));
            float disdif= abs(d*(sin(diffractincang)-sin(diffractoutang)));
            //TODO intensity
            for (int order=1;order<=4;order++){
                float lambda=disdif/order;
                tuple<int,int,int> rgbval=wavelengthToRGB(lambda*1e9);
                rgbout[0]+=get<0>(rgbval)/255.0;
                rgbout[1]+=get<1>(rgbval)/255.0;
                rgbout[2]+=get<2>(rgbval)/255.0;
            }
            float normpos[2]={T[0][0]*custompos[0], T[1][0]*custompos[0]};
            glPointSize(simplelinesize);
            glBegin(GL_POINTS);
            glColor3f(rgbout[0],rgbout[1], rgbout[2]);
            glVertex3f(normpos[0]+pos[0],pos[1]+thickness+0.0001,normpos[1]+pos[2]);
            glEnd();
        }  
    }
    
    void startinternalpropagation();

    void draw(Light& source, float* campos, bool drawreflectp=false){

        
        renderDiscWithHole();
        
        //switched to get observer effect with deviation
        float adjlightpos[3]={campos[0]-pos[0],campos[1]-pos[1],campos[2]-pos[2]};
        float adjcampos[3]={source.pos[0]-pos[0],source.pos[1]-pos[1],source.pos[2]-pos[2]};
        float reflectedcam[3]={adjcampos[0],-adjcampos[1],adjcampos[2]};
        float factor=adjlightpos[1]/(adjlightpos[1]-reflectedcam[1]);
        float reflectionpoint[3]={adjlightpos[0]+factor*(reflectedcam[0]-adjlightpos[0]), 0, adjlightpos[2]+factor*(reflectedcam[2]-adjlightpos[2])};

        if (drawreflectp){
            glColor3f(source.rescolor[0],source.rescolor[1],source.rescolor[2]);
            glPointSize(4);
            glBegin(GL_POINTS);
            glVertex3f(reflectionpoint[0]+pos[0],pos[1],reflectionpoint[2]+pos[2]);
            glEnd();
        }

       source.draw();

        if (line)calclinecol(adjcampos,adjlightpos);
       
        for (int x=0;x<=internalpossampnum;x++){
            for (int y=0;y<=internalpossampnum;y++){
                float startpoint[3]={outer*(float(x)/internalpossampnum*2-1), thickness, outer*(float(y)/internalpossampnum*2-1)};
                float incomingray[3]={startpoint[0]-adjlightpos[0],startpoint[1]-adjlightpos[1],startpoint[2]-adjlightpos[2]};

                if (x==pathx&&pathy==y){
                    glColor3f(1-cdcolor[0],1-cdcolor[1],1-cdcolor[2]);
                    glPointSize(4);
                    glBegin(GL_POINTS);
                    glVertex3f(startpoint[0],startpoint[1]-thickness+0.0001,startpoint[2]);
                    glEnd();

                    glEnable(GL_BLEND);
                    glDepthMask(GL_FALSE);
                    glBlendFunc(GL_ONE, GL_ONE);
                    drawpaths=true;
                    startinternalpropagation(startpoint, incomingray, adjcampos, source);
                    drawpaths=false;
                    glDisable(GL_BLEND);
                    glDepthMask(GL_TRUE);

                }else{
                    glEnable(GL_BLEND);
                    glDepthMask(GL_FALSE);
                    glBlendFunc(GL_ONE, GL_ONE);
                    startinternalpropagation(startpoint, incomingray, adjcampos, source);
                    glDisable(GL_BLEND);
                    glDepthMask(GL_TRUE);
                }
                
            }
        }
        
        


    }
  
    float internalup();
    float internaldown(float* cdpos, float* dir, float disdif, float lambda, float intensity, bool entry, int steps){//positions relative to cd, pos height is always = thickness here
        if (steps>=maxinternalsteps)return 0;
        float scaleddir[3]={dir[0]/dir[1]*-thickness,-thickness,dir[2]/dir[1]*-thickness};
      
        float collisionpoint[3]={cdpos[0]+scaleddir[0],cdpos[1]+scaleddir[1], cdpos[2]+scaleddir[2]};
        if (dist(collisionpoint[0],collisionpoint[2])>outer || dist(collisionpoint[0],collisionpoint[2])<inner)return 0;
        float visible=0.0;
        if (internaldiffract){
      
        float lineangle=atan(collisionpoint[0]/collisionpoint[2]);
        float T[2][2] = {{sin(lineangle),cos(lineangle)}, {cos(lineangle), -sin(lineangle)}}; //custom->norm
        float det=T[0][0]*T[1][1]-T[1][0]*T[0][1];
        float T_inv[2][2]= {{-sin(lineangle)/det,-cos(lineangle)/det},{-cos(lineangle)/det,sin(lineangle)/det}}; //norm->custom
        
        float customlight[2]={T_inv[0][0]*cdpos[0]+T_inv[0][1]*cdpos[1], T_inv[1][0]*cdpos[0]+T_inv[1][1]*cdpos[1]};
    
        float incomingray[3]={T_inv[0][0]*scaleddir[0]+T_inv[0][1]*scaleddir[2],-thickness,T_inv[1][0]*scaleddir[0]+T_inv[1][1]*scaleddir[2]};
        float diffractincang=atan(incomingray[0]/dist(incomingray[2],incomingray[1]));
        //TODO intensity
        
        int b1=(-1-sin(diffractincang))*d/lambda;
        int b2=(1-sin(diffractincang))*d/lambda;
        
        //this is physically inaccurate, but seems to work to add the bright arc around zero order reflection
        for (int order=floor(min(b1,b2));order<=ceil(max(b1,b2));order++){
            if (abs((lambda*order/d+sin(diffractincang)))>1)continue;
            float intensityfactor;
            if (order==0)intensityfactor=1;
            else intensityfactor=pow(sin(order*0.5/1.6*PI)/(order*0.5/1.6*PI),2);
            float diffractoutang=asin((lambda*order/d+sin(diffractincang)));//todo
            float outgoingray[3]={dist(thickness, incomingray[2])*tan(diffractoutang),thickness,incomingray[2]};
            float normoutgoingray[3]={T[0][0]*outgoingray[0]+T[0][1]*outgoingray[2],thickness,T[1][0]*outgoingray[0]+T[1][1]*outgoingray[2]};
            visible+=internalup(collisionpoint,normoutgoingray,disdif,lambda,intensity*intensityfactor,steps, true);
        }  

        }else{
            float normoutgoingray[3]={scaleddir[0],-scaleddir[1], scaleddir[2]};
            visible+=internalup(collisionpoint,normoutgoingray,disdif,lambda,intensity*1,steps, true);
        }

        if (visible>0.0f&&drawpaths){
            array<float,6> ray={cdpos[0],cdpos[1],cdpos[2],cdpos[0]+scaleddir[0],cdpos[1]+scaleddir[1],cdpos[2]+scaleddir[2]}; 
            downline.push_back(ray);
        }
        return visible;
        
                                                                         
    }

    float internalup(float* cdpos, float* dir, float disdif, float lambda, float intensity, int steps, bool allowdiffract){//positions relative to cd, pos height is always = thickness here
        float scaleddir[3]={dir[0]/dir[1]*thickness,thickness,dir[2]/dir[1]*thickness};
     
        float collisionpoint[3]={cdpos[0]+scaleddir[0],cdpos[1]+scaleddir[1], cdpos[2]+scaleddir[2]};
        if (dist(collisionpoint[0],collisionpoint[2])>outer || dist(collisionpoint[0],collisionpoint[2])<inner)return false;

        float n1=1,n2=1.58;
        float criticalangle=asin(n1/n2);
        float pcexitangle=atan(dist(scaleddir[0],scaleddir[2])/scaleddir[1]);
        
        if (pcexitangle>criticalangle){
            float reflecteddir[3]={scaleddir[0],-thickness, scaleddir[2]};
            float visible=internaldown(collisionpoint, reflecteddir,disdif, lambda, intensity, false,steps+1);
            if (visible>0.0f){
                if (drawpaths){
                    array<float,6> ray={cdpos[0],cdpos[1],cdpos[2],cdpos[0]+scaleddir[0],cdpos[1]+scaleddir[1],cdpos[2]+scaleddir[2]}; 
                    internalupline.push_back(ray);
                    
                }
            }
            return visible;
            
        }else{
            if (!allowdiffract)return 0.0f;
            float refractedangle=asin(n2/n1*sin(pcexitangle));
            float refracteddir[3]={scaleddir[0],dist(scaleddir[0],scaleddir[2])/tan(refractedangle),scaleddir[2]};
            
            float lineangle=atan(cdpos[0]/cdpos[2]);
            float T[2][2] = {{sin(lineangle),cos(lineangle)}, {cos(lineangle), -sin(lineangle)}}; //custom->norm
            float det=T[0][0]*T[1][1]-T[1][0]*T[0][1];
            float T_inv[2][2]= {{-sin(lineangle)/det,-cos(lineangle)/det},{-cos(lineangle)/det,sin(lineangle)/det}}; //norm->custom
            float pcoutgoing[3]={T_inv[0][0]*refracteddir[0]+T_inv[0][1]*refracteddir[2],thickness,T_inv[1][0]*refracteddir[0]+T_inv[1][1]*refracteddir[2]};
            
            //TODO intensity
            int b1=(-1-sin(disdif))*d/lambda;
            int b2=(1-sin(disdif))*d/lambda;
            float visible=0.0f;
            for (int order=floor(min(b1,b2));order<=ceil(max(b1,b2));order++){
                if (abs((lambda*order)/d+disdif)>1)continue;
                float diffractoutang=asin((lambda*order)/d+disdif);//todo
                float outgoingray[3]={dist(refracteddir[1], pcoutgoing[2])*tan(diffractoutang),refracteddir[1],pcoutgoing[2]};
                float normoutgoingray[3]={T[0][0]*outgoingray[0]+T[0][1]*outgoingray[2],refracteddir[1],T[1][0]*outgoingray[0]+T[1][1]*outgoingray[2]};

                float factor=(currentinternalreflectiontarget[1]-thickness)/normoutgoingray[1];
                float scaledrefracteddir[3]={normoutgoingray[0]*factor,currentinternalreflectiontarget[1]-thickness, normoutgoingray[2]*factor};
               
                float destination[3]={collisionpoint[0]+scaledrefracteddir[0], currentinternalreflectiontarget[1], collisionpoint[2]+scaledrefracteddir[2]};


                float camdist2=pow(currentinternalreflectiontarget[0]-collisionpoint[0],2)+pow(currentinternalreflectiontarget[1]-collisionpoint[1],2)+pow(currentinternalreflectiontarget[2]-collisionpoint[2],2);
                float var_t=camdist2/((currentinternalreflectiontarget[0]-collisionpoint[0])*scaledrefracteddir[0]+(currentinternalreflectiontarget[1]-collisionpoint[1])*scaledrefracteddir[1]+(currentinternalreflectiontarget[2]-collisionpoint[2])*scaledrefracteddir[2]);
                float deviation=sqrt(var_t*var_t*(pow(scaledrefracteddir[0],2)+pow(scaledrefracteddir[1],2)+pow(scaledrefracteddir[2],2))-camdist2);
                float deviationangle=atan(deviation/sqrt(camdist2));
                if (abs(deviation)<=maxinternalraydeviation){
                    float intensityfactor;
                    if (order==0)intensityfactor=1;
                    else intensityfactor=pow(sin(order*0.5/1.6*PI)/(order*0.5/1.6*PI),2);
                    visible+=intensity*intensityfactor*pow(1-abs(deviation)/maxinternalraydeviation,0.5);
                    if (drawpaths){  
                        array<float,6> ray={collisionpoint[0],collisionpoint[1],collisionpoint[2],collisionpoint[0]+scaledrefracteddir[0],collisionpoint[1]+scaledrefracteddir[1],collisionpoint[2]+scaledrefracteddir[2]}; 
                        outgoingline.push_back(ray);
                    }
                }
            }
            if (visible>0.0f && drawpaths){
               
               
                array<float,6> ray={cdpos[0],cdpos[1],cdpos[2],cdpos[0]+scaleddir[0],cdpos[1]+scaleddir[1],cdpos[2]+scaleddir[2]}; 
                upline.push_back(ray);
               
            }
            return visible;
        }
    }  


    void startinternalpropagation(float* cdpos, float* dir, float* campos, Light& source){
        currentinternalreflectiontarget=campos;
        float scaleddir[3]={dir[0]/dir[1]*-thickness,-thickness,dir[2]/dir[1]*-thickness};

        float n1=1,n2=1.58;
        float incang=atan(dist(scaleddir[0],scaleddir[2])/scaleddir[1]);
        float refractincang=asin(n1/n2*sin(incang));
        float refractedray[3]={scaleddir[0],dist(scaleddir[0],scaleddir[2])/tan(refractincang),scaleddir[2]};

        float collisionpoint[3]={cdpos[0]+scaleddir[0],cdpos[1]+scaleddir[1], cdpos[2]+scaleddir[2]};
        float lineangle=atan(collisionpoint[0]/collisionpoint[2]);
        float T[2][2] = {{sin(lineangle),cos(lineangle)}, {cos(lineangle), -sin(lineangle)}}; //custom->norm
        float det=T[0][0]*T[1][1]-T[1][0]*T[0][1];
        float T_inv[2][2]= {{-sin(lineangle)/det,-cos(lineangle)/det},{-cos(lineangle)/det,sin(lineangle)/det}}; //norm->custom
        float incomingray[3]={T_inv[0][0]*scaleddir[0]+T_inv[0][1]*scaleddir[2],-thickness,T_inv[1][0]*scaleddir[0]+T_inv[1][1]*scaleddir[2]};
        float diffractincang=atan(incomingray[0]/dist(incomingray[2],incomingray[1]));

      
        glPointSize(raysize);
        glBegin(GL_POINTS);
        bool visible=false;
        for (float lambda:internallambdachecks){
            float signal=internaldown(cdpos,refractedray,sin(diffractincang),lambda, source.spectralcomp(lambda)*lightintensity, true, 0);
            if (signal>0.0f){visible=true;
            float eyeintensity=pow(signal, 1/2.2);
            auto [r, g, b]=wavelengthToRGB(lambda*1.0e9);
            glColor3f(min(1.0,eyeintensity*r/255.0),min(1.0,eyeintensity*g/255.0),min(1.0,eyeintensity*b/255.0));
            glVertex3f(pos[0]+cdpos[0], pos[1]+0.001, pos[2]+cdpos[2]);
            }

            
        }
        glEnd();
        if (visible && drawpaths){
            array<float,6> ray={cdpos[0],cdpos[1],cdpos[2],cdpos[0]-dir[0],cdpos[1]-dir[1],cdpos[2]-dir[2]}; 
            incomeline.push_back(ray);        
        }
        
        
    }


};


