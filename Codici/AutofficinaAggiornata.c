/* -------------------------------------------------------------------------- * 
 * Nome            : Autofficina.c                                            *
 * Autore          : G. A. Tummolo                                            *
 * Linguaggio      : C                                                        *
 * Ultima Revisione: 19-05-2022                                               *
 * Questa versione è un modello più complesso                                 * 
 * -------------------------------------------------------------------------- */
#include <stdio.h>
#include <math.h>
#include "rngs.h"               /* the multi-stream generator           */
#include "rvgs.h"               /* random variate generators            */
#include <unistd.h>
#include <stdbool.h>
#define START 0.0               /* initial time                         */
#define STOP 30000.0            /* terminal (close the door) time       */
#define INFINITE (100.0 * STOP) /* must be much larger than STOP        */
#define SERVERS 2
#define LAMBDA 0.2941            /* Traffic flow rate                    */
#define MU 0.25                  /* Service flow rate to repair machine  */
#define MU_D 2.0                /* Service flow rate to analyze machine */
#define MU_P 4.0               /* Service flow rate to show if the machine has new a new repair */

//TODO: ERRORI sull'individuazione degli eventi con alcuni seed
// Clock Time
typedef struct
{
    double current; // current time
    double next;    // next-event time
} t;

// Output Statistics
typedef struct
{                   // aggregated sums of:       
    double service;     //   service times                    
    long served;        //   number of served jobs          
    long arrives;       //   arrives in the node             

} sum[SERVERS];


typedef struct
{
    double t;  //next event time
    int x;    // status: 0 (off) or 1-2-3 (on)
                // nel caso dell'arrivo se 1 vuol dire che è nella prima coda 2 nella seconda 3 nella terza
                // oppure che sto servendo il job della prima coda (1) o della seconda coda (2)
}event_list[1+SERVERS+2];

/*
Evento 0 è arrivo nel sistema
Evento 1 è servizio per Diagnosi Auto
Evento 2 a SERVER+1 servizio per riparazione Auto
Evento SERVER+2 servizio prova auto
*/

int n_server_free = SERVERS;
long queue[5] = { 0 , 0 , 0 , 0 , 0 }; //number of jobs in queue
/*Coda 0 è la coda nel servizio di Diagnosi
Coda 1 è la coda dei Job con massimo grado di privilegio
Coda 2 è la coda dei job con medio grado di privilegio
Coda 3 è la coda dei job con più basso grado di privilegio
Coda 4 è la coda dei job per la prova do averla scesa dal ponte
*/
long arrivals = 0;
long departures = 0;
double area[5] = { 0.0 , 0.0 , 0.0 , 0.0 , 0.0 };
t clock;
event_list event;
sum statistics;

int Type_of_arrive()
{
    double rand = Random();
    if(rand > 0 && rand < 6.5/10.0)
    {
        return 2;
    }else
    {
        return 3;
    }
}

int NumeroDiJobPerTipoNelSistema(int tipo_di_job)
{
    int numero = 0;
    for(int z = 2; z <= SERVERS+1; z++)
    {
        if(event[z].x == tipo_di_job)
        {
            numero++;
        }
    }
    return numero;
}

double GetArrival()
{
/* -------------------------------------------------------------------------- * 
 * generate the next arrival time, with rate LAMBDA                           *
 * -------------------------------------------------------------------------- */
    static double arrival = START;

    SelectStream(0);
    arrival += Exponential(1.0 / LAMBDA);
    return (arrival);
}

double GetService(double mu)
{
/* -------------------------------------------------------------------------- * 
 * generate the next service time for the access points                       *
 * -------------------------------------------------------------------------- */
    SelectStream(1);
    return (Exponential(1.0 / mu)); 
}

bool empty_queues()
{
    //Vero se ci sono ancora elementi da processare
    int sum_queues = queue[0] + queue[1] + queue[2] + queue[3] + queue[4];
    if (sum_queues != 0)
        {
            return true;
        }
}


int NextEvent()
{
    int e;
    int i = 0;
    while (event[i].x == 0)
    {
        i++;
    }
    e = i;
    for(int z = i+1; z<= SERVERS+2;z++){
        if(event[z].x !=0 && (event[z].t < event[e].t))
        {
            e = z;
        }        
    }
    return (e);

}

int FoundServerFree()
{
    for(int z = 2; z <= SERVERS+1; z++)
    {
        //printf("Il server %d è nello stato %d\n",z,event[z].x);
        if(event[z].x == 0)
        {
            return z;
        }
    }
}


void ProcessArrivals()
{
    if(event[0].x == 2)
    {
        /*
        In questo caso l'arrivo è nella coda senza servizio di Diagnosi, quindi se la coda delle auto già riparate è vuota posso mettere 
        l'auto direttamente sul ponte
        */

        if(n_server_free != 0)
        {
            /*
            C'è qualche ponte libero
            */
            int server_free = FoundServerFree(); //Trovo il ponte libero
            double service_time = GetService(MU);
            event[server_free].t = clock.current + service_time;
            event[server_free].x = 2;
            n_server_free--;
            
            
        }
        queue[2]++;
        

    }
    else
    {
        /*
        In questo caso l'auto ha bisogno prima di fare la diagnosi e poi di essere messa sul ponte
        */
        if(event[1].x == 0)
        {
            double service_time = GetService(MU_D);
            event[1].t = clock.current + service_time;
            event[1].x = 1;
        }
        queue[0]++;
        
    }
    
}

int QualeCodaStavaEseguendo(int current_server)
{
    return event[current_server].x;
}


void ProcessDeparture(int current_server)
{
    double service_time = 0.0;

    if(current_server == 1)
    {
        /*
        Partenza dal servizio di diagnosi
        */
        if(queue[0] > 1)
        {
            double service_time = GetService(MU_D);
            event[1].t = clock.current + service_time;
            event[1].x = 1;
        }
        else
        {
            event[1].t = INFINITE;
            event[1].x = 0;
            
        }
        queue[0]--;
        if(n_server_free != 0 && (queue[3] - NumeroDiJobPerTipoNelSistema(3)) == 0)
        {
            int server_free = FoundServerFree();
            double service_time = GetService(MU);
            event[server_free].t = clock.current + service_time;
            event[server_free].x = 3;
            n_server_free--;
        }
        queue[3]++;
    }
    else if(current_server == (SERVERS + 2))
    {
        queue[4]--;
        double rand = Random();
        if(rand > 0 && rand <= 1.0/10.0)
        {
            queue[1]++;
            if(n_server_free != 0)
            {
                /*
                C'è qualche ponte libero
                */
                int server_free = FoundServerFree(); //Trovo il ponte libero
                double service_time = GetService(MU);
                event[server_free].t = clock.current + service_time;
                event[server_free].x = 1;
                n_server_free--;
            }
        }
        else
        {
            departures++;
        }

        if(queue[4] > 0)
        {
            double service_time = GetService(MU_P);
            event[SERVERS + 2].t = clock.current + service_time;
            event[SERVERS + 2].x = 1;
        }
        else
        {
            event[SERVERS + 2].t = INFINITE;
            event[SERVERS + 2].x = 0;
        }
    }
    else
    {
        if(QualeCodaStavaEseguendo(current_server) == 1)
        {
            queue[1]--;

        }
        else if(QualeCodaStavaEseguendo(current_server) == 2)
        {
            queue[2]--;

        }
        else if(QualeCodaStavaEseguendo(current_server) == 3)
        {
            queue[3]--;

        }

        event[current_server].x = 0;
        
        if(queue[1] - NumeroDiJobPerTipoNelSistema(1) > 0)
        {
              
            service_time = GetService(MU);
            event[current_server].t = service_time +clock.current;
            event[current_server].x = 1;
        }
        else if(queue[2] - NumeroDiJobPerTipoNelSistema(2) > 0)
        {
               
            service_time = GetService(MU);
            event[current_server].t = service_time +clock.current;
            event[current_server].x = 2;
            statistics[current_server-1].service += service_time;
            statistics[current_server-1].served++;
        
        }
        else if(queue[3] - NumeroDiJobPerTipoNelSistema(3)> 0)
        {
               
            service_time = GetService(MU);
            event[current_server].t = service_time +clock.current;
            event[current_server].x = 3;
            statistics[current_server-1].service += service_time;
            statistics[current_server-1].served++;
            
        }
        else
        {
            event[current_server].t = INFINITE;
            event[current_server].x = 0;
            n_server_free++;
        }

        if(event[SERVERS+2].x == 0)
        {
            double service_time = GetService(MU_P);
            event[SERVERS+2].t = service_time +clock.current;
            event[SERVERS+2].x = 1;
        }
        queue[4]++;
    }
    

    
}

void main()
{
    //Init
    
    PlantSeeds(0);
    clock.current = START;
    event[0].t = GetArrival();
    event[0].x = Type_of_arrive();
    for(int z=1; z<=SERVERS+2; z++)
    {
        event[z].t = INFINITE;
        event[z].x = 0;
    }
    int aa;
    while((event[0].t < STOP || empty_queues())){
        int e = NextEvent();
        
        
        
        
        
        clock.next = event[e].t;
        for(int i = 0 ; i < 5 ; i++)
        {
            double current_area = (clock.next - clock.current) * queue[i];
            area[i] += (clock.next - clock.current) * queue[i]; 
        }
        /*if(clock.current > 29910.0)
            {
                printf("%ld Partenze\n",departures);
                printf("%ld Arrivi\n",arrivals);
                printf("Server    Stato\n");
                for(int k=1 ; k<= SERVERS+2 ;k++)
                {
                    printf("%d          %d\n",k,event[k].x);
                }
                printf("Numero server Liberi =%d\n",n_server_free);
                printf("-----------------------------\n");
                for(int k=0 ; k<= SERVERS+2 ;k++)
                {
                    printf("evento %d  t=%f stato x=%d\n",k,event[k].t,event[k].x);
                }
                printf("Coda 0 (CODA DIAGNOSI) =%ld\n",queue[0]);
                printf("Coda 1 (CODA MACCHINE GIÀ SERVITE) =%ld\n",queue[1]);
                printf("Coda 2 (CODA MACCHINE NO DIAGNOSI) =%ld\n",queue[2]);
                printf("Coda 3 (CODA MACCHINE DOPO DIAGNOSI) =%ld\n",queue[3]);
                printf("Coda 4 (CODA MACCHINE PROVA) =%ld\n",queue[4]);
                
                printf("L'evento preso in carico è %d\n",e);
                scanf("%d",&aa);
                printf("\n\n");

            }*/
        
        clock.current = clock.next;
        

        if(e == 0)
        {
            // Process an Arrival
            arrivals++;
            ProcessArrivals();
            event[0].t = GetArrival();
            event[0].x = Type_of_arrive();
            if(event[0].t > STOP)
            {
                event[0].x = 0;
            }
        }
        else
        {
            ProcessDeparture(e);
        }
        
    }
    

    
    double tot_area = area[0] + area[1] + area[2] + area[3] + area[4];
    printf("Area 0 %lf\n",area[0]);
    printf("Area 1 %lf\n",area[1]);
    printf("Area 2 %lf\n",area[2]);
    printf("Area 3 %lf\n",area[3]);
    printf("Area 4 %lf\n",area[4]);
    printf("Output Statistics (computed using %ld jobs) are:\n\n", departures);
    printf("1) Global Statistics\n");
    printf("  avg interarrival time = %6.6f\n", event[0].t/arrivals);
    printf("  avg waiting time = %6.6f\n", tot_area/departures);
    printf("  avg number of jobs in the network = %6.2f\n",
           tot_area/clock.current);
    printf("%ld Partenze\n",departures);
    printf("%ld Arrivi\n",arrivals);
    printf("%ld\n",queue[0] + queue[1] + queue[2] + queue[3] + queue[4]);
    printf("Current clock è %lf\n",clock.current);
    

}