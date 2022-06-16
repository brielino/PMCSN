#include <stdio.h>
#include <math.h>
#include <unistd.h>
//Calcolo valori teorici

void main()
{
    double prob1=0.65;
    double prob2=0.35;
    
    double LAMBDA=0.2941;
    double MU=0.25;
    
    double E_s = 1/(MU);


    double LAMBDA1 = prob1*LAMBDA;
    double LAMBDA2 = prob2*LAMBDA;

    double R1 = LAMBDA1/(MU);
    double R2 = LAMBDA2/(MU);

    double RO = LAMBDA/(n_server*MU);
    

    double E_ts = prob1*((RO*E_s)/(1-R1)+E_s)+prob2*((RO*E_s)/((1-RO)*(1-R1))+E_s);
    printf("R1 = %f  ----   R2 = %f ----- E(tq) = %f\n",R1,R2,E_ts);


}