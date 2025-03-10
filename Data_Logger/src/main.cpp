#include <Arduino.h>
#include <BluetoothSerial.h>
#include <stdio.h>
#include <HX711.h>

// ESP32 Settings
#define  FREQUENCY_PWM     100 // frecuencia del pwm
#define  RESOLUTION_PWM    12  // bits del pwm
#define  CH_PWM_A       0
#define  AIN1      27 
#define  AIN2      26   
#define  ENABLEA      25
#define  DT 18  // Pin de datos del HX711
#define  SCK 5  // Pin de reloj del HX711

// factor de conversion de bits a voltios:  2^ #bits -1 / voltios
float  voltsToPwm =   (pow(2, RESOLUTION_PWM) - 1) / 12;
String S;
char buffer[1];
 
// Constructores
BluetoothSerial SerialBT;
HX711 hx711;

// Variables Kalman
float dt;
float t_now;
float t_prev;
const int n = 200; // Numero de muestras de calibración
const int m = 2; // Numero de sensores
float f[n][m];  // Datos de calibración
float X_0[m]; // Estimación inicial
float B[m] = {0,1}; // Matriz de entrada     
float C[1][m]= {{1, 0}}; // Matriz de salida
float P[m][m]; // Matriz de covarianza de error
float Q[m][m]; // Ruido del proceso
float R[m][m]; // Ruido de la medición

// Funciones
void configPins(void);
void pwmMotor(float);
void LlenarMatrices(float [m][m], float [m][m], float);
void media(float [n][m], float [m]);
void covarianza(float [n][m], float [m][m]);
String GetLine(void);
float Filtrokalman(float, float);

void setup() {
  Serial.begin(115200);
  SerialBT.begin("ESP32");
  Serial.println("Dispositivo iniciado, puedes conectarlo a Bluetooth");
  configPins();
  hx711.begin(DT, SCK);
  Serial.println("Calibrando");
  for (int j = 0; j<m; j++){
    for (int i = 0; i<n; i++){
      f[i][j] = hx711.read();
    }
  }   
  Serial.println("Calibración terminada"); 
  covarianza(f, R);
  LlenarMatrices(P, Q, 0.03);
  Serial.println("Matriz de covarianza"); 
  for (int j = 0; j<m; j++){
    for (int i = 0; i<m; i++){
      Serial.printf("%f ", R[j][i]);
    }
    Serial.println();
  }  
  t_now = millis();
}

void loop() {
  if(SerialBT.available()){
    S = GetLine();
    strcpy(buffer,S.c_str());
    buffer[strcspn(buffer, "\r\n")] = 0;
  }
  if(strcmp(buffer,"u") == 0)
    pwmMotor(3.2);
  if(strcmp(buffer,"d") == 0)
    pwmMotor(-1);
  if(strcmp(buffer,"s") == 0)
    pwmMotor(0);
  dt = t_now-t_prev;
  float m1 = hx711.read();
  float m2 = hx711.read();
  X_0[0] = m1;
  X_0[1] = m2;
  float m = Filtrokalman(m1,dt);
  Serial.printf("Medición presión 1 %f\n", m1);
  Serial.printf("Medición presión 2 %f\n", m2);
  Serial.printf("Medición con filtro de Kalman %f\n", m);
  t_prev = t_now;
  delay(500);
}

void configPins(void){
  pinMode(AIN1,OUTPUT);
  pinMode(AIN2,OUTPUT);
  ledcSetup(CH_PWM_A,  FREQUENCY_PWM, RESOLUTION_PWM);
  ledcAttachPin(ENABLEA, CH_PWM_A);
}

void pwmMotor(float volts){
  // This function convert a voltage value given in the variable volts
  // to a bipolar pwm signal for controlling the DC motor
  
  uint16_t pwm = abs(volts) * voltsToPwm;

  if (volts < 0){
      // if var volts is negative use CH_PWM_AIN2 to output a pwm signal
      // proportional to the input voltage
      digitalWrite(AIN1, 0);
      digitalWrite(AIN2, HIGH);
      ledcWrite(CH_PWM_A, pwm);
  }
  else{
      // if var volts is negative use CH_PWM_AIN1 to output a pwm signal
      // proportional to the input voltage
      digitalWrite(AIN1, HIGH);
      digitalWrite(AIN2, 0);
      ledcWrite(CH_PWM_A, pwm);
  }
}

void LlenarMatrices(float P[m][m], float Q[m][m], float q){
  for (int j = 0; j<m; j++){
    for (int i = 0; i<m; i++){
      if(i == j){
        P[i][j] = 1;
        Q[i][j] = q;
      }
      else
        P[i][j] = Q[i][j] = 0;
    }
  }  
}

String GetLine(void){   
  String S = "" ;
  if(SerialBT.available()){    
    char c = SerialBT.read(); 
    while ( c != '\n'){             //Hasta que el caracter sea intro    
      S = S + c;
      delay(25);
      c = SerialBT.read();
    }
  }
  return( S + '\n') ;
}

void media(float data[n][m], float mean_vals[m]) {
  for (int j = 0; j < 2; j++) {
      mean_vals[j] = 0.0;
      for (int i = 0; i < n; i++) {
          mean_vals[j] += data[i][j];
      }
      mean_vals[j] /= n;
  }
}

void covarianza(float data[n][m], float cov_matrix[m][m]) {
  float mean_vals[m];
  media(data, mean_vals);

  for (int i = 0; i < m; i++) {
      for (int j = 0; j < m; j++) {
          cov_matrix[i][j] = 0.0;
          for (int k = 0; k < n; k++) {
              cov_matrix[i][j] += (data[k][i] - mean_vals[i]) * (data[k][j] - mean_vals[j]);
          }
          cov_matrix[i][j] /= (n - 1); // Dividir por N-1 para estimación no sesgada
      }
  }
}

float Filtrokalman(float newRate, float dt) {
  // Predicción del estado
  float A[m][m]= {{1+dt*0.000389f, -dt/1.2f}, {0, 1-dt*0.0000148f}}; 
  float X_pred[m] = {A[0][0] * X_0[0] + A[0][1] * X_0[1] + dt * newRate, A[1][0] * X_0[0] + A[1][1] * X_0[1]}; 

  // Predicción de la covarianza de error
  float P_pred[m][m];
  for (int i = 0; i < m; i++) {
    for (int j = 0; j < m; j++) {
        P_pred[i][j] = 0;
        for (int k = 0; k < 2; k++) {
            P_pred[i][j] += A[i][k] * P[k][j];
        }
    }
    P_pred[i][i] += Q[i][i] * dt; // Añadir ruido del proceso
  }

  // Actualización
  float S = P_pred[0][0] + R[0][0];
  float K[m] = {P_pred[0][0] / S, P_pred[1][0] / S}; // Ganancia de Kalman

  // Actualización de la covarianza del error
  float I_KC[m][m] = {{1 - K[0], 0}, {-K[1], 1}};
  float P_new[m][m] = {{0, 0}, {0, 0}};
  for (int i = 0; i < m; i++) {
      for (int j = 0; j < m; j++) {
          for (int k = 0; k < m; k++) {
              P_new[i][j] += I_KC[i][k] * P_pred[k][j];
          }
      }
  }

  // Guardar valores actualizados
  for (int i = 0; i < m; i++)
      for (int j = 0; j < m; j++)
          P[i][j] = P_new[i][j];

  return X_0[0]; // Retorna la predicción
}