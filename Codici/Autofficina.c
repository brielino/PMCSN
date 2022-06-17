/* -------------------------------------------------------------------------- * 
 * Name            : Autofficina.c                                            *
 * Authors         : G. A. Tummolo                                            *
 * Language        : C                                                        *
 * Latest Revision : 16-06-2022                                               *
 * -------------------------------------------------------------------------- */
/*
                                 ________                      ______
                                        |                     /  M   \
------------------------------>  Coda1  |--------->|         |   U    |
                                ________|          |         |   L    |
                                                   |-------->|   T    |-------->
        ________   ____         ________           |         |   I    |
                | /    \                |          |         |   S    |  
------> Coda2   ||1/MU_D|------> Coda3  |--------->|          \______/  
        ________| \____/        ________|
*/

#include <stdio.h>
#include <math.h>
#include "rngs.h"               /* the multi-stream generator           */
#include "rvgs.h"               /* random variate generators            */
#include <unistd.h>
#include <stdbool.h>
#define START 0.0               /* initial time                         */
#define STOP 11520.0            /* terminal (close the door) time       */
#define INFINITE (100.0 * STOP) /* must be much larger than STOP        */
#define SERVERS 2
#define LAMBDA 0.2941               /* Traffic flow rate                    */
#define MU 0.2                 /* Service flow rate                    */
#define MU_D 1.66                /* Service flow rate Diagnosi           */


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

} sum[SERVERS+1];


typedef struct
{
    double t;  //next event time
    int x;    // status: 0 (off) or 1 (on)
                // nel caso dell'arrivo se 1 vuol dire che è nella prima coda 2 nella seconda
                // oppure che sto servendo il job della prima coda (1) o della seconda coda (2)
}event_list[1+1+SERVERS]; 
//event_list[0] = Evento di arrivo nel sistema
//event_list[1] = Evento servizio Diagnosi riferimento alla coda 2
//event_list[i>1] = Evento servizio Multi-server
int n_server_free = SERVERS;
long queue[3] = { 0 , 0 , 0}; //number of jobs in queue
long arrivals = 0;
long departures = 0;
double area[3] = { 0.0 , 0.0 , 0.0};
t clock;
event_list event;
sum statistics;

int Type_of_arrive()
{
    double rand = Random();
    if(rand > 0 && rand < 6.5/10.0)
    {
        return 1;
    }else
    {
        return 2;
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
    int sum_queues = queue[0] + queue[1] +queue[2];
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
    while(i < SERVERS+2){
        i++;
        if(event[i].x !=0 && (event[i].t < event[e].t))
            e = i;
    }
    return e;

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
    if(event[0].x == 1)
    {
        
        if(n_server_free != 0)
        {
            //printf("Sto controllando elementi in coda 1\n");
            //printf("il numero di elementi in coda è %ld\n",queue[0] - NumeroDiJobPerTipoNelSistema(1));
            //if((queue[0] - NumeroDiJobPerTipoNelSistema(1))== 0)
            
                int server_free = FoundServerFree();
                //printf("Il server libero è  = %d\n",server_free);
                double service_time = GetService(MU);
                event[server_free].t = clock.current + service_time;
                event[server_free].x = 1;
                statistics[server_free-1].service += service_time;
                statistics[server_free-1].served++;
                n_server_free--;
            
            
        }
        queue[0]++;
        

    }
    else
    {
        
        if(event[1].x == 0)
        {
            
            double service_time = GetService(MU_D);
            event[1].t = clock.current + service_time;
            event[1].x = 3;
            statistics[1].service += service_time;
            statistics[1].served++;
        }
        queue[1]++;
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
        if(queue[1] > 1)
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
        queue[1]--;
        if(n_server_free != 0 && (queue[2] - NumeroDiJobPerTipoNelSistema(2)) == 0)
        {
            int server_free = FoundServerFree();
            double service_time = GetService(MU);
            event[server_free].t = clock.current + service_time;
            event[server_free].x = 2;
            statistics[server_free-1].service += service_time;
            statistics[server_free-1].served++;
            n_server_free--;
        }
        queue[2]++;
        
    }
    else
    {

        if(queue[0] - NumeroDiJobPerTipoNelSistema(1)> 0)
        {   
            if(QualeCodaStavaEseguendo(current_server) == 1)
            {
                queue[0]--;
            }
            else if(QualeCodaStavaEseguendo(current_server) == 2)
            {
                queue[2]--;
            }
            service_time = GetService(MU);
            event[current_server].t = service_time +clock.current;
            event[current_server].x = 1;
            statistics[current_server-1].service += service_time;
            statistics[current_server-1].served++;
        
        }
        else if(queue[2] - NumeroDiJobPerTipoNelSistema(2)> 0)
        {
            if(QualeCodaStavaEseguendo(current_server) == 1)
            {
                queue[0]--;
            }
            else if(QualeCodaStavaEseguendo(current_server) == 2)
            {
                queue[2]--;
            }
            service_time = GetService(MU);
            event[current_server].t = service_time +clock.current;
            event[current_server].x = 2;
            statistics[current_server-1].service += service_time;
            statistics[current_server-1].served++;
        }
        else
        {
            if(QualeCodaStavaEseguendo(current_server) == 1)
            {
                queue[0]--;
            }
            else if(QualeCodaStavaEseguendo(current_server) == 2)
            {
                queue[2]--;
            }
            event[current_server].t = INFINITE;
            event[current_server].x = 0;
            n_server_free++;
        }
        departures++;
    }


}

void main()
{
    //Init
    PlantSeeds(0);
    clock.current = START;
    event[0].t = GetArrival();
    event[0].x = Type_of_arrive();
    for(int z=1; z<=SERVERS+1; z++)
    {
        event[z].t = INFINITE;
        event[z].x = 0;
    }
    int aa;
    while((event[0].t < STOP || empty_queues())){
        int e = NextEvent();
        /*
        printf("%ld Partenze\n",departures);
        printf("%ld Arrivi\n",arrivals);
        printf("Server    Stato\n");
        for(int k=1 ; k<= SERVERS+1 ;k++)
        {
            printf("%d          %d\n",k,event[k].x);
        }
        printf("Numero server Liberi =%d\n",n_server_free);
        printf("-----------------------------\n");
        for(int k=0 ; k<= SERVERS+1 ;k++)
        {
            printf("evento %d  t=%f stato x=%d\n",k,event[k].t,event[k].x);
        }
        printf("Coda Privilegiata =%ld\n",queue[0]);
        printf("Coda Diagnosi =%ld\n",queue[1]);
        printf("Coda Non Privilegiata =%ld\n",queue[2]);
        printf("L'evento preso in carico è %d\n",e);
        scanf("%d",&aa);
        printf("\n\n");*/
        
        clock.next = event[e].t;
        area[0] += (clock.next - clock.current) * queue[0];
        area[1] += (clock.next - clock.current) * queue[1];
        area[2] += (clock.next - clock.current) * queue[2];
        clock.current = clock.next;
        

        if(e == 0)
        {
            // Process an Arrival
            //printf("Processo Arrivo\n\n");
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
    

    
    double tot_area = area[0] + area[1] + area[2];
    printf("Area 0 %lf\n",area[0]);
    printf("Area 1 %lf\n",area[1]);
    printf("Area 2 %lf\n",area[2]);
    printf("Output Statistics (computed using %ld jobs) are:\n\n", departures);
    printf("1) Global Statistics\n");
    printf("  avg interarrival time = %6.6f\n", event[0].t/arrivals);
    printf("  avg waiting time = %6.6f\n", tot_area/departures);
    printf("  avg number of jobs in the network = %6.2f\n",
           tot_area/clock.current);
    printf("%ld Partenze\n",departures);
    printf("%ld Arrivi\n",arrivals);
    printf("%ld\n",queue[0]+queue[1]+queue[2]);
    printf(" avg interarrival time = %6.6f\n",clock.current/departures);

}
