#include <TimerOne.h>
#define A A0 // sensor A0
#define B A1 // sensor A1
#define C A2 // sensor A2
#define D A3 // sensor A3
#define ME 3 // saída 3
#define MD 5 // saída 5
#define INA 4
#define INB 6 

float r, y, e, e_a, u, u_da, u_ia;
uint8_t k;
bool flag_i = false;
bool flag_s = false;

const float u_base = 127;
const float k_p = 50;
const float k_i = 0;
const float k_d = 5;
const float t_s = 0.01;
const float f_f = 1;
const uint8_t p = 10;
const uint8_t u_i_max = 127;
const uint8_t u_p_max = 64;


// função para interrupção do Timer1
void Timer1_ISR(void)
{
  flag_i = true;
}

// função para leitura dos sensores
void medicao()
{
  float y_1 = 1023 -analogRead(A);
  float y_2 = 1023 -analogRead(B);
  float y_3 = 1023 -analogRead(C);
  float y_4 = 1023 -analogRead(D);
  float y_soma = y_1 +y_2 +y_3 +y_4;
  if (y_soma > 0) y = (1*y_1 +2*y_2 +3*y_3 +4*y_4)/y_soma;
}

// função para lógica de ativação dos motores
void controle()
{
  // setpoint
  r = 2.5;

  // erro de seguimento
  e = r -y;

  // sinal de controle
  float u_p = k_p*e*abs(e);
  if (u_p >  u_p_max) u_p =  u_p_max;
  if (u_p < -u_p_max) u_p = -u_p_max;
  float u_i = u_ia*f_f;
  if( abs(e) < 1 )
  {
    u_i = u_ia*f_f +k_i*t_s*(e +e_a)/2;
  }
  if (u_i >  u_i_max) u_i =  u_i_max;
  if (u_i < -u_i_max) u_i = -u_i_max;
  float u_d = u_da*(2-p*t_s)/(2+p*t_s) +2*p*k_d/(2+p*t_s)*(e -e_a);
  u = u_p +u_i +u_d;

  // sinal de controle para cada motor
  float u_ME = u_base +u;
  float u_MD = u_base -u;

  // limita sinal de controle
  if(u_ME > 255)  u_ME =  255;
  if(u_ME < -255) u_ME = -255;
  if(u_MD > 255)  u_MD =  255;
  if(u_MD < -255) u_MD = -255;

  // acionamento PWM
  if (u_ME >= 0) digitalWrite(INA, HIGH);
  else analogWrite(INA, LOW);
  if (u_MD >= 0) digitalWrite(INB, HIGH);
  else analogWrite(INB, LOW);
  analogWrite(ME, u_ME);
  analogWrite(MD, u_MD);
  
  // atualiza valores passados
  e_a  = e;
  u_ia = u_i;
  u_da = u_d;
}

// função comunicação para aquisição de dados
void comunicacao()
{
  // r, y, e, u são variáveis globais para poder usá-las nessa função
  if (k == 50){
    Serial.print(r, 2);
    Serial.print(" ");
    Serial.print(y, 2);
    Serial.print(" ");
    Serial.print(e, 2);
    Serial.print(" ");
    Serial.println(u, 2);
    k = 0;
  }
  k++;
}

// função de inicilização
void setup()
{
  // inicialização de variáveis
  y    = 2.5;
  e_a  = 0;
  u_ia = 0;
  u_da = 0;
  k    = 0;

  // define pinos como saídas
  pinMode(ME, OUTPUT);
  pinMode(MD, OUTPUT);
  pinMode(INA, OUTPUT);
  pinMode(INB, OUTPUT);

  // inicia interrupção do Timer1 a cada t_s segundos
  Timer1.initialize(t_s*1000000);
  Timer1.attachInterrupt(Timer1_ISR);

  // inicia comunicação serial
  if(flag_s)
  {
    Serial.begin(115200);
  }
}

// função laço de repetição
void loop()
{
  if (flag_i)
  {
    flag_i = false;
    medicao();
    controle();
    if (flag_s)
    {
      comunicacao();
    }
  }
}