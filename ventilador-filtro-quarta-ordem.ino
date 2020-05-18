#include <Wire.h>

void setup()
{
  Wire.begin();        //Input   
  Serial.begin(115200);  // start Serial for output
}

unsigned long timemicro=0;

unsigned long timemili=0;

int timeadditional=0;


float lowpass4(float current, float previous[4], float sf)
{
    float a0=(1-sf)*(1-sf)*(1-sf)*(1-sf);
    float b1=4*sf;
    float b2=-6*sf*sf;
    float b3=4*sf*sf*sf;
    float b4=-sf*sf*sf*sf; 
    return (a0*current+b1*previous[3]+b2*previous[2]+b3*previous[1]+b4*previous[0]);
} 


float gettime() //Pega o tempo decorrido desde o início da execução
{
    unsigned long time=millis();  
    if((time-timemili)<0){timeadditional++;}  //Ajusta o incremento para caso de overflow do contador de milissegundos
    timemili=time;
    return ((time/1000.0)+timeadditional*4294967.296);
} 

float resettimer() //Reseta o timer de microssegundos
{
    unsigned long timemicro=micros();  
} 

float laptimer() //Pega o tempo do timer de microssegundos
{  
    unsigned long micro=micros();
    if((micro-timemicro)<0)
    {
        return timemicro-4294967295+1+micro;  //Tempo em caso de overflow do contador
    }
    else
    {
        return (micro-timemicro);
    }       
}

float getpressure()
{
    int mb=-1;
    int lb=-1;
    while(mb<0||lb<0)
    {
        Wire.requestFrom(40, 2); 
        mb = Wire.read() << 8;
        lb = Wire.read();
    }
    return (0.024897028*(mb+lb) - 203.944 + 0.25);
}   
     
float pressure=0;
float rate=0;
float prevLPrate[4]={0.0};
float prevLPpressure[4]={0.0};
float LPpressure=0;
float ratesf=0.9;
float pressuresf=0.9;
float transitionratetrigger=50.0;
float stagnationratetrigger=20.0;
float rhythm = -2;
float lastcycletime = 0;
int collect=0;
int ypointer=0;
float ypoints[50];

float stuckalarm=0;
float fastkalarm=0;

float lasttransitiontime=0;
int state=0;
float pipalarm=0;
float peepalarm=0;
float time=0;

float timesec=0;
float prevtimesec=-0.1;
float LPrate=0.0;
float prevpressure=0;

void loop()
{
  resettimer();
  pressure=getpressure();
  timesec=gettime(); 
  rate=(pressure-prevpressure)/(timesec-prevtimesec);   
 
  LPrate=lowpass4(rate,prevLPrate,ratesf); 
  int i=0;
  for(i=0; i<3; i++)
  { 
    prevLPrate[i]=prevLPrate[i+1];   
  }   
  prevLPrate[i]=LPrate;

  LPpressure=lowpass4(pressure,prevLPpressure,pressuresf); 
  i=0;
  for(i=0; i<3; i++)
  { 
    prevLPpressure[i]=prevLPpressure[i+1];   
  }   
  prevLPpressure[i]=LPpressure;  

 
  if(LPrate>transitionratetrigger)
  {
    if(state==0||state==4)
    {
            state=1;
            if(rhythm>=0){(51/(timesec-lastcycletime))+0.15*rhythm;}
        if(rhythm==-1){rhythm=60/(timesec-lastcycletime);}
        if(rhythm==-2){rhythm++;lastcycletime=timesec;}        
        lasttransitiontime=timesec;        
    }
    if(state==2){state=1;Serial.print("APNEA NA INSPIRACAO\r\n");lasttransitiontime=timesec;}//Alarme apenea
    if(state==3){state=1;Serial.print("TRANSICAO REPENTINA\r\n");lasttransitiontime=timesec;}   //Alarme repetino
  }
  if(LPrate<-transitionratetrigger)
  { 
    if(state==0||state==2){state=3;lasttransitiontime=timesec;}  
    if(state==4){state=3;Serial.print("APNEA NA EXPIRACAO\r\n");lasttransitiontime=timesec;} //Alarme apenea
    if(state==1){state=3;Serial.print("TRANSICAO REPENTINA\r\n");lasttransitiontime=timesec;} //Alarme repetino
  }       
  if((LPrate<stagnationratetrigger&&LPrate>0)||((LPrate>(-stagnationratetrigger)&&LPrate<0)))
  {  
    if(state==1){state=2;lasttransitiontime=timesec;}
    if(state==3){state=4;lasttransitiontime=timesec;}
  }   
  collect++;
  if(collect>3)
  {
    ypoints[ypointer]=LPpressure;
    ypointer++; 
    collect=0;  
  }         
  if(ypointer>49)
  {
    ypointer=0;      
  }  
  if((timesec-lasttransitiontime)>6){Serial.print("SISTEMA TRAVADO\r\n");}  
  if(rhythm>30){Serial.print("RITMO ACIMA DO NORMAL\r\n");} 
  if(pressure<4){Serial.print("PEEP ABAIXO DO PADRAO\r\n");}
  if(pressure>40){Serial.print("PIP ACIMA DO PADRAO\r\n");}
  prevpressure=pressure;  
  prevtimesec=timesec;  
  Serial.print(timesec);
  Serial.print("\t");    
  Serial.print(LPpressure);  
  Serial.print("\t"); 
  Serial.print(LPrate);     
  Serial.print("\t");  
  Serial.print(rhythm);     
  Serial.print("\t");    
  if(state==0){Serial.print("\tindefinido\r\n");}  
  if(state==1||state==2){Serial.print("\tinspirando\r\n");digitalWrite(LED_BUILTIN, HIGH);}
  if(state==3||state==4){Serial.print("\texpirando\r\n");digitalWrite(LED_BUILTIN, LOW);}       
  while(laptimer()<5000){}  
}
