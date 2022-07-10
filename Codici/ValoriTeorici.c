#include <stdio.h>
#include <math.h>
#include <unistd.h>
//Calcolo valori teorici

int Fattoriale(int valore)
{
    if(valore == 0)
    {
        return 1;
    }
    int fattor = valore;
    valore--;
    while(valore>0)
    {
        fattor = fattor* valore;
        valore--;
    }
    return fattor;
}

double Calculate_PQ(int n_ponti,double RO)
{
    double partial_sum = 0.0;
    for(int i=0; i<n_ponti; i++)
    {
        double potenza = pow(n_ponti*RO,i);
        int fattoriale = Fattoriale(i);
        partial_sum+= potenza/fattoriale;
    }
    partial_sum+= pow(n_ponti*RO,n_ponti)/(Fattoriale(n_ponti)*(1-RO));
    partial_sum = 1/partial_sum;
    partial_sum = partial_sum*pow(n_ponti*RO,n_ponti)/(Fattoriale(n_ponti)*(1-RO));
    return partial_sum;
}

void MultiServer()
{
    double prob1 = 0.65;
    double prob2 = 0.35;
    int n_ponti = 3;
    double LAMBDA = 0.5;
    double MU = 0.2;
    double MU_D = 1.66;
    
    double E_sD = 1/MU_D;
    
    double E_s = 1/(MU*n_ponti);

    double E_si = 1/MU;

    double LAMBDA1 = prob1*LAMBDA;
    double LAMBDA2 = prob2*LAMBDA;

    double RO_D = LAMBDA2/MU_D;

    double R1 = LAMBDA1/(n_ponti*MU);
    double R2 = LAMBDA2/(n_ponti*MU);

    double RO = LAMBDA/(n_ponti*MU);
    double PQ1 = Calculate_PQ(n_ponti,R1);
    double PQ = Calculate_PQ(n_ponti,RO);
    double E_tq1 = ((PQ1*E_s)/(1-R1));
    double E_tq2 = ((PQ*E_s)/((1-RO)*(1-R1)));
    double E_tqD = (RO_D*(1/MU_D))/(1-RO_D);
    printf("1) Statistiche Globali (Sistema)\n");
    printf("    E(Ts) = %6.6f\n",(prob1*E_tq1+prob2*E_tq2) +E_si +prob2*(E_tqD+E_sD));
    printf("2) Statistiche Locali\n");
    printf("            E(ts)       E(tq)       E(S)\n");
    printf("Diagnosi    %6.6f   %6.6f   %6.6f\n",E_tqD+E_sD,E_tqD,E_sD);
    
    printf("Coda_1      %6.6f   %6.6f   %6.6f\n",E_tq1+E_si,E_tq1,E_si);
    printf("Coda_2      %6.6f   %6.6f   %6.6f\n",E_tq2+E_si,E_tq2,E_si);
    printf("Multiserver %6.6f   %6.6f   %6.6f\n",(E_tq1+E_si)*0.65+(E_tq2+E_si)*0.35,E_tq1*prob1+E_tq2*prob2,E_si);
    
}

void SingleServer()
{
    double prob1=0.65;
    double prob2=0.35;
    
    double LAMBDA=0.25;
    double MU=0.2;
    
    double E_s = 1/(MU);


    double LAMBDA1 = prob1*LAMBDA;
    double LAMBDA2 = prob2*LAMBDA;

    double R1 = LAMBDA1/(MU);
    double R2 = LAMBDA2/(MU);

    double RO = LAMBDA/(MU);
    

    double E_ts = prob1*((RO*E_s)/(1-R1)+E_s)+prob2*((RO*E_s)/((1-RO)*(1-R1))+E_s);
    printf("R1 = %f  ----   R2 = %f ----- E(tq) = %f\n",R1,R2,E_ts);
}

void main(int argc, char * argv[])
{
    MultiServer();

}