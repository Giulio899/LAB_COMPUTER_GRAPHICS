// Fragment shader: : Toon shading
// ================
#version 450 core

in vec3 N;
in vec3 L;
in vec3 E;

// Ouput data
out vec4 FragColor;


void main()
{    
   vec4 a = vec4(0.0,0.5,0.8,1.0);
   vec4 b = vec4(0.0,0.3,0.6,1.0);
   vec4 c = vec4(0.0,0.2,0.5,1.0);
   vec4 d = vec4(0.0,0.3,0.6,1.0);
   vec4 e = vec4(0.0,0.0,0.1,1.0);
   float intensity = dot(normalize(L), normalize(N));
   vec4 color;
   if(intensity>0.95){
      color=a;
   }
   else if(intensity>0.5){
      color=b;
   }
   else if(intensity>0.25){
      color=c;
   }
   else if(intensity>0.15){
      color=d;
   }
   else{
      color=e;
   }

   FragColor = color;
    
}
