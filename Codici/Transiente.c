/* -------------------------------------------------------------------------- * 
 * Name            : Transiente.c                                            *
 * Authors         : G. A. Tummolo                                            *
 * Language        : C                                                        *
 * Latest Revision : 19-06-2022                                               *
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
#include "rngs.h"               
#include "rvgs.h"               
#include <unistd.h>
#include <stdbool.h>
#define START 0.0               //Tempo di inizio della simulazione
#define STOP 3000000.0 
#define INFINITE (100.0 * STOP) 
#define PONTI 2                 //Numero di ponti in Autofficina
#define LAMBDA 0.2941           //Tasso di arrivo

#define MU 0.2                  //Tasso di servizio Ponte
#define MU_D 1.66               //Tasso di servizio Diagnosi



typedef struct
{
    double current;             //Tempo corrente
    double next;                //Tempo prossimo evento
} t;



typedef struct
{
    double t;                   //Istante di tempo in cui avviene l'evento
    int x;                      //Tipo di evento 
                                // nel caso dell'arrivo se 1 vuol dire che è nella prima coda  mentre 2 nella seconda
                                // oppure che sto servendo il job della prima coda (1) o della seconda coda (2)
}event_list[1+1+PONTI]; 
                                //event_list[0] = Evento di arrivo nel sistema
                                //event_list[1] = Evento servizio Diagnosi riferimento alla coda 2
                                //event_list[i>1] = Evento servizio Multi-server
int n_ponti_liberi = PONTI;
long queue[3] = { 0 , 0 , 0};   //Numero Automobili in coda
long arrivi = 0;              //Totale Automobili arrivate nel sistema
long partenze = 0;            //Totale Automobili servite
double area[3] = { 0.0 , 0.0 , 0.0};
long arrivi_tipo1 = 0;          //Totale numero Automobili di tipo 1 arrivate nel sistema
long arrivi_tipo2 = 0;          //Totale numero Automobili di tipo 2 arrivate nel sistema
double arrival = 0.0;
t clock;
event_list event;


void Restart()
{
    n_ponti_liberi = PONTI;
    arrival = 0.0;
    arrivi = 0;
    partenze = 0;
    for(int i = 0 ; i < 3 ; i++)
    {
        area[i] = 0.0;
        queue[i] = 0;
    }
    for(int j = 0 ; j< PONTI+2; j++)
    {
        event[j].x = 0;
        event[j].t = INFINITE;
    }
}

int Type_of_arrive()
/*
Input: void
Output: int (Rappresenta il tipo di Automobile)
Descrizione:
    Funzione che utilizza la funzione Random() per identificare il tipo di Automobile arrivata nel sistema
    1)Automobile di cui si conosce già il problema e non ha bisogno di effettuare la Diagnosi
    2)Automobile che ha bisogno di effettuare la Diagnosi
*/
{
    double rand = Random();
    if(rand > 0 && rand < 6.5/10.0)
    {
        arrivi_tipo1++;
        return 1;
    }else
    {
        arrivi_tipo2++;
        return 2;
    }
}

int NumeroDiJobPerTipoNelSistema(int tipo_auto)
/*
Input: int (Identificativo del tipo di Auto , 1 o 2)
Output: int (Identificativo del numero di AUto di un determinato tipo che sono in fase di Riparazine)
Descrizione:
    Questa funzione calcola il numero di Auto che stanno attualemente utilizzando un ponte
    Bisogna passare come parametro il tipo di auto 1 o 2
*/
{
    int numero = 0;
    for(int z = 2; z <= PONTI+1; z++)
    {
        if(event[z].x == tipo_auto)
        {
            numero++;
        }
    }
    return numero;
}

double GetArrival()
/*
Input: void
Output: double (Tempo prossimo arrivo nel sistema)
Descrizione:
    Funzione che genera gli eventi che corrispondono agli arrivi delle Automobili in Officina
*/
{
    arrivi++;
    SelectStream(0);
    arrival += Exponential(1.0 / LAMBDA);
    return (arrival);
}

double GetService(double mu)
/*
Input: double (Tempo medio di servizio)
Output: double (Tempo necessario per riparare Automobile)
Descrizione:
    Funzione per calcolare il tempo che serve al Meccanico per poter riparare l'automobile
    Bisogna passare il parametro mu perchè si può calcolare anche il tempo necessario per effettuare la Diagnosi
*/
{
    SelectStream(1);
    return (Exponential(1.0 / mu)); 
}

bool empty_queues()
/*
Input: void
Output: bool (true se ci sono elementi in coda /false se non ci sono elementi in coda)
Descrizione:
    Funzione che permette di verificare se nel sistema
*/
{
    int sum_queues = queue[0] + queue[1] +queue[2];
    if (sum_queues != 0)
        {
            return true;
        }
}


int NextEvent()
/*
Input: void
Output: int (Identificativo prossimo evento da gestire)
Descrizione:
    Funzione permette di identificare il prossimo evento da gestire
*/
{
    int e;
    int i = 0;
    while (event[i].x == 0)
    {
        i++;
    }
    e = i;
    while(i < PONTI+2){
        i++;
        if(event[i].x !=0 && (event[i].t < event[e].t))
            e = i;
    }
    return e;

}

int FoundServerFree()
/*
Input: void
Output: int (Identificativo del ponte libero)
Descrizione:
    Questa funzione permette di identificare uno dei Ponti non impegnati
*/
{
    for(int z = 2; z <= PONTI+1; z++)
    {
        if(event[z].x == 0)
        {
            return z;
        }
    }
}


void ProcessArrivals()
/*
Input: void
Output: void
Descrizione:
    Questa funzione quando l'evento da gestire è un arrivo permette di gestire l'evento,
    distinguendo anche il tipo di errivo
*/
{
    if(event[0].x == 1)
    {
        if(n_ponti_liberi != 0)
        {
            int server_free = FoundServerFree();
            double service_time = GetService(MU);
            event[server_free].t = clock.current + service_time;
            event[server_free].x = 1;
            n_ponti_liberi--;  
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
        }
        queue[1]++;
    }
    
}

int QualeCodaStavaEseguendo(int ponte_corrente)
/*
Input: int (Identificativo del Ponte)
Output: int (Identificativo del tipo di Automobile che era presente sullo specifico ponte)
Descrizione:
    Verifica quale Auto sta eseguendo sul ponte specificato
*/
{
    return event[ponte_corrente].x;
}


void ProcessDeparture(int ponte_corrente)
/*
Input: int (Identificativo del Ponte che ha finito di effettuare la riparazione)
Output: void
Descrizione:
    In questa funzione viene gestito l'evento di "Partenza" ovvero quando un Automobile libera un Ponte
*/
{
    double service_time = 0.0;

    if(ponte_corrente == 1)
    {
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
        if(n_ponti_liberi != 0 && (queue[2] - NumeroDiJobPerTipoNelSistema(2)) == 0)
        {
            int server_free = FoundServerFree();
            double service_time = GetService(MU);
            event[server_free].t = clock.current + service_time;
            event[server_free].x = 2;
            n_ponti_liberi--;
        }
        queue[2]++;   
    }
    else
    {
        if(queue[0] - NumeroDiJobPerTipoNelSistema(1)> 0)
        {   
            if(QualeCodaStavaEseguendo(ponte_corrente) == 1)
            {
                queue[0]--;
            }
            else if(QualeCodaStavaEseguendo(ponte_corrente) == 2)
            {
                queue[2]--;
            }
            service_time = GetService(MU);
            event[ponte_corrente].t = service_time +clock.current;
            event[ponte_corrente].x = 1;
        
        }
        else if(queue[2] - NumeroDiJobPerTipoNelSistema(2)> 0)
        {
            if(QualeCodaStavaEseguendo(ponte_corrente) == 1)
            {
                queue[0]--;
            }
            else if(QualeCodaStavaEseguendo(ponte_corrente) == 2)
            {
                queue[2]--;
            }
            service_time = GetService(MU);
            event[ponte_corrente].t = service_time +clock.current;
            event[ponte_corrente].x = 2;
        }
        else
        {
            if(QualeCodaStavaEseguendo(ponte_corrente) == 1)
            {
                queue[0]--;
            }
            else if(QualeCodaStavaEseguendo(ponte_corrente) == 2)
            {
                queue[2]--;
            }
            event[ponte_corrente].t = INFINITE;
            event[ponte_corrente].x = 0;
            n_ponti_liberi++;
        }
        partenze++;
    }


}


double Transiente(double t_arresto)
{
    Restart();
    clock.current = START;
    event[0].t = GetArrival();
    event[0].x = Type_of_arrive();

    while(event[0].t < t_arresto ){
        int e = NextEvent();
        clock.next = event[e].t;
        area[0] += (clock.next - clock.current) * queue[0];
        area[1] += (clock.next - clock.current) * queue[1];
        area[2] += (clock.next - clock.current) * queue[2];
        clock.current = clock.next;
    
        if(e == 0)
        {
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
    return (double)(tot_area/partenze);
}

int main()
{
    double t_arresto[10] = { 80.0, 160.0, 640.0, 1280.0, 2560.0, 5120.0, 10000.0, 15000.0, 20000.0, 30000.0 };
    char* nomiFile[10] = {"tran80.txt","tran160.txt","tran640.txt","tran1280.txt","tran2560.txt","tran5120.txt","tran10000.txt","tran15000.txt","tran20000.txt","tran30000.txt"};
    long seed = 98765;
    double tempo_di_risposta;
    
    for(int j = 0; j < 10 ; j++)
    {
        FILE *file = fopen(nomiFile[j], "w+");
        if (file == NULL)
        {
            printf("Error");
            return 0;
        }
        PlantSeeds(seed);
        for (int i = 0; i < 50; i++)
        {
            
            tempo_di_risposta = Transiente(t_arresto[j]);
            fprintf(file, "%f\n", tempo_di_risposta);
            fflush(file);
        }
        fclose(file);
    }

}