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

double Calculate_PQ(int n_server,double RO)
{
    double partial_sum = 0.0;
    for(int i=0; i<n_server; i++)
    {
        double potenza = pow(n_server*RO,i);
        int fattoriale = Fattoriale(i);
        partial_sum+= potenza/fattoriale;
    }
    partial_sum+= pow(n_server*RO,n_server)/(Fattoriale(n_server)*(1-RO));
    partial_sum = 1/partial_sum;
    partial_sum = partial_sum*pow(n_server*RO,n_server)/(Fattoriale(n_server)*(1-RO));
    return partial_sum;
}

void MultiServer()
{
    double prob1=0.65;
    double prob2=0.35;
    int n_server=2;
    double LAMBDA=0.2941;
    double MU=0.25;
    
    double E_s = 1/(MU*n_server);

    double E_si = 1/MU;

    double LAMBDA1 = prob1*LAMBDA;
    double LAMBDA2 = prob2*LAMBDA;

    double R1 = LAMBDA1/(n_server*MU);
    double R2 = LAMBDA2/(n_server*MU);

    double RO = LAMBDA/(n_server*MU);
    double PQ1 = Calculate_PQ(n_server,R1);
    double PQ = Calculate_PQ(n_server,RO);

    double E_ts = prob1*((PQ1*E_s)/(1-R1)+E_si)+prob2*((PQ*E_s)/((1-RO)*(1-R1))+E_si);
    printf("PQ1 = %f  ----   PQ = %f ----- E(tq) = %f\n",PQ1,PQ,E_ts);
}

void SingleServer()
{
    double prob1=0.65;
    double prob2=0.35;
    
    double LAMBDA=0.2941;
    double MU=0.7;
    
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
    if(argc < 2)
    {
        printf("Passare 1 se si vuole il Single-server, un altro numero se Multi-server\n");
        return;
    }
    if(*argv[1] == '1')
    {
        SingleServer();
    }else
    {
        MultiServer();
    }


}